use core::ffi::{c_int, c_void};

use crate::ffi::{
    ct_equal, der_spki_rsa, rsa_pkcs1_sha256_verify, sha256, sha256_final, sha256_init,
    sha256_update, t_rsapub, t_sha256, x509_spki, zip_entry,
};
use crate::rd::{apk_lp, low32, rd32, rd32_at, rd64, rd64_at, uz};
use crate::{
    APK_ALGO_RSA_PKCS1_SHA256, APK_CHUNK, APK_EOCD_LEN, APK_EOCD_SIG, APK_MAGIC, APK_SIG_V2_ID,
    APKR_ALGO, APKR_OK, APKR_SIGNATAIRES, APKR_SIGNATURE, E_ACCES, E_INVAL, E_NOTSUP, E_OK,
    SHA256_LEN, t_apk, t_apksig, t_span, t_zipent,
};

struct Block<'a> {
    off: usize,
    eocd: usize,
    v2: &'a [u8],
}

struct Signer<'a> {
    signed_data: &'a [u8],
    digest: &'a [u8],
    cert: &'a [u8],
    sig: &'a [u8],
    pubkey: &'a [u8],
    found: bool,
    reason: c_int,
}

fn chunk_count(n: usize) -> u32 {
    let Some(sum) = n.checked_add(APK_CHUNK - 1) else {
        return u32::MAX;
    };
    u32::try_from(sum / APK_CHUNK).unwrap_or(u32::MAX)
}

fn put32(dst: &mut [u8], v: u32) -> bool {
    let Some(slot) = dst.get_mut(..4) else {
        return false;
    };
    slot.copy_from_slice(&v.to_le_bytes());
    true
}

fn sha_init(c: &mut t_sha256) {
    // SAFETY: c est un contexte exclusif, même non initialisé.
    unsafe { sha256_init(core::ptr::from_mut(c)) };
}

fn sha_update(c: &mut t_sha256, data: &[u8]) {
    // SAFETY: c est initialisé. data vit le temps de l'appel. n nul ne lit pas data.
    unsafe {
        sha256_update(
            core::ptr::from_mut(c),
            data.as_ptr().cast::<c_void>(),
            data.len(),
        )
    };
}

fn sha_final(c: &mut t_sha256, out: &mut [u8; SHA256_LEN]) {
    // SAFETY: c est initialisé. out couvre 32 octets.
    unsafe { sha256_final(core::ptr::from_mut(c), out.as_mut_ptr()) };
}

fn sha_once(data: &[u8], out: &mut [u8; SHA256_LEN]) {
    // SAFETY: data vit le temps de l'appel. out couvre 32 octets. n nul accepte un data vide.
    unsafe { sha256(data.as_ptr().cast::<c_void>(), data.len(), out.as_mut_ptr()) };
}

fn section_hash(top: &mut t_sha256, data: &[u8]) -> Result<(), c_int> {
    let mut off = 0usize;
    while off < data.len() {
        let n = (data.len() - off).min(APK_CHUNK);
        let Some(chunk) = data.get(off..off + n) else {
            return Err(E_ACCES);
        };
        let mut head = [0u8; 5];
        head[0] = 0xa5;
        if !put32(&mut head[1..], low32(n)) {
            return Err(E_ACCES);
        }
        let mut child = t_sha256::zero();
        let mut h = [0u8; SHA256_LEN];
        sha_init(&mut child);
        sha_update(&mut child, &head);
        sha_update(&mut child, chunk);
        sha_final(&mut child, &mut h);
        sha_update(top, &h);
        off += n;
    }
    Ok(())
}

