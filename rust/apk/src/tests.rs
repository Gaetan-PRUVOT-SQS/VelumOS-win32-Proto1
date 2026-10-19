use core::ffi::{c_char, c_int, c_void};
use core::ptr;
use std::collections::HashMap;
use std::ffi::CStr;
use std::sync::{LazyLock, Mutex, MutexGuard};

use crate::ffi::{t_arsc, t_axml, t_axmlattr, t_resquery, t_rsapub, t_sha256, t_text};
use crate::mem::{alloc_live, heap_free, reset_alloc};
use crate::rd::apk_lp;
use crate::{
    APK_ALGO_RSA_PKCS1_SHA256, APK_CHUNK, APK_ENTRY_MAX, APK_FILE_MAX, APKR_ACTIVITE, APKR_ALGO,
    APKR_ARCHIVE, APKR_COUNT, APKR_LIBELLE, APKR_MANIFESTE, APKR_MANQUE, APKR_MEMOIRE,
    APKR_MULTIDEX, APKR_NATIF, APKR_NOM, APKR_OK, APKR_PAQUET, APKR_SIGNATAIRES, APKR_SIGNATURE,
    APKR_TAILLE, E_ACCES, E_INVAL, E_IO, E_NOENT, E_NOMEM, E_NOTSUP, E_OK, E_OVERFLOW, E_RANGE,
    apk_manifest, apk_open, apk_read, apk_reason, apk_verify, t_apk, t_apkmanifest, t_apksig,
    t_span, t_zip, t_zipent,
};

const LABEL: u32 = 0x0101_0001;
const ANAME: u32 = 0x0101_0003;
const MIN_SDK: u32 = 0x0101_020c;
const VCODE: u32 = 0x0101_021b;
const VNAME: u32 = 0x0101_021c;
const TARGET: u32 = 0x0101_0270;
const T_REF: u32 = 0x01;
const T_STR: u32 = 0x03;
const T_DEC: u32 = 0x10;
const T_HEX: u32 = 0x11;
const CERT: &[u8] = b"CERTDATA";
const SIGB: &[u8] = b"SIG!";
const PUBK: &[u8] = b"PUBKEY01";
const PREFIX: &[u8] = b"PKDATA";
const CD: &[u8] = b"CENTRAL!";

static GATE: Mutex<()> = Mutex::new(());
static WORLD: LazyLock<Mutex<World>> = LazyLock::new(|| {
    Mutex::new(World {
        entries: Vec::new(),
        fail_open: 0,
        cd_off: 0,
        cd_size: 0,
        open_calls: 0,
        extract: None,
        find_rc: None,
        pub_bytes: Vec::new(),
        nlen: 0,
        rsa_rc: 0,
        x509_rc: 0,
        sha: HashMap::new(),
    })
});

struct Ent {
    name: Vec<u8>,
    data: Vec<u8>,
    reported: u32,
    lfh: u32,
    data_off: u32,
    csize: u32,
}

struct World {
    entries: Vec<Ent>,
    fail_open: c_int,
    cd_off: u32,
    cd_size: u32,
    open_calls: u32,
    extract: Option<i64>,
    find_rc: Option<c_int>,
    pub_bytes: Vec<u8>,
    nlen: u32,
    rsa_rc: c_int,
    x509_rc: c_int,
    sha: HashMap<usize, Vec<u8>>,
}

struct Parts {
    file: Vec<u8>,
    cd_off: u32,
    cd_size: u32,
    digest: [u8; 32],
    prefix_len: usize,
}

struct Pay<'a> {
    res: u32,
    kind: u32,
    name: &'a [u8],
    data: u32,
    text: &'a [u8],
}

fn begin() -> MutexGuard<'static, ()> {
    let g = GATE.lock().unwrap_or_else(|e| e.into_inner());
    assert_eq!(alloc_live(), 0);
    reset_alloc(u32::MAX);
    with_world(World::clear);
    g
}

fn with_world<T>(f: impl FnOnce(&mut World) -> T) -> T {
    let mut g = WORLD.lock().unwrap_or_else(|e| e.into_inner());
    f(&mut g)
}

impl World {
    fn clear(&mut self) {
        self.entries.clear();
        self.fail_open = 0;
        self.cd_off = 0;
        self.cd_size = 0;
        self.open_calls = 0;
        self.extract = None;
        self.find_rc = None;
        self.pub_bytes.clear();
        self.nlen = 0;
        self.rsa_rc = 0;
        self.x509_rc = 0;
        self.sha.clear();
    }
}

fn ent(name: &str, data: &[u8]) -> Ent {
    Ent {
        name: name.as_bytes().to_vec(),
        data: data.to_vec(),
        reported: u32::try_from(data.len()).unwrap(),
        lfh: 0,
        data_off: 0,
        csize: 0,
    }
}

fn fill(e: &Ent) -> t_zipent {
    t_zipent {
        name: e.name.as_ptr().cast(),
        name_len: u32::try_from(e.name.len()).unwrap(),
        method: 0,
        crc: 0,
        csize: e.csize,
        r#usize: e.reported,
        data_off: e.data_off,
        lfh_off: e.lfh,
    }
}

fn cstr<'a>(p: *const c_char) -> &'a [u8] {
    assert!(!p.is_null());
    let mut n = 0usize;
    // SAFETY: p est une chaîne C du test ou de la crate, lue jusqu'au NUL, bornée.
    unsafe {
        while *p.add(n) != 0 {
            n += 1;
            assert!(n < 4096);
        }
        core::slice::from_raw_parts(p.cast(), n)
    }
}

fn span_of(b: &[u8]) -> t_span {
    t_span {
        p: b.as_ptr(),
        len: b.len(),
    }
}

fn span_bytes<'a>(s: t_span) -> &'a [u8] {
    if s.p.is_null() || s.len == 0 {
        return &[];
    }
    // SAFETY: le span vient d'un tampon du test encore vivant, de longueur s.len.
    unsafe { core::slice::from_raw_parts(s.p, s.len) }
}

fn z(buf: &[u8]) -> &[u8] {
    match buf.iter().position(|&c| c == 0) {
        Some(i) => &buf[..i],
        None => buf,
    }
}

fn toy_hash(data: &[u8]) -> [u8; 32] {
    let mut h = [0u8; 32];
    let mut acc = 0x811c_9dc5_u32;
    for (i, b) in data.iter().enumerate() {
        acc ^= u32::from(*b);
        acc = acc.wrapping_mul(0x0100_0193);
        acc ^= u32::try_from(i & 0xffff).unwrap_or(u32::MAX);
        let slot = i % 28;
        h[slot] ^= acc.to_le_bytes()[i % 4];
        h[(slot + 7) % 28] = h[(slot + 7) % 28].wrapping_add(*b);
    }
    let n = u32::try_from(data.len()).unwrap_or(u32::MAX);
    h[28..32].copy_from_slice(&n.to_le_bytes());
    h[0] ^= acc.to_le_bytes()[0];
    h
}