fn eocd_hash(top: &mut t_sha256, eocd: &[u8], block_off: u32) -> Result<(), c_int> {
    if eocd.len() < APK_EOCD_LEN {
        return Err(E_ACCES);
    }
    let Some(head16) = eocd.get(..16) else {
        return Err(E_ACCES);
    };
    let Some(tail) = eocd.get(20..) else {
        return Err(E_ACCES);
    };
    let mut head = [0u8; 5];
    head[0] = 0xa5;
    if !put32(&mut head[1..], low32(eocd.len())) {
        return Err(E_ACCES);
    }
    let mut offb = [0u8; 4];
    if !put32(&mut offb, block_off) {
        return Err(E_ACCES);
    }
    let mut child = t_sha256::zero();
    let mut h = [0u8; SHA256_LEN];
    sha_init(&mut child);
    sha_update(&mut child, &head);
    sha_update(&mut child, head16);
    sha_update(&mut child, &offb);
    sha_update(&mut child, tail);
    sha_final(&mut child, &mut h);
    sha_update(top, &h);
    Ok(())
}

fn sig_digest(
    file: &[u8],
    block: &Block<'_>,
    cd_off: usize,
    cd_size: usize,
) -> Result<[u8; SHA256_LEN], c_int> {
    let Some(prefix) = file.get(..block.off) else {
        return Err(E_ACCES);
    };
    let cd_end = match cd_off.checked_add(cd_size) {
        Some(v) => v,
        None => return Err(E_ACCES),
    };
    let Some(cd) = file.get(cd_off..cd_end) else {
        return Err(E_ACCES);
    };
    let Some(eocd) = file.get(block.eocd..) else {
        return Err(E_ACCES);
    };
    let mut head = [0u8; 5];
    head[0] = 0x5a;
    let chunks = chunk_count(block.off)
        .wrapping_add(chunk_count(cd_size))
        .wrapping_add(1);
    if !put32(&mut head[1..], chunks) {
        return Err(E_ACCES);
    }
    let mut top = t_sha256::zero();
    sha_init(&mut top);
    sha_update(&mut top, &head);
    section_hash(&mut top, prefix)?;
    section_hash(&mut top, cd)?;
    eocd_hash(&mut top, eocd, low32(block.off))?;
    let mut out = [0u8; SHA256_LEN];
    sha_final(&mut top, &mut out);
    Ok(out)
}

fn sig_eocd(file: &[u8], cd_off: u32, cd_size: u32) -> Result<usize, c_int> {
    let pos = match uz(cd_off).checked_add(uz(cd_size)) {
        Some(v) => v,
        None => return Err(E_ACCES),
    };
    if pos > file.len() {
        return Err(E_ACCES);
    }
    let tail = file.len() - pos;
    if tail < APK_EOCD_LEN {
        return Err(E_ACCES);
    }
    let Some(sig) = rd32_at(file, pos) else {
        return Err(E_ACCES);
    };
    if sig != APK_EOCD_SIG {
        return Err(E_ACCES);
    }
    let Some(got_size) = rd32_at(file, pos + 12) else {
        return Err(E_ACCES);
    };
    let Some(got_off) = rd32_at(file, pos + 16) else {
        return Err(E_ACCES);
    };
    let Some(c0) = file.get(pos + 20).copied() else {
        return Err(E_ACCES);
    };
    let Some(c1) = file.get(pos + 21).copied() else {
        return Err(E_ACCES);
    };
    let comment = usize::from(c0) + (usize::from(c1) << 8);
    if got_size != cd_size || got_off != cd_off || comment != tail - APK_EOCD_LEN {
        return Err(E_ACCES);
    }
    let mut i = pos + APK_EOCD_LEN;
    while i.saturating_add(APK_MAGIC.len()) <= file.len() {
        let Some(win) = file.get(i..i + APK_MAGIC.len()) else {
            return Err(E_ACCES);
        };
        if win == APK_MAGIC {
            return Err(E_ACCES);
        }
        i += 1;
    }
    Ok(pos)
}