fn chunk_count(n: usize) -> u32 {
    let sum = n.checked_add(APK_CHUNK - 1).unwrap();
    u32::try_from(sum / APK_CHUNK).unwrap()
}

fn section_hashes(data: &[u8]) -> Vec<[u8; 32]> {
    let mut out = Vec::new();
    let mut off = 0usize;
    while off < data.len() {
        let n = (data.len() - off).min(APK_CHUNK);
        let mut buf = Vec::new();
        buf.push(0xa5);
        buf.extend(u32::try_from(n).unwrap().to_le_bytes());
        buf.extend(&data[off..off + n]);
        out.push(toy_hash(&buf));
        off += n;
    }
    out
}

fn expect_digest(prefix: &[u8], cd: &[u8], eocd: &[u8], block_off: u32) -> [u8; 32] {
    let chunks = chunk_count(prefix.len())
        .wrapping_add(chunk_count(cd.len()))
        .wrapping_add(1);
    let mut top = Vec::new();
    top.push(0x5a);
    top.extend(chunks.to_le_bytes());
    for h in section_hashes(prefix) {
        top.extend(h);
    }
    for h in section_hashes(cd) {
        top.extend(h);
    }
    let mut child = Vec::new();
    child.push(0xa5);
    child.extend(u32::try_from(eocd.len()).unwrap().to_le_bytes());
    child.extend(&eocd[..16]);
    child.extend(block_off.to_le_bytes());
    child.extend(&eocd[20..]);
    top.extend(toy_hash(&child));
    toy_hash(&top)
}

fn lp(data: &[u8]) -> Vec<u8> {
    let mut v = u32::try_from(data.len()).unwrap().to_le_bytes().to_vec();
    v.extend(data);
    v
}

fn item(algo: u32, value: &[u8]) -> Vec<u8> {
    let mut v = algo.to_le_bytes().to_vec();
    v.extend(lp(value));
    v
}

fn one_body(digest: &[u8], algo: u32, copies: usize) -> Vec<u8> {
    let mut digests = Vec::new();
    let mut sigs = Vec::new();
    for _ in 0..copies.max(1) {
        digests.extend(lp(&item(algo, digest)));
        sigs.extend(lp(&item(algo, SIGB)));
    }
    let mut signed = lp(&digests);
    signed.extend(lp(&lp(CERT)));
    signed.extend(lp(&[]));
    let mut body = lp(&signed);
    body.extend(lp(&sigs));
    body.extend(lp(PUBK));
    body
}

fn v2_bodies(digest: &[u8], algo: u32, copies: usize, signers: usize) -> Vec<u8> {
    let body = one_body(digest, algo, copies);
    let mut seq = Vec::new();
    for _ in 0..signers.max(1) {
        seq.extend(lp(&body));
    }
    lp(&seq)
}

fn eocd(cd_off: u32, cd_size: u32, comment: &[u8]) -> Vec<u8> {
    let mut e = vec![0u8; 22 + comment.len()];
    e[..4].copy_from_slice(&0x0605_4b50u32.to_le_bytes());
    e[12..16].copy_from_slice(&cd_size.to_le_bytes());
    e[16..20].copy_from_slice(&cd_off.to_le_bytes());
    e[20..22].copy_from_slice(&u16::try_from(comment.len()).unwrap().to_le_bytes());
    e[22..].copy_from_slice(comment);
    e
}

fn pair_of(v2: &[u8]) -> Vec<u8> {
    let n = u64::try_from(v2.len() + 4).unwrap();
    let mut p = n.to_le_bytes().to_vec();
    p.extend(0x7109_871au32.to_le_bytes());
    p.extend(v2);
    p
}

fn block_of(pair: &[u8]) -> Vec<u8> {
    let size = u64::try_from(pair.len() + 24).unwrap();
    let mut b = size.to_le_bytes().to_vec();
    b.extend(pair);
    b.extend(size.to_le_bytes());
    b.extend(b"APK Sig Block 42");
    b
}

fn assemble(v2: &[u8], comment: &[u8]) -> Parts {
    let block = block_of(&pair_of(v2));
    let cd_off = u32::try_from(PREFIX.len() + block.len()).unwrap();
    let cd_size = u32::try_from(CD.len()).unwrap();
    let tail = eocd(cd_off, cd_size, comment);
    let digest = expect_digest(PREFIX, CD, &tail, u32::try_from(PREFIX.len()).unwrap());
    let mut file = Vec::new();
    file.extend(PREFIX);
    file.extend(&block);
    file.extend(CD);
    file.extend(&tail);
    Parts {
        file,
        cd_off,
        cd_size,
        digest,
        prefix_len: PREFIX.len(),
    }
}

fn signed(algo: u32, copies: usize, signers: usize, comment: &[u8], flip: bool) -> Parts {
    let dummy = v2_bodies(&[0u8; 32], algo, copies, signers);
    let guess = assemble(&dummy, comment);
    let mut dig = guess.digest;
    if flip {
        dig[0] ^= 0xff;
    }
    let v2 = v2_bodies(&dig, algo, copies, signers);
    assert_eq!(v2.len(), dummy.len());
    let parts = assemble(&v2, comment);
    assert_eq!(parts.digest, guess.digest);
    parts
}

fn open_ents(entries: Vec<Ent>, file: &[u8]) -> (c_int, t_apk) {
    with_world(|w| w.entries = entries);
    let mut a = t_apk::zero();
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    let rc = unsafe { apk_open(&mut a, span_of(file)) };
    (rc, a)
}

fn open_names(names: &[&str]) -> (c_int, t_apk) {
    let entries = names.iter().map(|n| ent(n, b"x")).collect();
    open_ents(entries, b"apk")
}

fn open_signed(parts: &Parts, lfh: u32) -> t_apk {
    let mut man = ent("AndroidManifest.xml", b"m");
    man.lfh = lfh;
    let dex = ent("classes.dex", b"d");
    with_world(|w| {
        w.entries = vec![man, dex];
        w.cd_off = parts.cd_off;
        w.cd_size = parts.cd_size;
        w.pub_bytes = PUBK.to_vec();
        w.nlen = u32::try_from(SIGB.len()).unwrap();
        w.rsa_rc = 0;
        w.x509_rc = 0;
    });
    let mut a = t_apk::zero();
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    let rc = unsafe { apk_open(&mut a, span_of(&parts.file)) };
    assert_eq!(rc, E_OK);
    assert_eq!(a.reason, APKR_OK);
    a
}

fn verify(a: &t_apk) -> (c_int, t_apksig) {
    let mut out = t_apksig::zero();
    out.cert_sha256 = [0xab; 32];
    out.algo = 9;
    out.reason = 3;
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    let rc = unsafe { apk_verify(a, &mut out) };
    (rc, out)
}