fn sig_pairs(mut input: &[u8]) -> Result<&[u8], c_int> {
    let mut found = 0u32;
    let mut v2: &[u8] = &[];
    while input.len() >= 12 {
        let Some(n) = rd64(input) else {
            return Err(E_ACCES);
        };
        let avail = input.len() - 8;
        let Ok(avail_u) = u64::try_from(avail) else {
            return Err(E_ACCES);
        };
        if n < 4 || n > avail_u {
            return Err(E_ACCES);
        }
        let Ok(n_us) = usize::try_from(n) else {
            return Err(E_ACCES);
        };
        let Some(tail) = input.get(8..) else {
            return Err(E_ACCES);
        };
        let Some(id) = rd32(tail) else {
            return Err(E_ACCES);
        };
        if id == APK_SIG_V2_ID {
            let Some(value) = input.get(12..8 + n_us) else {
                return Err(E_ACCES);
            };
            v2 = value;
            found = found.wrapping_add(1);
        }
        let Some(rest) = input.get(8 + n_us..) else {
            return Err(E_ACCES);
        };
        input = rest;
    }
    if !input.is_empty() || found != 1 {
        return Err(E_ACCES);
    }
    Ok(v2)
}

fn entries_before(apk: &t_apk, limit: usize) -> c_int {
    let mut i = 0u32;
    while i < apk.zip.count {
        let mut e = t_zipent::zero();
        // SAFETY: apk.zip reste lisible. e est exclusif. L'indice est inférieur à count.
        let r = unsafe {
            zip_entry(
                core::ptr::from_ref(&apk.zip),
                i,
                core::ptr::from_mut(&mut e),
            )
        };
        if r < 0 {
            return E_ACCES;
        }
        let lfh = uz(e.lfh_off);
        let data = uz(e.data_off);
        let csize = uz(e.csize);
        if lfh >= limit || data > limit {
            return E_ACCES;
        }
        let Some(room) = limit.checked_sub(data) else {
            return E_ACCES;
        };
        if csize > room {
            return E_ACCES;
        }
        i += 1;
    }
    E_OK
}

fn sig_locate<'a>(apk: &'a t_apk, file: &'a [u8]) -> Result<Block<'a>, c_int> {
    let cd = uz(apk.zip.cd_off);
    let eocd = sig_eocd(file, apk.zip.cd_off, apk.zip.cd_size)?;
    if cd > file.len() || cd < 32 {
        return Err(E_ACCES);
    }
    let Some(magic) = file.get(cd - APK_MAGIC.len()..cd) else {
        return Err(E_ACCES);
    };
    if magic != APK_MAGIC {
        return Err(E_ACCES);
    }
    let Some(size) = rd64_at(file, cd - 24) else {
        return Err(E_ACCES);
    };
    let Ok(limit) = u64::try_from(cd - 8) else {
        return Err(E_ACCES);
    };
    if size < 24 || size > limit {
        return Err(E_ACCES);
    }
    let Ok(size_us) = usize::try_from(size) else {
        return Err(E_ACCES);
    };
    let off = cd - 8 - size_us;
    let Some(first) = rd64_at(file, off) else {
        return Err(E_ACCES);
    };
    if first != size {
        return Err(E_ACCES);
    }
    let Some(start) = off.checked_add(8) else {
        return Err(E_ACCES);
    };
    let Some(pair_len) = size_us.checked_sub(24) else {
        return Err(E_ACCES);
    };
    let Some(pair_end) = start.checked_add(pair_len) else {
        return Err(E_ACCES);
    };
    let Some(pairs) = file.get(off + 8..pair_end) else {
        return Err(E_ACCES);
    };
    let v2 = sig_pairs(pairs)?;
    let r = entries_before(apk, off);
    if r < 0 {
        return Err(r);
    }
    Ok(Block { off, eocd, v2 })
}

fn algo_known(id: u32) -> bool {
    (0x0101..=0x0104).contains(&id) || id == 0x0201 || id == 0x0202 || id == 0x0301
}

fn sig_item(item: &[u8]) -> Result<(u32, &[u8]), c_int> {
    if item.len() < 8 {
        return Err(E_ACCES);
    }
    let Some(algo) = rd32(item) else {
        return Err(E_ACCES);
    };
    let mut rest = &item[4..];
    let value = apk_lp(&mut rest)?;
    if !rest.is_empty() || !algo_known(algo) {
        return Err(E_ACCES);
    }
    Ok((algo, value))
}

fn sig_algos<'a>(
    mut digests: &'a [u8],
    mut sigs: &'a [u8],
    s: &mut Signer<'a>,
) -> Result<(), c_int> {
    if digests.is_empty() {
        return Err(E_ACCES);
    }
    while !digests.is_empty() || !sigs.is_empty() {
        let d = apk_lp(&mut digests)?;
        let g = apk_lp(&mut sigs)?;
        let (ad, d) = sig_item(d)?;
        let (ag, g) = sig_item(g)?;
        if ad != ag {
            return Err(E_ACCES);
        }
        if ad == APK_ALGO_RSA_PKCS1_SHA256 && s.found {
            return Err(E_ACCES);
        }
        if ad == APK_ALGO_RSA_PKCS1_SHA256 {
            s.digest = d;
            s.sig = g;
            s.found = true;
        }
    }
    Ok(())
}

fn sig_signed<'a>(s: &mut Signer<'a>, sigs: &'a [u8]) -> Result<(), c_int> {
    let mut sd = s.signed_data;
    let digests = apk_lp(&mut sd)?;
    let mut certs = apk_lp(&mut sd)?;
    let mut attrs = apk_lp(&mut sd)?;
    s.cert = apk_lp(&mut certs)?;
    while !certs.is_empty() {
        let _extra = apk_lp(&mut certs)?;
    }
    while !attrs.is_empty() {
        let extra = apk_lp(&mut attrs)?;
        if extra.len() < 4 {
            return Err(E_ACCES);
        }
    }
    sig_algos(digests, sigs, s)?;
    if !s.found {
        s.reason = APKR_ALGO;
        return Err(E_NOTSUP);
    }
    Ok(())
}

fn sig_signer(mut v2: &[u8]) -> (Signer<'_>, c_int) {
    let mut s = Signer {
        signed_data: &[],
        digest: &[],
        cert: &[],
        sig: &[],
        pubkey: &[],
        found: false,
        reason: APKR_SIGNATURE,
    };
    let mut signers = match apk_lp(&mut v2) {
        Ok(v) => v,
        Err(e) => return (s, e),
    };
    if !v2.is_empty() {
        return (s, E_ACCES);
    }
    let mut one = match apk_lp(&mut signers) {
        Ok(v) => v,
        Err(e) => return (s, e),
    };
    if !signers.is_empty() {
        s.reason = APKR_SIGNATAIRES;
        return (s, E_NOTSUP);
    }
    s.signed_data = match apk_lp(&mut one) {
        Ok(v) => v,
        Err(e) => return (s, e),
    };
    let sigs = match apk_lp(&mut one) {
        Ok(v) => v,
        Err(e) => return (s, e),
    };
    s.pubkey = match apk_lp(&mut one) {
        Ok(v) => v,
        Err(e) => return (s, e),
    };
    if !one.is_empty() {
        return (s, E_ACCES);
    }
    match sig_signed(&mut s, sigs) {
        Ok(()) => (s, E_OK),
        Err(e) => (s, e),
    }
}

fn same_bytes(a: &[u8], p: *const u8, n: usize) -> bool {
    if a.len() != n {
        return false;
    }
    if n == 0 {
        return true;
    }
    if p.is_null() || n > isize::MAX as usize {
        return false;
    }
    // SAFETY: p est non nul et couvre n octets, n égal à a.len().
    let b = unsafe { core::slice::from_raw_parts(p, n) };
    a == b
}