fn read_copy(a: &t_apk, name: &CStr) -> (i64, Option<Vec<u8>>) {
    let mut p: *mut u8 = ptr::null_mut();
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    let n = unsafe { apk_read(a, name.as_ptr(), &mut p) };
    if n < 0 {
        assert!(p.is_null());
        return (n, None);
    }
    assert!(!p.is_null());
    let len = usize::try_from(n).unwrap();
    // SAFETY: apk_read a rendu len octets initialisés, libérés juste après la copie.
    let v = unsafe { core::slice::from_raw_parts(p, len).to_vec() };
    heap_free(p.cast());
    (n, Some(v))
}

fn manifest_of(axml: &[u8], extra: Vec<Ent>, left: u32) -> (c_int, t_apkmanifest) {
    let mut entries = vec![ent("AndroidManifest.xml", axml), ent("classes.dex", b"dex")];
    entries.extend(extra);
    let (rc, a) = open_ents(entries, b"apk");
    assert_eq!(rc, E_OK);
    reset_alloc(left);
    let mut m = t_apkmanifest::zero();
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    let r = unsafe { apk_manifest(&a, &mut m) };
    assert_eq!(alloc_live(), 0);
    (r, m)
}

fn push_u16(b: &mut Vec<u8>, v: u16) {
    b.extend(v.to_le_bytes());
}

fn push_u32(b: &mut Vec<u8>, v: u32) {
    b.extend(v.to_le_bytes());
}

fn push_z(b: &mut Vec<u8>, s: &[u8]) {
    push_u16(b, u16::try_from(s.len()).unwrap());
    b.extend(s);
    b.push(0);
}