fn sig_check(file: &[u8], apk: &t_apk, block: &Block<'_>, s: &Signer<'_>) -> c_int {
    if s.digest.len() != SHA256_LEN {
        return E_ACCES;
    }
    let cd_off = uz(apk.zip.cd_off);
    let cd_size = uz(apk.zip.cd_size);
    let h = match sig_digest(file, block, cd_off, cd_size) {
        Ok(v) => v,
        Err(e) => return e,
    };
    // SAFETY: les deux tampons couvrent SHA256_LEN octets.
    let eq = unsafe {
        ct_equal(
            h.as_ptr().cast::<c_void>(),
            s.digest.as_ptr().cast::<c_void>(),
            SHA256_LEN,
        )
    };
    if eq == 0 {
        return E_ACCES;
    }
    let cert = t_span {
        p: s.cert.as_ptr(),
        len: s.cert.len(),
    };
    let mut spki = t_span::zero();
    // SAFETY: cert vise les octets du certificat. spki est exclusif.
    let rc = unsafe { x509_spki(cert, core::ptr::from_mut(&mut spki)) };
    if rc < 0 || !same_bytes(s.pubkey, spki.p, spki.len) {
        return E_ACCES;
    }
    let mut key = t_rsapub::zero();
    let pub_span = t_span {
        p: s.pubkey.as_ptr(),
        len: s.pubkey.len(),
    };
    // SAFETY: pub_span vise la clé publique. key est exclusif.
    let rc = unsafe { der_spki_rsa(pub_span, core::ptr::from_mut(&mut key)) };
    if rc < 0 || s.sig.len() != uz(key.nlen) {
        return E_ACCES;
    }
    let mut dig = [0u8; SHA256_LEN];
    sha_once(s.signed_data, &mut dig);
    let sig = t_span {
        p: s.sig.as_ptr(),
        len: s.sig.len(),
    };
    // SAFETY: key est initialisé. sig vise la signature. dig couvre 32 octets.
    let rc = unsafe { rsa_pkcs1_sha256_verify(core::ptr::from_ref(&key), sig, dig.as_ptr()) };
    if rc != 0 {
        return E_ACCES;
    }
    E_OK
}

fn file_bytes(apk: &t_apk) -> Result<&[u8], c_int> {
    if apk.file.p.is_null() || apk.file.len > isize::MAX as usize {
        return Err(E_ACCES);
    }
    if apk.file.len == 0 {
        return Ok(&[]);
    }
    // SAFETY: apk_open a gardé file.p pour file.len octets, encore valides ici.
    Ok(unsafe { core::slice::from_raw_parts(apk.file.p, apk.file.len) })
}

/// # Safety
/// `a` est nul, ou bien aligné, avec `a.file` encore lisible si `a.reason` vaut `APKR_OK`.
/// `out` est nul, ou bien aligné et exclusif. Un refus met `out.reason` et laisse l'empreinte à zéro.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn apk_verify(a: *const t_apk, out: *mut t_apksig) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: out est non nul et aligné, exclusif le temps de l'appel.
    let out = unsafe { &mut *out };
    *out = t_apksig::zero();
    out.reason = APKR_SIGNATURE;
    if a.is_null() || !a.is_aligned() {
        return E_ACCES;
    }
    // SAFETY: a est non nul et aligné, lecture seule le temps de l'appel.
    let a = unsafe { &*a };
    if a.reason != APKR_OK || a.file.p.is_null() {
        return E_ACCES;
    }
    let file = match file_bytes(a) {
        Ok(v) => v,
        Err(e) => return e,
    };
    let block = match sig_locate(a, file) {
        Ok(v) => v,
        Err(e) => return e,
    };
    let (signer, r) = sig_signer(block.v2);
    if r < 0 {
        out.reason = signer.reason;
        return r;
    }
    let r = sig_check(file, a, &block, &signer);
    if r < 0 {
        out.reason = signer.reason;
        return r;
    }
    sha_once(signer.cert, &mut out.cert_sha256);
    out.algo = APK_ALGO_RSA_PKCS1_SHA256;
    out.reason = APKR_OK;
    E_OK
}