fn ev_start(b: &mut Vec<u8>, name: &[u8], attrs: &[Pay<'_>]) {
    b.push(1);
    push_z(b, name);
    push_u16(b, u16::try_from(attrs.len()).unwrap());
    for a in attrs {
        push_u32(b, a.res);
        push_u32(b, a.kind);
        push_z(b, a.name);
        if a.kind == T_STR {
            push_z(b, a.text);
        } else {
            push_u32(b, a.data);
        }
    }
}

fn ev_end(b: &mut Vec<u8>) {
    b.push(2);
}

fn pay<'a>(res: u32, kind: u32, name: &'a [u8], data: u32, text: &'a [u8]) -> Pay<'a> {
    Pay {
        res,
        kind,
        name,
        data,
        text,
    }
}

fn tree(pkg: &[u8], activity: &[u8], label: Option<Pay<'_>>, version_name: bool) -> Vec<u8> {
    let mut b = b"VMAN".to_vec();
    let mut root = vec![
        pay(0, T_STR, b"package", 0, pkg),
        pay(VCODE, T_HEX, b"versionCode", 0x2a, b""),
    ];
    if version_name {
        root.push(pay(VNAME, T_STR, b"versionName", 0, b"1.2"));
    }
    ev_start(&mut b, b"manifest", &root);
    ev_start(
        &mut b,
        b"uses-sdk",
        &[
            pay(MIN_SDK, T_DEC, b"minSdkVersion", 21, b""),
            pay(TARGET, T_DEC, b"targetSdkVersion", 34, b""),
        ],
    );
    ev_end(&mut b);
    ev_start(
        &mut b,
        b"uses-permission",
        &[pay(
            ANAME,
            T_STR,
            b"name",
            0,
            b"android.permission.INTERNET",
        )],
    );
    ev_end(&mut b);
    let app = match label {
        Some(p) => vec![p],
        None => Vec::new(),
    };
    ev_start(&mut b, b"application", &app);
    ev_start(
        &mut b,
        b"activity",
        &[pay(ANAME, T_STR, b"name", 0, activity)],
    );
    ev_start(&mut b, b"intent-filter", &[]);
    ev_start(
        &mut b,
        b"action",
        &[pay(ANAME, T_STR, b"name", 0, b"android.intent.action.MAIN")],
    );
    ev_end(&mut b);
    ev_start(
        &mut b,
        b"category",
        &[pay(
            ANAME,
            T_STR,
            b"name",
            0,
            b"android.intent.category.LAUNCHER",
        )],
    );
    ev_end(&mut b);
    ev_end(&mut b);
    ev_end(&mut b);
    ev_end(&mut b);
    ev_end(&mut b);
    b
}

fn arsc_blob(id: u32, text: &[u8]) -> Vec<u8> {
    let mut b = b"ARSC".to_vec();
    b.extend(id.to_le_bytes());
    b.extend(u32::try_from(text.len()).unwrap().to_le_bytes());
    b.extend(text);
    b.push(0);
    b
}

fn rd_u16(f: &[u8], i: usize) -> Option<(u16, usize)> {
    let s = f.get(i..i + 2)?;
    Some((u16::from_le_bytes([s[0], s[1]]), i + 2))
}

fn rd_u32(f: &[u8], i: usize) -> Option<(u32, usize)> {
    let s = f.get(i..i + 4)?;
    Some((u32::from_le_bytes([s[0], s[1], s[2], s[3]]), i + 4))
}

fn skip_z(f: &[u8], i: usize) -> Option<(u32, usize)> {
    let (n, i) = rd_u16(f, i)?;
    let at = u32::try_from(i).ok()?;
    let next = i.checked_add(usize::from(n))?.checked_add(1)?;
    if next > f.len() || f[next - 1] != 0 {
        return None;
    }
    Some((at, next))
}

struct AttrRec {
    res: u32,
    kind: u32,
    name_at: u32,
    data: u32,
    next: usize,
}

fn parse_attr(f: &[u8], i: usize) -> Option<AttrRec> {
    let (res, i) = rd_u32(f, i)?;
    let (kind, i) = rd_u32(f, i)?;
    let (name_at, i) = skip_z(f, i)?;
    if kind == T_STR {
        let (at, next) = skip_z(f, i)?;
        Some(AttrRec {
            res,
            kind,
            name_at,
            data: at,
            next,
        })
    } else {
        let (data, next) = rd_u32(f, i)?;
        Some(AttrRec {
            res,
            kind,
            name_at,
            data,
            next,
        })
    }
}

fn fit(n: usize) -> c_int {
    match u32::try_from(n) {
        Ok(_) => E_OK,
        Err(_) => E_INVAL,
    }
}

fn write_z(out: t_text, s: &[u8]) -> c_int {
    if out.p.is_null() || s.len() >= out.cap {
        return E_INVAL;
    }
    // SAFETY: out.p couvre out.cap octets, et s.len() + 1 tient dedans.
    unsafe {
        ptr::copy_nonoverlapping(s.as_ptr(), out.p.cast(), s.len());
        *out.p.add(s.len()) = 0;
    }
    E_OK
}

fn name_is_safe(name: &[u8]) -> bool {
    if name.is_empty() || name[0] == b'/' {
        return false;
    }
    let len = name.len();
    let mut i = 0usize;
    let mut start = 0usize;
    while i <= len {
        if i < len && (name[i] == 0 || name[i] == b'\\') {
            return false;
        }
        if i == len || name[i] == b'/' {
            if i - start == 2 && name[start] == b'.' && name[start + 1] == b'.' {
                return false;
            }
            start = i + 1;
        }
        i += 1;
    }
    true
}

#[unsafe(no_mangle)]
unsafe extern "C" fn zip_open(z: *mut t_zip, file: t_span) -> c_int {
    if z.is_null() {
        return E_INVAL;
    }
    let mut rc = E_OK;
    let mut zip = t_zip::zero();
    with_world(|w| {
        w.open_calls = w.open_calls.wrapping_add(1);
        if w.fail_open < 0 {
            rc = w.fail_open;
            return;
        }
        zip = t_zip {
            file,
            cd_off: w.cd_off,
            cd_size: w.cd_size,
            count: u32::try_from(w.entries.len()).unwrap(),
        };
    });
    if rc < 0 {
        return rc;
    }
    // SAFETY: z est non nul. zip est une valeur locale copiée.
    unsafe { *z = zip };
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn zip_entry(z: *const t_zip, index: u32, out: *mut t_zipent) -> c_int {
    if z.is_null() || out.is_null() {
        return E_INVAL;
    }
    let found = with_world(|w| {
        let i = usize::try_from(index).ok()?;
        w.entries.get(i).map(fill)
    });
    match found {
        Some(e) => {
            // SAFETY: out est non nul et exclusif.
            unsafe { *out = e };
            E_OK
        }
        None => E_INVAL,
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn zip_find(z: *const t_zip, name: *const c_char, out: *mut t_zipent) -> c_int {
    if z.is_null() || name.is_null() || out.is_null() {
        return E_INVAL;
    }
    if let Some(rc) = with_world(|w| w.find_rc) {
        return rc;
    }
    let want = cstr(name);
    let found = with_world(|w| w.entries.iter().find(|e| e.name == want).map(fill));
    match found {
        Some(e) => {
            // SAFETY: out est non nul et exclusif.
            unsafe { *out = e };
            E_OK
        }
        None => E_NOENT,
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn zip_extract(
    z: *const t_zip,
    e: *const t_zipent,
    out: *mut u8,
    cap: usize,
) -> i64 {
    if z.is_null() || e.is_null() {
        return i64::from(E_INVAL);
    }
    if let Some(rc) = with_world(|w| w.extract) {
        return rc;
    }
    // SAFETY: e est non nul. On ne lit que le pointeur de nom, pas au-delà.
    let name = unsafe { (*e).name };
    let data = with_world(|w| {
        w.entries
            .iter()
            .find(|it| ptr::eq(it.name.as_ptr().cast::<c_char>(), name))
            .map(|it| it.data.clone())
    });
    let Some(data) = data else {
        return i64::from(E_NOENT);
    };
    if cap < data.len() {
        return i64::from(E_OVERFLOW);
    }
    if data.is_empty() {
        return 0;
    }
    if out.is_null() {
        return i64::from(E_INVAL);
    }
    // SAFETY: cap couvre data.len() octets inscriptibles à out.
    unsafe { ptr::copy_nonoverlapping(data.as_ptr(), out, data.len()) };
    i64::try_from(data.len()).unwrap()
}

#[unsafe(no_mangle)]
unsafe extern "C" fn zip_name_safe(name: *const c_char, len: usize) -> c_int {
    if name.is_null() || len == 0 {
        return 0;
    }
    // SAFETY: apk_open passe name_len octets lisibles, posés par zip_entry.
    let bytes = unsafe { core::slice::from_raw_parts(name.cast::<u8>(), len) };
    if name_is_safe(bytes) { 1 } else { 0 }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn sha256_init(c: *mut t_sha256) {
    if c.is_null() {
        return;
    }
    with_world(|w| {
        w.sha.insert(c.addr(), Vec::new());
    });
}

#[unsafe(no_mangle)]
unsafe extern "C" fn sha256_update(c: *mut t_sha256, data: *const c_void, n: usize) {
    if c.is_null() || n == 0 || data.is_null() {
        return;
    }
    // SAFETY: data couvre n octets le temps de l'appel.
    let bytes = unsafe { core::slice::from_raw_parts(data.cast::<u8>(), n) };
    with_world(|w| {
        w.sha.entry(c.addr()).or_default().extend_from_slice(bytes);
    });
}

#[unsafe(no_mangle)]
unsafe extern "C" fn sha256_final(c: *mut t_sha256, out: *mut u8) {
    if c.is_null() || out.is_null() {
        return;
    }
    let data = with_world(|w| w.sha.remove(&c.addr()).unwrap_or_default());
    let h = toy_hash(&data);
    // SAFETY: out couvre 32 octets, contrat de sha256_final.
    unsafe { ptr::copy_nonoverlapping(h.as_ptr(), out, 32) };
}

#[unsafe(no_mangle)]
unsafe extern "C" fn sha256(data: *const c_void, n: usize, out: *mut u8) {
    if out.is_null() {
        return;
    }
    let bytes = if n == 0 || data.is_null() {
        &[][..]
    } else {
        // SAFETY: data couvre n octets le temps de l'appel.
        unsafe { core::slice::from_raw_parts(data.cast::<u8>(), n) }
    };
    let h = toy_hash(bytes);
    // SAFETY: out couvre 32 octets.
    unsafe { ptr::copy_nonoverlapping(h.as_ptr(), out, 32) };
}

#[unsafe(no_mangle)]
unsafe extern "C" fn ct_equal(a: *const c_void, b: *const c_void, n: usize) -> c_int {
    if n == 0 {
        return 1;
    }
    if a.is_null() || b.is_null() {
        return 0;
    }
    // SAFETY: a et b couvrent n octets.
    let (x, y) = unsafe {
        (
            core::slice::from_raw_parts(a.cast::<u8>(), n),
            core::slice::from_raw_parts(b.cast::<u8>(), n),
        )
    };
    if x == y { 1 } else { 0 }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn x509_spki(cert: t_span, spki: *mut t_span) -> c_int {
    let _ = cert;
    if spki.is_null() {
        return E_INVAL;
    }
    let (rc, p, n) = with_world(|w| (w.x509_rc, w.pub_bytes.as_ptr(), w.pub_bytes.len()));
    if rc < 0 {
        return rc;
    }
    // SAFETY: spki est non nul. p reste valide tant que pub_bytes n'est pas réalloué.
    unsafe {
        *spki = t_span { p, len: n };
    }
    rc
}

#[unsafe(no_mangle)]
unsafe extern "C" fn der_spki_rsa(spki: t_span, out: *mut t_rsapub) -> c_int {
    let _ = spki;
    if out.is_null() {
        return E_INVAL;
    }
    let nlen = with_world(|w| w.nlen);
    // SAFETY: out est non nul et exclusif.
    unsafe {
        *out = t_rsapub::zero();
        (*out).nlen = nlen;
        (*out).e = 65537;
    }
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn rsa_pkcs1_sha256_verify(
    k: *const t_rsapub,
    sig: t_span,
    digest: *const u8,
) -> c_int {
    let _ = (k, sig, digest);
    with_world(|w| w.rsa_rc)
}

#[unsafe(no_mangle)]
unsafe extern "C" fn axml_open(x: *mut t_axml, file: t_span) -> c_int {
    if x.is_null() {
        return E_INVAL;
    }
    let bytes = span_bytes(file);
    if bytes.len() < 4 || &bytes[..4] != b"VMAN" {
        return E_INVAL;
    }
    // SAFETY: x est non nul et exclusif.
    unsafe {
        *x = t_axml::zero();
        (*x).file = file;
        (*x).pos = 4;
    }
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn axml_next(x: *mut t_axml) -> c_int {
    if x.is_null() {
        return E_INVAL;
    }
    // SAFETY: x est non nul. Le fichier posé par axml_open est encore vivant.
    let x = unsafe { &mut *x };
    let file = span_bytes(x.file);
    let pos = match usize::try_from(x.pos) {
        Ok(v) => v,
        Err(_) => return E_INVAL,
    };
    if pos >= file.len() {
        return 0;
    }
    let tag = file[pos];
    if tag == 2 {
        let Some(next) = u32::try_from(pos + 1).ok() else {
            return E_INVAL;
        };
        x.pos = next;
        x.ev = 2;
        return 2;
    }
    if tag != 1 {
        return E_INVAL;
    }
    let Some((nlen, after)) = rd_u16(file, pos + 1) else {
        return E_INVAL;
    };
    let Some(name_at) = u32::try_from(after).ok() else {
        return E_INVAL;
    };
    let Some(after_name) = after
        .checked_add(usize::from(nlen))
        .and_then(|v| v.checked_add(1))
    else {
        return E_INVAL;
    };
    if after_name > file.len() || file[after_name - 1] != 0 {
        return E_INVAL;
    }
    let Some((nattr, attr_at)) = rd_u16(file, after_name) else {
        return E_INVAL;
    };
    let mut i = attr_at;
    for _ in 0..nattr {
        let Some(rec) = parse_attr(file, i) else {
            return E_INVAL;
        };
        i = rec.next;
    }
    if fit(i) < 0 {
        return E_INVAL;
    }
    x.ev = 1;
    x.ev_off = name_at;
    x.map_count = u32::from(nlen);
    x.attr_off = u32::try_from(attr_at).unwrap();
    x.attr_count = u32::from(nattr);
    x.pos = u32::try_from(i).unwrap();
    1
}

#[unsafe(no_mangle)]
unsafe extern "C" fn axml_name(x: *const t_axml, out: t_text) -> c_int {
    if x.is_null() {
        return E_INVAL;
    }
    // SAFETY: x est non nul et son fichier est vivant.
    let x = unsafe { &*x };
    let file = span_bytes(x.file);
    let off = match usize::try_from(x.ev_off) {
        Ok(v) => v,
        Err(_) => return E_INVAL,
    };
    let n = match usize::try_from(x.map_count) {
        Ok(v) => v,
        Err(_) => return E_INVAL,
    };
    let Some(s) = file.get(off..off + n) else {
        return E_INVAL;
    };
    write_z(out, s)
}

#[unsafe(no_mangle)]
unsafe extern "C" fn axml_attr_count(x: *const t_axml) -> u32 {
    if x.is_null() {
        return 0;
    }
    // SAFETY: x est non nul.
    unsafe { (*x).attr_count }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn axml_attr(x: *const t_axml, i: u32, out: *mut t_axmlattr) -> c_int {
    if x.is_null() || out.is_null() {
        return E_INVAL;
    }
    // SAFETY: x est non nul. out est non nul.
    let x = unsafe { &*x };
    if i >= x.attr_count {
        return E_INVAL;
    }
    let file = span_bytes(x.file);
    let mut at = match usize::try_from(x.attr_off) {
        Ok(v) => v,
        Err(_) => return E_INVAL,
    };
    for _ in 0..i {
        let Some(rec) = parse_attr(file, at) else {
            return E_INVAL;
        };
        at = rec.next;
    }
    let Some(rec) = parse_attr(file, at) else {
        return E_INVAL;
    };
    // SAFETY: out est non nul et exclusif.
    unsafe {
        *out = t_axmlattr {
            name: rec.name_at,
            res_id: rec.res,
            raw: 0,
            r#type: rec.kind,
            data: rec.data,
            ns: 0,
        };
    }
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn axml_attr_find(x: *const t_axml, res_id: u32, out: *mut t_axmlattr) -> c_int {
    if res_id == 0 {
        return E_INVAL;
    }
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    let n = unsafe { axml_attr_count(x) };
    let mut i = 0u32;
    while i < n {
        let mut a = t_axmlattr::zero();
        // SAFETY: pointeurs du test, contrôlés avant l'appel.
        let r = unsafe { axml_attr(x, i, &mut a) };
        if r < 0 {
            return r;
        }
        if a.res_id == res_id {
            if out.is_null() {
                return E_INVAL;
            }
            // SAFETY: out est non nul.
            unsafe { *out = a };
            return E_OK;
        }
        i += 1;
    }
    E_NOENT
}

#[unsafe(no_mangle)]
unsafe extern "C" fn axml_string(x: *const t_axml, index: u32, out: t_text) -> c_int {
    if x.is_null() {
        return E_INVAL;
    }
    // SAFETY: x est non nul et son fichier est vivant.
    let file = span_bytes(unsafe { (*x).file });
    let off = match usize::try_from(index) {
        Ok(v) => v,
        Err(_) => return E_INVAL,
    };
    let Some(rest) = file.get(off..) else {
        return E_INVAL;
    };
    let Some(n) = rest.iter().position(|&c| c == 0) else {
        return E_INVAL;
    };
    write_z(out, &rest[..n])
}

#[unsafe(no_mangle)]
unsafe extern "C" fn arsc_open(a: *mut t_arsc, file: t_span) -> c_int {
    if a.is_null() {
        return E_INVAL;
    }
    let bytes = span_bytes(file);
    if bytes.len() < 12 || &bytes[..4] != b"ARSC" {
        return E_INVAL;
    }
    // SAFETY: a est non nul et exclusif.
    unsafe {
        *a = t_arsc::zero();
        (*a).file = file;
    }
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn arsc_string(a: *const t_arsc, q: *const t_resquery, out: t_text) -> c_int {
    if a.is_null() || q.is_null() {
        return E_INVAL;
    }
    // SAFETY: a et q sont non nuls. Le fichier d'arsc_open est vivant.
    let (file, id_q, lang) = unsafe { (span_bytes((*a).file), (*q).res_id, (*q).lang) };
    if lang.is_null() {
        return E_INVAL;
    }
    if file.len() < 12 || &file[..4] != b"ARSC" {
        return E_INVAL;
    }
    let id = u32::from_le_bytes([file[4], file[5], file[6], file[7]]);
    let n = u32::from_le_bytes([file[8], file[9], file[10], file[11]]);
    if id != id_q {
        return E_NOENT;
    }
    let start = 12usize;
    let Some(end) = start.checked_add(usize::try_from(n).unwrap_or(usize::MAX)) else {
        return E_INVAL;
    };
    let Some(s) = file.get(start..end) else {
        return E_INVAL;
    };
    write_z(out, s)
}

#[cfg(velum_host)]
unsafe extern "C" {
    fn malloc(size: usize) -> *mut c_void;
    fn free(ptr: *mut c_void);
}

#[cfg(velum_host)]
#[unsafe(no_mangle)]
unsafe extern "C" fn apk_host_alloc(size: usize) -> *mut c_void {
    // SAFETY: malloc de la libc. size est transmis tel quel.
    unsafe { malloc(size) }
}

#[cfg(velum_host)]
#[unsafe(no_mangle)]
unsafe extern "C" fn apk_host_free(ptr: *mut c_void) {
    // SAFETY: ptr est nul ou vient d'apk_host_alloc dans ce test.
    unsafe { free(ptr) }
}

#[test]
fn null_pointers() {
    let _g = begin();
    let blob = b"a";
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    assert_eq!(unsafe { apk_open(ptr::null_mut(), span_of(blob)) }, E_INVAL);
    let mut gone = 0x11u8;
    let mut out = &mut gone as *mut u8;
    assert_eq!(
        // SAFETY: pointeurs du test, contrôlés avant l'appel.
        unsafe { apk_read(ptr::null(), ptr::null(), ptr::null_mut()) },
        i64::from(E_INVAL)
    );
    assert_eq!(gone, 0x11);
    assert_eq!(
        // SAFETY: pointeurs du test, contrôlés avant l'appel.
        unsafe { apk_read(ptr::null(), c"a".as_ptr(), &mut out) },
        i64::from(E_INVAL)
    );
    assert!(out.is_null());
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    assert_eq!(unsafe { apk_verify(ptr::null(), ptr::null_mut()) }, E_INVAL);
    let mut sig = t_apksig::zero();
    sig.cert_sha256 = [7; 32];
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    assert_eq!(unsafe { apk_verify(ptr::null(), &mut sig) }, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    assert_eq!(sig.cert_sha256, [0; 32]);
    assert_eq!(sig.algo, 0);
    assert_eq!(
        // SAFETY: pointeurs du test, contrôlés avant l'appel.
        unsafe { apk_manifest(ptr::null(), ptr::null_mut()) },
        E_INVAL
    );
}

#[test]
fn reason_table_and_outside() {
    let _g = begin();
    let phrases = [
        "Aucun refus.",
        "Le fichier n'est pas une archive APK lisible.",
        "L'application ou l'un de ses fichiers est trop volumineux.",
        "L'archive contient un nom de fichier dangereux.",
        "L'application contient du code natif, non pris en charge.",
        "L'application contient plusieurs fichiers DEX, non pris en charge.",
        "Il manque le manifeste ou le code de l'application.",
        "La signature de l'application est absente ou invalide.",
        "L'algorithme de signature n'est pas pris en charge.",
        "L'application a plusieurs signataires, non pris en charge.",
        "Le manifeste de l'application est illisible.",
        "Le nom de paquet de l'application est invalide.",
        "L'application n'a pas d'écran principal à lancer.",
        "Le nom de l'application est introuvable ou invalide.",
        "La mémoire manque pour lire l'application.",
    ];
    assert_eq!(phrases.len(), usize::try_from(APKR_COUNT).unwrap());
    for (i, phrase) in phrases.iter().enumerate() {
        // SAFETY: pointeurs du test, contrôlés avant l'appel.
        let p = unsafe { apk_reason(c_int::try_from(i).unwrap()) };
        // SAFETY: apk_reason renvoie une chaîne C statique pour un indice du tableau.
        let got = unsafe { CStr::from_ptr(p) };
        assert_eq!(got.to_bytes(), phrase.as_bytes());
    }
    for reason in [APKR_COUNT, -1, 100, c_int::MIN] {
        // SAFETY: pointeurs du test, contrôlés avant l'appel.
        let p = unsafe { apk_reason(reason) };
        // SAFETY: hors table, la chaîne statique « Refus inconnu. » est renvoyée.
        let got = unsafe { CStr::from_ptr(p) };
        assert_eq!(got.to_bytes(), b"Refus inconnu.");
    }
}

#[test]
fn lp_round_trip_and_reject() {
    let _g = begin();
    let raw = [3u8, 0, 0, 0, b'a', b'b', 0, b'Z'];
    let mut view = &raw[..];
    let body = apk_lp(&mut view).unwrap();
    assert_eq!(body, b"ab\0");
    assert_eq!(view, b"Z");
    let short = [1u8, 2, 3];
    let mut s = &short[..];
    assert_eq!(apk_lp(&mut s), Err(E_ACCES));
    assert_eq!(s, &short);
    let huge = [0xffu8, 0xff, 0xff, 0x7f, 0xaa];
    let mut h = &huge[..];
    assert_eq!(apk_lp(&mut h), Err(E_ACCES));
    assert_eq!(h, &huge);
}

#[test]
fn read_round_trip_interior_nul() {
    let _g = begin();
    let payload = b"ab\0cd";
    let (rc, a) = open_ents(
        vec![
            ent("AndroidManifest.xml", b"m"),
            ent("classes.dex", b"d"),
            ent("res/a", payload),
        ],
        b"apk",
    );
    assert_eq!(rc, E_OK);
    let (n, got) = read_copy(&a, c"res/a");
    assert_eq!(n, i64::try_from(payload.len()).unwrap());
    assert_eq!(got.unwrap(), payload);
    assert_eq!(alloc_live(), 0);
}

#[test]
fn read_errors_and_alloc() {
    let _g = begin();
    let (rc, a) = open_names(&["AndroidManifest.xml", "classes.dex"]);
    assert_eq!(rc, E_OK);
    let (n, got) = read_copy(&a, c"nope");
    assert_eq!(n, i64::from(E_NOENT));
    assert!(got.is_none());
    with_world(|w| w.find_rc = Some(E_IO));
    let (n, _) = read_copy(&a, c"AndroidManifest.xml");
    assert_eq!(n, i64::from(E_NOENT));
    with_world(|w| {
        w.find_rc = None;
        w.extract = Some(2);
    });
    let (n, got) = read_copy(&a, c"AndroidManifest.xml");
    assert_eq!(n, i64::from(E_INVAL));
    assert!(got.is_none());
    with_world(|w| w.extract = Some(i64::from(E_IO)));
    let (n, _) = read_copy(&a, c"classes.dex");
    assert_eq!(n, i64::from(E_IO));
    with_world(|w| w.extract = None);
    reset_alloc(0);
    let mut p = ptr::null_mut();
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    let n = unsafe { apk_read(&a, c"classes.dex".as_ptr(), &mut p) };
    assert_eq!(n, i64::from(E_NOMEM));
    assert!(p.is_null());
    assert_eq!(alloc_live(), 0);
    let mut big = ent("big.bin", b"zz");
    big.reported = APK_ENTRY_MAX + 1;
    let (rc, a) = open_ents(
        vec![
            ent("AndroidManifest.xml", b"m"),
            ent("classes.dex", b"d"),
            big,
        ],
        b"apk",
    );
    assert_eq!(rc, E_OK);
    let (n, _) = read_copy(&a, c"big.bin");
    assert_eq!(n, i64::from(E_RANGE));
    assert_eq!(alloc_live(), 0);
}

#[test]
fn open_bounds_and_names() {
    let _g = begin();
    static ONE: u8 = 0;
    let mut a = t_apk::zero();
    let huge = t_span {
        p: ptr::null(),
        len: APK_FILE_MAX + 1,
    };
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    assert_eq!(unsafe { apk_open(&mut a, huge) }, E_INVAL);
    assert_eq!(a.reason, APKR_ARCHIVE);
    assert!(a.file.p.is_null());
    assert_eq!(with_world(|w| w.open_calls), 0);
    let over = t_span {
        p: &ONE,
        len: APK_FILE_MAX + 1,
    };
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    assert_eq!(unsafe { apk_open(&mut a, over) }, E_RANGE);
    assert_eq!(a.reason, APKR_TAILLE);
    assert!(a.file.p.is_null());
    assert_eq!(with_world(|w| w.open_calls), 0);
    let exact = t_span {
        p: &ONE,
        len: APK_FILE_MAX,
    };
    // SAFETY: pointeurs du test, contrôlés avant l'appel.
    assert_eq!(unsafe { apk_open(&mut a, exact) }, E_NOENT);
    assert_eq!(a.reason, APKR_MANQUE);
    assert_eq!(with_world(|w| w.open_calls), 1);
    with_world(|w| w.fail_open = E_NOENT);
    let (rc, a) = open_names(&["AndroidManifest.xml", "classes.dex"]);
    assert_eq!(rc, E_INVAL);
    assert_eq!(a.reason, APKR_ARCHIVE);
    assert!(a.file.p.is_null());
    with_world(|w| w.fail_open = 0);
    let (rc, a) = open_names(&["../x"]);
    assert_eq!(rc, E_INVAL);
    assert_eq!(a.reason, APKR_NOM);
    assert!(a.file.p.is_null());
    let (rc, a) = open_names(&["/abs"]);
    assert_eq!(a.reason, APKR_NOM);
    assert_eq!(rc, E_INVAL);
    let (rc, a) = open_names(&["lib/a.so", "AndroidManifest.xml", "classes.dex"]);
    assert_eq!(rc, E_NOTSUP);
    assert_eq!(a.reason, APKR_NATIF);
    let (rc, a) = open_names(&["AndroidManifest.xml", "classes.dex", "classes2.dex"]);
    assert_eq!(rc, E_NOTSUP);
    assert_eq!(a.reason, APKR_MULTIDEX);
    let (rc, a) = open_names(&[
        "AndroidManifest.xml",
        "classes.dex",
        "lib.so",
        "Lib/a.so",
        "notlib/a.so",
        "foo/classes2.dex",
    ]);
    assert_eq!(rc, E_OK);
    assert_eq!(a.reason, APKR_OK);
    assert!(!a.file.p.is_null());
    let (rc, a) = open_names(&["classes.dex"]);
    assert_eq!(rc, E_NOENT);
    assert_eq!(a.reason, APKR_MANQUE);
    let (rc, a) = open_names(&["AndroidManifest.xml"]);
    assert_eq!(rc, E_NOENT);
    assert_eq!(a.reason, APKR_MANQUE);
    let mut man = ent("AndroidManifest.xml", b"m");
    man.reported = APK_ENTRY_MAX + 1;
    let (rc, a) = open_ents(vec![man, ent("classes.dex", b"d")], b"apk");
    assert_eq!(rc, E_RANGE);
    assert_eq!(a.reason, APKR_TAILLE);
    assert!(a.file.p.is_null());
}

#[test]
fn manifest_activity_and_label() {
    let _g = begin();
    let doc = tree(b"com.exemple", b".Principale", None, true);
    let (r, m) = manifest_of(&doc, Vec::new(), u32::MAX);
    assert_eq!(r, E_OK);
    assert_eq!(m.reason, APKR_OK);
    assert_eq!(z(&m.activity), b"com.exemple.Principale");
    assert_eq!(z(&m.activity_desc), b"Lcom/exemple/Principale;");
    assert_eq!(z(&m.label), b"com.exemple");
    assert_eq!(z(&m.version_name), b"1.2");
    assert_eq!(m.version_code, 0x2a);
    assert_eq!(m.min_sdk, 21);
    assert_eq!(m.target_sdk, 34);
    assert_eq!(m.perm_count, 1);
    assert_eq!(z(&m.perms[0]), b"android.permission.INTERNET");
    let doc = tree(b"com.exemple", b"Principale", None, false);
    let (r, m) = manifest_of(&doc, Vec::new(), u32::MAX);
    assert_eq!(r, E_OK);
    assert_eq!(z(&m.activity), b"com.exemple.Principale");
    assert_eq!(m.version_name[0], 0);
    let cafe = "Café".as_bytes();
    let doc = tree(
        b"com.exemple",
        b".Principale",
        Some(pay(LABEL, T_STR, b"label", 0, cafe)),
        true,
    );
    let (r, m) = manifest_of(&doc, Vec::new(), u32::MAX);
    assert_eq!(r, E_OK);
    assert_eq!(z(&m.label), cafe);
    let id = 0x7f01_0001u32;
    let doc = tree(
        b"com.exemple",
        b".Principale",
        Some(pay(LABEL, T_REF, b"label", id, b"")),
        true,
    );
    let extra = vec![ent("resources.arsc", &arsc_blob(id, b"Bonjour"))];
    let (r, m) = manifest_of(&doc, extra, u32::MAX);
    assert_eq!(r, E_OK);
    assert_eq!(z(&m.label), b"Bonjour");
    let doc = tree(b"com.exemple", b"com.Ex$A", None, true);
    let (r, m) = manifest_of(&doc, Vec::new(), u32::MAX);
    assert_eq!(r, E_OK);
    assert_eq!(z(&m.activity), b"com.Ex$A");
    assert_eq!(z(&m.activity_desc), b"Lcom/Ex$A;");
}

#[test]
fn manifest_rejections() {
    let _g = begin();
    let mut pkg = b"com.".to_vec();
    pkg.extend(std::iter::repeat_n(b'a', 66));
    assert_eq!(pkg.len(), 70);
    let doc = tree(&pkg, b".P", None, true);
    let (r, m) = manifest_of(&doc, Vec::new(), u32::MAX);
    assert_eq!(r, E_OK);
    assert_eq!(z(&m.label).len(), 63);
    assert_eq!(z(&m.label), &pkg[..63]);
    let mut b = b"VMAN".to_vec();
    ev_start(
        &mut b,
        b"manifest",
        &[pay(0, T_STR, b"package", 0, b"com.exemple")],
    );
    for i in 0..17 {
        let name = format!("android.permission.P{i}");
        ev_start(
            &mut b,
            b"uses-permission",
            &[pay(ANAME, T_STR, b"name", 0, name.as_bytes())],
        );
        ev_end(&mut b);
    }
    ev_end(&mut b);
    let (r, m) = manifest_of(&b, Vec::new(), u32::MAX);
    assert_eq!(r, E_RANGE);
    assert_eq!(m.reason, APKR_MANIFESTE);
    assert_eq!(m.package[0], 0);
    let doc = tree(b"com.bad$x", b".P", None, true);
    let (r, m) = manifest_of(&doc, Vec::new(), u32::MAX);
    assert_eq!(r, E_INVAL);
    assert_eq!(m.reason, APKR_PAQUET);
    assert_eq!(m.package[0], 0);
    let mut b = b"VMAN".to_vec();
    ev_start(
        &mut b,
        b"manifest",
        &[pay(0, T_STR, b"package", 0, b"com.exemple")],
    );
    ev_start(&mut b, b"application", &[]);
    ev_end(&mut b);
    ev_end(&mut b);
    let (r, m) = manifest_of(&b, Vec::new(), u32::MAX);
    assert_eq!(r, E_NOENT);
    assert_eq!(m.reason, APKR_ACTIVITE);
    let doc = tree(
        b"com.exemple",
        b".P",
        Some(pay(LABEL, T_STR, b"label", 0, b"a\x01b")),
        true,
    );
    let (r, m) = manifest_of(&doc, Vec::new(), u32::MAX);
    assert_eq!(r, E_INVAL);
    assert_eq!(m.reason, APKR_LIBELLE);
    assert_eq!(m.label[0], 0);
    let mut b = b"VMAN".to_vec();
    ev_start(
        &mut b,
        b"manifest",
        &[pay(0, T_STR, b"package", 0, b"com.exemple")],
    );
    ev_start(&mut b, b"application", &[]);
    ev_end(&mut b);
    ev_start(&mut b, b"application", &[]);
    ev_end(&mut b);
    ev_end(&mut b);
    let (r, m) = manifest_of(&b, Vec::new(), u32::MAX);
    assert_eq!(r, E_INVAL);
    assert_eq!(m.reason, APKR_MANIFESTE);
    let (r, m) = manifest_of(b"not-axml", Vec::new(), u32::MAX);
    assert_eq!(r, E_INVAL);
    assert_eq!(m.reason, APKR_MANIFESTE);
    let doc = tree(b"com.exemple", b".P", None, true);
    let (r, m) = manifest_of(&doc, Vec::new(), 0);
    assert_eq!(r, E_NOMEM);
    assert_eq!(m.reason, APKR_MEMOIRE);
    assert_eq!(m.package[0], 0);
    let id = 0x7f01_0001u32;
    let doc = tree(
        b"com.exemple",
        b".P",
        Some(pay(LABEL, T_REF, b"label", id, b"")),
        true,
    );
    let extra = vec![ent("resources.arsc", &arsc_blob(id, b"Bonjour"))];
    let (r, m) = manifest_of(&doc, extra, 1);
    assert_eq!(r, E_NOMEM);
    assert_eq!(m.reason, APKR_MEMOIRE);
    assert_eq!(m.package[0], 0);
    assert_eq!(alloc_live(), 0);
}

#[test]
fn verify_block_and_refusals() {
    let _g = begin();
    let parts = signed(APK_ALGO_RSA_PKCS1_SHA256, 1, 1, b"", false);
    let cd = usize::try_from(parts.cd_off).unwrap();
    assert_eq!(&parts.file[cd - 16..cd], b"APK Sig Block 42");
    let a = open_signed(&parts, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_OK);
    assert_eq!(sig.reason, APKR_OK);
    assert_eq!(sig.algo, APK_ALGO_RSA_PKCS1_SHA256);
    assert_eq!(sig.cert_sha256, toy_hash(CERT));
    let bad = signed(APK_ALGO_RSA_PKCS1_SHA256, 1, 1, b"", true);
    let a = open_signed(&bad, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    assert_eq!(sig.cert_sha256, [0; 32]);
    assert_eq!(sig.algo, 0);
    let magic = signed(APK_ALGO_RSA_PKCS1_SHA256, 1, 1, b"APK Sig Block 42", false);
    let a = open_signed(&magic, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    let two = signed(APK_ALGO_RSA_PKCS1_SHA256, 1, 2, b"", false);
    let a = open_signed(&two, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_NOTSUP);
    assert_eq!(sig.reason, APKR_SIGNATAIRES);
    let algo = signed(0x0104, 1, 1, b"", false);
    let a = open_signed(&algo, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_NOTSUP);
    assert_eq!(sig.reason, APKR_ALGO);
    let unknown = signed(0x9999, 1, 1, b"", false);
    let a = open_signed(&unknown, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    let none = assemble(&lp(&[]), b"");
    let a = open_signed(&none, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    let dup = signed(APK_ALGO_RSA_PKCS1_SHA256, 2, 1, b"", false);
    let a = open_signed(&dup, 0);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    let past = signed(APK_ALGO_RSA_PKCS1_SHA256, 1, 1, b"", false);
    let lfh = u32::try_from(past.prefix_len).unwrap();
    let a = open_signed(&past, lfh);
    let (rc, sig) = verify(&a);
    assert_eq!(rc, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    let (rc, a) = open_names(&["../x"]);
    assert_eq!(a.reason, APKR_NOM);
    let (v, sig) = verify(&a);
    assert_eq!(rc, E_INVAL);
    assert_eq!(v, E_ACCES);
    assert_eq!(sig.reason, APKR_SIGNATURE);
    assert_eq!(sig.cert_sha256, [0; 32]);
    assert_eq!(alloc_live(), 0);
}
