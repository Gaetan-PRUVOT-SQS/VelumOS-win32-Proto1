use core::ffi::{c_char, c_int, c_void};
use core::sync::atomic::Ordering;
use std::sync::{Mutex, MutexGuard};

use crate::check::{
    pm_act_ok, pm_dec_u32, pm_hex_cert, pm_info_ok, pm_label_ok, pm_nlen, pm_pkg_ok, pm_same_pkg,
    pm_utf8_ok,
};
use crate::mem::{ALLOC_CALLS, ALLOC_LIVE, reset_alloc};
use crate::ops::{pm_find, pm_install, pm_list, pm_remove};
use crate::text::{
    pm_defaults, pm_index, pm_path, pm_registry_format, pm_registry_parse, pm_registry_scan,
};
use crate::{
    E_ACCES, E_EXIST, E_INVAL, E_NOENT, E_NOMEM, E_NOSPC, E_NOTSUP, E_OVERFLOW, E_PERM, E_RANGE,
    PM_ACT_MAX, PM_APK_MAX, PM_CERT_LEN, PM_LABEL_MAX, PM_MAX_PACKAGES, PM_ORIGIN_DATA,
    PM_ORIGIN_SYSTEM, PM_PATH_MAX, PM_PKG_MAX, PM_REG_MAX, PM_ROOT_MAX, PmInspect, PmMkdir, PmRead,
    PmRename, PmUnlink, PmWrite, t_pm, t_pmentry, t_pmfs, t_pminfo, t_pmreg, t_span, t_text,
};

const HEX: &str = "0123456789abcDEF0123456789abcDEF0123456789abcdef0123456789abcdef";
const SYS: &[u8] = b"/system/apps\0";
const USR: &[u8] = b"/data/apps\0";
const REG: &str = "/data/apps/paquets";
const SYSREG: &str = "/system/apps/paquets";
const FAKE_FILES: usize = 80;
const FAKE_DATA: usize = 16384;

static GATE: Mutex<()> = Mutex::new(());
static FAKE: Mutex<Fake> = Mutex::new(Fake {
    files: Vec::new(),
    ops: 0,
    fail_at: 0,
    crash: false,
    insp: t_pminfo::zero(),
    insp_err: 0,
});

struct File {
    path: String,
    data: Vec<u8>,
}

struct Fake {
    files: Vec<File>,
    ops: i32,
    fail_at: i32,
    crash: bool,
    insp: t_pminfo,
    insp_err: c_int,
}

fn lock() -> MutexGuard<'static, ()> {
    GATE.lock().unwrap_or_else(|e| e.into_inner())
}

fn with_fake<T>(f: impl FnOnce(&mut Fake) -> T) -> T {
    let mut g = FAKE.lock().unwrap_or_else(|e| e.into_inner());
    f(&mut g)
}

impl Fake {
    fn clear(&mut self) {
        self.files.clear();
        self.ops = 0;
        self.fail_at = 0;
        self.crash = false;
        self.insp = t_pminfo::zero();
        self.insp_err = 0;
    }

    fn tick(&mut self) -> c_int {
        self.ops += 1;
        if self.fail_at > 0 && (self.ops == self.fail_at || (self.crash && self.ops > self.fail_at))
        {
            return E_IO_TEST;
        }
        0
    }

    fn get(&self, path: &str) -> Option<&File> {
        self.files.iter().find(|f| f.path == path)
    }

    fn put(&mut self, path: &str, data: &[u8]) -> c_int {
        if data.len() > FAKE_DATA || path.len() >= PM_PATH_MAX {
            return E_NOSPC;
        }
        if let Some(f) = self.files.iter_mut().find(|f| f.path == path) {
            f.data.clear();
            f.data.extend_from_slice(data);
            return 0;
        }
        if self.files.len() >= FAKE_FILES {
            return E_NOSPC;
        }
        self.files.push(File {
            path: path.to_string(),
            data: data.to_vec(),
        });
        0
    }

    fn rename(&mut self, from: &str, to: &str) -> c_int {
        if self.tick() < 0 {
            return E_IO_TEST;
        }
        if !self.files.iter().any(|f| f.path == from) {
            return E_NOENT;
        }
        self.files.retain(|f| f.path != to || f.path == from);
        if let Some(f) = self.files.iter_mut().find(|f| f.path == from) {
            f.path = to.to_string();
            0
        } else {
            E_NOENT
        }
    }

    fn unlink(&mut self, path: &str) -> c_int {
        if self.tick() < 0 {
            return E_IO_TEST;
        }
        let n = self.files.len();
        self.files.retain(|f| f.path != path);
        if self.files.len() == n { E_NOENT } else { 0 }
    }
}

const E_IO_TEST: c_int = -5;

fn cstr(p: *const c_char) -> Option<String> {
    if p.is_null() {
        return None;
    }
    let mut v = Vec::new();
    let mut i = 0usize;
    while i < 4096 {
        // SAFETY: i < 4096 et p non nul. Les chemins testés sont terminés par NUL avant.
        let c = unsafe { p.add(i).read() };
        if c == 0 {
            return Some(String::from_utf8_lossy(&v).into_owned());
        }
        v.push(c as u8);
        i += 1;
    }
    None
}

fn bytes_of(data: t_span) -> Vec<u8> {
    if data.len == 0 {
        return Vec::new();
    }
    if data.p.is_null() || data.len > isize::MAX as usize {
        return Vec::new();
    }
    // SAFETY: p non nul, len borné, vivant le temps du callback.
    unsafe { core::slice::from_raw_parts(data.p, data.len).to_vec() }
}

unsafe extern "C" fn f_read(_ctx: *mut c_void, path: *const c_char, out: t_text) -> i64 {
    let Some(path) = cstr(path) else {
        return i64::from(E_INVAL);
    };
    with_fake(|f| {
        if f.tick() < 0 {
            return i64::from(E_IO_TEST);
        }
        let Some(file) = f.files.iter().find(|x| x.path == path) else {
            return i64::from(E_NOENT);
        };
        if file.data.len() > out.cap {
            return i64::from(E_OVERFLOW);
        }
        if !file.data.is_empty() {
            if out.p.is_null() {
                return i64::from(E_INVAL);
            }
            // SAFETY: out.p non nul, file.data.len() <= cap.
            unsafe {
                core::ptr::copy_nonoverlapping(file.data.as_ptr(), out.p.cast(), file.data.len());
            }
        }
        file.data.len() as i64
    })
}

unsafe extern "C" fn f_write(_ctx: *mut c_void, path: *const c_char, data: t_span) -> c_int {
    let Some(path) = cstr(path) else {
        return E_INVAL;
    };
    let bytes = bytes_of(data);
    with_fake(|f| {
        if f.tick() < 0 {
            let n = bytes.len() / 2;
            let _ = f.put(&path, &bytes[..n]);
            return E_IO_TEST;
        }
        f.put(&path, &bytes)
    })
}

unsafe extern "C" fn f_rename(_ctx: *mut c_void, from: *const c_char, to: *const c_char) -> c_int {
    let (Some(from), Some(to)) = (cstr(from), cstr(to)) else {
        return E_INVAL;
    };
    with_fake(|f| f.rename(&from, &to))
}

unsafe extern "C" fn f_unlink(_ctx: *mut c_void, path: *const c_char) -> c_int {
    let Some(path) = cstr(path) else {
        return E_INVAL;
    };
    with_fake(|f| f.unlink(&path))
}

unsafe extern "C" fn f_mkdir(_ctx: *mut c_void, _path: *const c_char) -> c_int {
    with_fake(|f| if f.tick() < 0 { E_IO_TEST } else { E_EXIST })
}

unsafe extern "C" fn f_inspect(_apk: t_span, out: *mut t_pminfo) -> c_int {
    with_fake(|f| {
        if f.insp_err < 0 {
            return f.insp_err;
        }
        if out.is_null() || !out.is_aligned() {
            return E_INVAL;
        }
        // SAFETY: out non nul et aligné, exclusif le temps du callback.
        unsafe { out.write(f.insp) };
        0
    })
}

fn table() -> t_pmfs {
    t_pmfs {
        read: Some(f_read as PmRead),
        write_new: Some(f_write as PmWrite),
        rename: Some(f_rename as PmRename),
        unlink: Some(f_unlink as PmUnlink),
        mkdir: Some(f_mkdir as PmMkdir),
    }
}

static FS_TABLE: Mutex<t_pmfs> = Mutex::new(t_pmfs {
    read: None,
    write_new: None,
    rename: None,
    unlink: None,
    mkdir: None,
});

fn fs_ptr() -> *const t_pmfs {
    let mut g = FS_TABLE.lock().unwrap_or_else(|e| e.into_inner());
    *g = table();
    &raw const *g
}

fn calm() {
    reset_alloc(0);
    ALLOC_LIVE.store(0, Ordering::SeqCst);
}

fn setup(pm: &mut t_pm) {
    with_fake(Fake::clear);
    calm();
    *pm = t_pm {
        sys_root: SYS.as_ptr().cast(),
        data_root: USR.as_ptr().cast(),
        fs: fs_ptr(),
        inspect: Some(f_inspect as PmInspect),
        ctx: core::ptr::null_mut(),
        max_packages: 0,
        max_apk: 0,
        max_registry: 0,
    };
    // SAFETY: pm est exclusif et aligné.
    unsafe { pm_defaults(pm) };
}

fn put_file(path: &str, data: &[u8]) -> c_int {
    with_fake(|f| f.put(path, data))
}

fn file_data(path: &str) -> Option<Vec<u8>> {
    with_fake(|f| f.get(path).map(|x| x.data.clone()))
}

fn reg_text() -> Vec<u8> {
    file_data(REG).unwrap_or_default()
}

fn fake_app(pkg: &str, version: u32, signer: u8) {
    with_fake(|f| {
        f.insp = t_pminfo::zero();
        let b = pkg.as_bytes();
        let n = b.len().min(PM_PKG_MAX - 1);
        f.insp.package[..n].copy_from_slice(&b[..n]);
        let lab = b"Appli";
        f.insp.label[..lab.len()].copy_from_slice(lab);
        let act = b"Lx/y/Main;";
        f.insp.activity[..act.len()].copy_from_slice(act);
        f.insp.cert = [signer; PM_CERT_LEN];
        f.insp.version_code = version;
    });
}

fn fake_install(pm: &mut t_pm, pkg: &str, v: u32, s: u8) -> c_int {
    fake_app(pkg, v, s);
    let mut body = [s; 8];
    body[0] = u8::try_from(v & 0xff).unwrap_or(0);
    let apk = t_span {
        p: body.as_ptr(),
        len: body.len(),
    };
    // SAFETY: pm exclusif, apk décrit body, out nul accepté.
    unsafe { pm_install(pm, apk, core::ptr::null_mut()) }
}

fn listed_ok(pm: &mut t_pm) -> c_int {
    with_fake(|f| f.fail_at = 0);
    let mut e = [t_pmentry::zero(); PM_MAX_PACKAGES as usize];
    // SAFETY: pm exclusif, e a PM_MAX_PACKAGES places.
    let n = unsafe { pm_list(pm, e.as_mut_ptr(), PM_MAX_PACKAGES) };
    if n < 0 {
        return n;
    }
    let count = usize::try_from(n).unwrap_or(0);
    for ent in e.iter().take(count) {
        if ent.origin == PM_ORIGIN_DATA {
            let path = cstr_buf(&ent.path);
            if file_data(&path).is_none() {
                return -1;
            }
        }
    }
    n
}

fn cstr_buf(buf: &[u8]) -> String {
    let n = buf.iter().position(|c| *c == 0).unwrap_or(buf.len());
    String::from_utf8_lossy(&buf[..n]).into_owned()
}

fn lit(s: &str) -> Vec<u8> {
    let mut v = s.as_bytes().to_vec();
    v.push(0);
    v
}

fn span_of(text: &[u8]) -> t_span {
    t_span {
        p: if text.is_empty() {
            core::ptr::null()
        } else {
            text.as_ptr()
        },
        len: text.len(),
    }
}

fn parse_at(text: &[u8], e: &mut [t_pmentry]) -> c_int {
    let s = span_of(text);
    // SAFETY: s décrit text (nul si vide). e est exclusif, sa longueur est max.
    unsafe { pm_registry_parse(s, e.as_mut_ptr(), u32::try_from(e.len()).unwrap_or(0)) }
}

fn scan_at(text: &[u8], e: &mut [t_pmentry], max: u32) -> (c_int, u32) {
    let mut r = t_pmreg {
        e: if max == 0 {
            core::ptr::null_mut()
        } else {
            e.as_mut_ptr()
        },
        max,
        n: 99,
        rejected: 99,
    };
    let s = span_of(text);
    // SAFETY: r exclusif. max 0 laisse e nul. text vit le temps de l'appel.
    let n = unsafe { pm_registry_scan(s, &mut r) };
    (n, r.rejected)
}

fn format_at(e: &[t_pmentry], n: u32, buf: &mut [u8]) -> c_int {
    let out = t_text {
        p: buf.as_mut_ptr().cast(),
        cap: buf.len(),
    };
    let ptr = if n == 0 {
        core::ptr::null()
    } else {
        e.as_ptr()
    };
    // SAFETY: buf exclusif. ptr nul seulement si n vaut 0.
    unsafe { pm_registry_format(ptr, n, out) }
}

fn ok_bytes(p: &[u8], v: &[u8], l: &[u8], a: &[u8]) -> bool {
    let mut line = Vec::new();
    line.extend_from_slice(p);
    line.push(b';');
    line.extend_from_slice(v);
    line.push(b';');
    line.extend_from_slice(l);
    line.push(b';');
    line.extend_from_slice(a);
    line.push(b';');
    line.extend_from_slice(HEX.as_bytes());
    line.push(b'\n');
    let mut e = [t_pmentry::zero(); 2];
    parse_at(&line, &mut e) == 1
}

fn fake_ok(p: &str, v: &str, l: &str, a: &str) -> bool {
    ok_bytes(p.as_bytes(), v.as_bytes(), l.as_bytes(), a.as_bytes())
}

fn fake_rep(n: usize, tail: &str) -> String {
    let mut s = "a".repeat(n);
    s.push_str(tail);
    s
}

fn base() -> Vec<u8> {
    let cafe = "Caf\u{00e9} \u{1f600}";
    format!(
        "com.a;1;Bonjour;Lcom/a/Main;;{HEX}\norg.b_2.c;4294967295;{cafe};Lo/B$1;;{HEX}\r\n\nx.y;0;;Lx/Y;;{HEX}\n"
    )
    .into_bytes()
}

fn rnd(seed: &mut u32) -> u32 {
    *seed ^= seed.wrapping_shl(13);
    *seed ^= seed.wrapping_shr(17);
    *seed ^= seed.wrapping_shl(5);
    *seed
}

fn check_text(src: &[u8]) {
    let mut e = [t_pmentry::zero(); 8];
    let n = parse_at(src, &mut e);
    assert!((0..=8).contains(&n), "analyse bornee");
    let count = u32::try_from(n).unwrap_or(0);
    let mut buf = [0u8; 4096];
    let wrote = format_at(&e, count, &mut buf);
    assert!(wrote >= 0);
    let len = usize::try_from(wrote).unwrap_or(0);
    let mut back = [t_pmentry::zero(); 8];
    assert_eq!(parse_at(&buf[..len], &mut back), n);
}

#[test]
fn tailles_abi() {
    use core::mem::{align_of, offset_of, size_of};
    assert_eq!(size_of::<usize>(), 8);
    assert_eq!(size_of::<crate::t_span>(), 16);
    assert_eq!(align_of::<crate::t_span>(), 8);
    assert_eq!(offset_of!(crate::t_span, len), 8);
    assert_eq!(size_of::<t_text>(), 16);
    assert_eq!(offset_of!(t_text, cap), 8);
    assert_eq!(size_of::<t_pminfo>(), 420);
    assert_eq!(align_of::<t_pminfo>(), 4);
    assert_eq!(offset_of!(t_pminfo, version_code), 128);
    assert_eq!(offset_of!(t_pminfo, label), 132);
    assert_eq!(offset_of!(t_pminfo, activity), 196);
    assert_eq!(offset_of!(t_pminfo, cert), 388);
    assert_eq!(size_of::<t_pmentry>(), 616);
    assert_eq!(offset_of!(t_pmentry, origin), 420);
    assert_eq!(offset_of!(t_pmentry, path), 424);
    assert_eq!(size_of::<t_pmreg>(), 24);
    assert_eq!(align_of::<t_pmreg>(), 8);
    assert_eq!(offset_of!(t_pmreg, rejected), 16);
    assert_eq!(size_of::<t_pmfs>(), 40);
    assert_eq!(size_of::<t_pm>(), 56);
    assert_eq!(offset_of!(t_pm, inspect), 24);
    assert_eq!(offset_of!(t_pm, max_registry), 48);
    assert_eq!(size_of::<crate::t_pmwork>(), 146_208);
    assert_eq!(align_of::<crate::t_pmwork>(), 4);
    assert_eq!(offset_of!(crate::t_pmwork, data), 39_424);
    assert_eq!(offset_of!(crate::t_pmwork, text), 79_900);
    assert_eq!(offset_of!(crate::t_pmwork, apk_tmp), 146_013);
}

#[test]
fn champs_paquet_version_libelle_activite() {
    assert!(fake_ok("a.b", "1", "L", "La/B;"));
    assert!(!fake_ok("", "1", "L", "La/B;"));
    assert!(!fake_ok("ab", "1", "L", "La/B;"));
    assert!(!fake_ok("a..b", "1", "L", "La/B;"));
    assert!(!fake_ok(".a.b", "1", "L", "La/B;"));
    assert!(!fake_ok("a.b.", "1", "L", "La/B;"));
    assert!(!fake_ok("a/b.c", "1", "L", "La/B;"));
    assert!(!fake_ok("a b.c", "1", "L", "La/B;"));
    assert!(!fake_ok("a\u{00e9}.c", "1", "L", "La/B;"));
    assert!(!fake_ok("a\u{0001}.c", "1", "L", "La/B;"));
    assert!(fake_ok("A_z.09", "1", "L", "La/B;"));
    assert!(fake_ok(&fake_rep(125, ".b"), "1", "L", "La/B;"));
    assert!(!fake_ok(&fake_rep(126, ".b"), "1", "L", "La/B;"));
    assert!(fake_ok("a.b", "0", "L", "La/B;"));
    assert!(fake_ok("a.b", "4294967295", "L", "La/B;"));
    assert!(!fake_ok("a.b", "4294967296", "L", "La/B;"));
    assert!(!fake_ok("a.b", "99999999999", "L", "La/B;"));
    assert!(!fake_ok("a.b", "", "L", "La/B;"));
    assert!(!fake_ok("a.b", "01", "L", "La/B;"));
    assert!(!fake_ok("a.b", "-1", "L", "La/B;"));
    assert!(!fake_ok("a.b", "+1", "L", "La/B;"));
    assert!(!fake_ok("a.b", " 1", "L", "La/B;"));
    assert!(!fake_ok("a.b", "1a", "L", "La/B;"));
    assert!(!fake_ok("a.b", "0x10", "L", "La/B;"));
    assert!(fake_ok("a.b", "1", "", "La/B;"));
    assert!(fake_ok("a.b", "1", &fake_rep(63, ""), "La/B;"));
    assert!(!fake_ok("a.b", "1", &fake_rep(64, ""), "La/B;"));
    assert!(!fake_ok("a.b", "1", "a;b", "La/B;"));
    assert!(!fake_ok("a.b", "1", "a\u{0001}", "La/B;"));
    assert!(!fake_ok("a.b", "1", "a\tb", "La/B;"));
    assert!(!fake_ok("a.b", "1", "a\u{007f}", "La/B;"));
    assert!(fake_ok("a.b", "1", "\u{00e9}t\u{00e9}", "La/B;"));
    assert!(fake_ok("a.b", "1", "\u{1f600}", "La/B;"));
    assert!(!ok_bytes(b"a.b", b"1", &[0xc3], b"La/B;"));
    assert!(!ok_bytes(b"a.b", b"1", &[0xc0, 0xaf], b"La/B;"));
    assert!(!ok_bytes(b"a.b", b"1", &[0xed, 0xa0, 0x80], b"La/B;"));
    assert!(!ok_bytes(b"a.b", b"1", &[0xf4, 0x90, 0x80, 0x80], b"La/B;"));
    assert!(!ok_bytes(b"a.b", b"1", &[0x80], b"La/B;"));
    assert!(fake_ok("a.b", "1", "L", "La;"));
    assert!(fake_ok("a.b", "1", "L", "La/B$1_c;"));
    assert!(!fake_ok("a.b", "1", "L", "L;"));
    assert!(!fake_ok("a.b", "1", "L", ""));
    assert!(!fake_ok("a.b", "1", "L", "a/B;"));
    assert!(!fake_ok("a.b", "1", "L", "La/B"));
    assert!(!fake_ok("a.b", "1", "L", "La.B;"));
    assert!(!fake_ok("a.b", "1", "L", "La;B;"));
    assert!(!fake_ok("a.b", "1", "L", "La B;"));
    let mut long = fake_rep(190, ";");
    long.replace_range(0..1, "L");
    assert_eq!(long.len(), 191);
    assert!(fake_ok("a.b", "1", "L", &long));
    let mut too = fake_rep(191, ";");
    too.replace_range(0..1, "L");
    assert_eq!(too.len(), 192);
    assert!(!fake_ok("a.b", "1", "L", &too));
}

#[test]
fn registre_lignes_doublons_aller_retour() {
    assert_eq!(HEX.len(), 64);
    let text = format!("a.b;7;L;La/B;;{HEX}\r\n\r\n\nmauvaise ligne\nc.d;2;M;Lc/D;;{HEX}");
    let mut e = [t_pmentry::zero(); 8];
    let (n, bad) = scan_at(text.as_bytes(), &mut e, 8);
    assert_eq!(n, 2);
    assert_eq!(bad, 1);
    assert_eq!(cstr_buf(&e[0].info.package), "a.b");
    assert_eq!(e[0].info.version_code, 7);
    assert_eq!(cstr_buf(&e[0].info.label), "L");
    assert_eq!(cstr_buf(&e[0].info.activity), "La/B;");
    assert_eq!(e[0].info.cert[0], 0x01);
    assert_eq!(e[0].info.cert[7], 0xef);
    assert_eq!(cstr_buf(&e[1].info.package), "c.d");
    let (n, bad) = scan_at(b"", &mut e, 8);
    assert_eq!(n, 0);
    assert_eq!(bad, 0);
    let (n, bad) = scan_at(b"\n\r\n\n", &mut e, 8);
    assert_eq!(n, 0);
    assert_eq!(bad, 0);
    let (n, bad) = scan_at(b"a.b;1;\x01\xff\n;;;;\n", &mut e, 8);
    assert_eq!(n, 0);
    assert_eq!(bad, 2);
    let dups = format!(
        "a.b;1;L;La/B;;{HEX}\na.b;2;L;La/B;;{HEX}\nA.B;3;L;La/B;;{HEX}\nc.d;4;L;La/B;;{HEX}\ne.f;5;L;La/B;;{HEX}\n"
    );
    let (n, bad) = scan_at(dups.as_bytes(), &mut e, 8);
    assert_eq!(n, 3);
    assert_eq!(bad, 2);
    assert_eq!(e[0].info.version_code, 1);
    let (n, bad) = scan_at(dups.as_bytes(), &mut e, 2);
    assert_eq!(n, 2);
    assert_eq!(bad, 3);
    let (n, _) = scan_at(dups.as_bytes(), &mut e, 0);
    assert_eq!(n, 0);
    let bad_cert = format!(
        "a.b;1;L;La/B;;{}\nc.d;1;L;La/B;;{HEX}a\ne.f;1;L;La/B;;g{}\n",
        &HEX[..63],
        &HEX[..63]
    );
    let (n, bad) = scan_at(bad_cert.as_bytes(), &mut e, 8);
    assert_eq!(n, 0);
    assert_eq!(bad, 3);
    let raw = base();
    let mut got = [t_pmentry::zero(); 8];
    assert_eq!(parse_at(&raw, &mut got), 3);
    let mut buf = [0u8; 4096];
    let len = format_at(&got, 3, &mut buf);
    assert!(len > 0);
    let nlen = usize::try_from(len).unwrap_or(0);
    let mut back = [t_pmentry::zero(); 8];
    assert_eq!(parse_at(&buf[..nlen], &mut back), 3);
    assert_eq!(&back[..3], &got[..3]);
    assert_eq!(format_at(&got, 3, &mut buf[..nlen]), E_OVERFLOW);
    assert_eq!(format_at(&got, 3, &mut buf[..nlen + 1]), len);
    got[1].info.label[0] = b';';
    assert_eq!(format_at(&got, 3, &mut buf[..nlen + 1]), E_INVAL);
}

#[test]
fn registre_mutations_et_troncatures() {
    let raw = base();
    let mut seed = 0x0d10_c0deu32;
    let len = raw.len() as u32;
    let mut i = 0;
    while i < 10000 {
        let mut work = raw.clone();
        let mut k = 1 + i32::try_from(rnd(&mut seed) % 4).unwrap_or(1);
        while k > 0 {
            let at = usize::try_from(rnd(&mut seed) % len).unwrap_or(0);
            work[at] = u8::try_from(rnd(&mut seed) & 0xff).unwrap_or(0);
            k -= 1;
        }
        let hit = u32::from(rnd(&mut seed).is_multiple_of(8));
        let cut = rnd(&mut seed) % len;
        let keep =
            raw.len() - usize::try_from(hit).unwrap_or(0) * usize::try_from(cut).unwrap_or(0);
        check_text(&work[..keep]);
        i += 1;
    }
    let mut cut = 0usize;
    while cut <= raw.len() {
        check_text(&raw[..cut]);
        cut += 1;
    }
}

#[test]
fn pointeurs_nuls_et_bornes() {
    let mut dead = t_pmreg {
        e: core::ptr::null_mut(),
        max: 1,
        n: 7,
        rejected: 8,
    };
    let s = t_span {
        p: core::ptr::null(),
        len: 4,
    };
    // SAFETY: r exclusif, texte nul avec une longueur non nulle.
    assert_eq!(unsafe { pm_registry_scan(s, &mut dead) }, E_INVAL);
    assert_eq!(dead.n, 7);
    let empty = t_span {
        p: core::ptr::null(),
        len: 0,
    };
    dead.max = 0;
    // SAFETY: texte vide, max nul, e nul.
    assert_eq!(unsafe { pm_registry_scan(empty, &mut dead) }, 0);
    // SAFETY: pm nul, rien n'est écrit.
    unsafe { pm_defaults(core::ptr::null_mut()) };
    // SAFETY: paquet nul.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_remove(core::ptr::null_mut(), core::ptr::null()) },
        E_INVAL
    );
    let apk = t_span {
        p: core::ptr::null(),
        len: 0,
    };
    // SAFETY: pm nul.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_install(core::ptr::null_mut(), apk, core::ptr::null_mut()) },
        E_INVAL
    );
    // SAFETY: arguments nuls.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe {
            pm_find(
                core::ptr::null_mut(),
                core::ptr::null(),
                core::ptr::null_mut(),
            )
        },
        E_INVAL
    );
    // SAFETY: sortie nulle avec max non nul.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_list(core::ptr::null_mut(), core::ptr::null_mut(), 1) },
        E_INVAL
    );
    // SAFETY: out nul.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_dec_u32(c"1".as_ptr(), 1, core::ptr::null_mut()) },
        E_INVAL
    );
    // SAFETY: pointeurs nuls.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_hex_cert(core::ptr::null(), core::ptr::null_mut()) },
        E_INVAL
    );
    // SAFETY: s nul et n non nul.
    assert_eq!(unsafe { pm_pkg_ok(core::ptr::null(), 3) }, 0);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_label_ok(core::ptr::null(), 0) }, 1);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_act_ok(core::ptr::null(), 0) }, 0);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_utf8_ok(core::ptr::null(), 0) }, 1);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_utf8_ok(core::ptr::null(), 2) }, 0);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_nlen(core::ptr::null(), 4) }, 0);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_info_ok(core::ptr::null()) }, 0);
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_same_pkg(core::ptr::null(), c"a.b".as_ptr()) },
        0
    );
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_index(core::ptr::null(), 0, core::ptr::null()) },
        E_NOENT
    );
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_index(core::ptr::null(), 1, c"a.b".as_ptr()) },
        E_INVAL
    );
    let mut dst = [0u8; PM_PATH_MAX];
    // SAFETY: dst nul.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe {
            pm_path(
                core::ptr::null_mut(),
                c"a".as_ptr(),
                c"b".as_ptr(),
                c"".as_ptr(),
            )
        },
        E_INVAL
    );
    let root = lit(&"r".repeat(47));
    let name = lit(&"n".repeat(100));
    let suf = lit(&"s".repeat(43));
    // SAFETY: dst a PM_PATH_MAX octets, chaînes terminées.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe {
            pm_path(
                dst.as_mut_ptr().cast(),
                root.as_ptr().cast(),
                name.as_ptr().cast(),
                suf.as_ptr().cast(),
            )
        },
        0
    );
    assert_eq!(cstr_buf(&dst).len(), 47 + 1 + 100 + 43);
    let suf_long = lit(&"s".repeat(44));
    // SAFETY: mêmes tampons, suffixe trop long.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe {
            pm_path(
                dst.as_mut_ptr().cast(),
                root.as_ptr().cast(),
                name.as_ptr().cast(),
                suf_long.as_ptr().cast(),
            )
        },
        E_RANGE
    );
    let root48 = lit(&"r".repeat(48));
    // SAFETY: racine de 48 octets sans NUL dans PM_ROOT_MAX.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe {
            pm_path(
                dst.as_mut_ptr().cast(),
                root48.as_ptr().cast(),
                c"a".as_ptr(),
                c"".as_ptr(),
            )
        },
        E_RANGE
    );
    assert_eq!(PM_ROOT_MAX, 48);
    let mut buf = [0u8; 8];
    assert_eq!(format_at(&[], 0, &mut buf), 0);
    assert_eq!(buf[0], 0);
    assert_eq!(format_at(&[], 65, &mut buf), E_INVAL);
    assert_eq!(format_at(&[], 1, &mut []), E_INVAL);
    let mut line = vec![b'a'; 461];
    let (n, bad) = scan_at(&line, &mut [t_pmentry::zero(); 2], 2);
    assert_eq!(n, 0);
    assert_eq!(bad, 1);
    line.truncate(460);
    let (_, bad) = scan_at(&line, &mut [t_pmentry::zero(); 2], 2);
    assert_eq!(bad, 1);
    let _ = (PM_LABEL_MAX, PM_ACT_MAX, PM_PKG_MAX, PM_REG_MAX);
}

#[test]
fn installation_maj_refus_liste_memoire() {
    let _g = lock();
    let mut pm = t_pm {
        sys_root: core::ptr::null(),
        data_root: core::ptr::null(),
        fs: core::ptr::null(),
        inspect: None,
        ctx: core::ptr::null_mut(),
        max_packages: 0,
        max_apk: 0,
        max_registry: 0,
    };
    setup(&mut pm);
    assert_eq!(fake_install(&mut pm, "com.a", 1, 0xaa), 0);
    assert!(file_data("/data/apps/com.a.apk").is_some());
    assert!(file_data("/data/apps/com.a.apk.tmp").is_none());
    assert!(file_data("/data/apps/paquets.tmp").is_none());
    let expect = format!("com.a;1;Appli;Lx/y/Main;;{}\n", "aa".repeat(32));
    assert_eq!(reg_text(), expect.as_bytes());
    assert_eq!(fake_install(&mut pm, "com.a", 2, 0xaa), 0);
    assert_eq!(fake_install(&mut pm, "com.a", 2, 0xaa), 0);
    assert_eq!(fake_install(&mut pm, "com.a", 1, 0xaa), E_INVAL);
    assert_eq!(fake_install(&mut pm, "com.a", 3, 0xbb), E_ACCES);
    assert_eq!(fake_install(&mut pm, "Com.A", 3, 0xaa), E_EXIST);
    let mut e = t_pmentry::zero();
    // SAFETY: pm exclusif, e exclusif, chaîne terminée.
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_find(&mut pm, c"com.a".as_ptr(), &mut e) },
        0
    );
    assert_eq!(e.info.version_code, 2);
    assert_eq!(e.origin, PM_ORIGIN_DATA);
    assert_eq!(cstr_buf(&e.path), "/data/apps/com.a.apk");
    let body = file_data("/data/apps/com.a.apk").unwrap_or_default();
    assert_eq!(body.get(1).copied(), Some(0xaa));
    assert_eq!(body.first().copied(), Some(2));
    assert_eq!(ALLOC_LIVE.load(Ordering::SeqCst), 0);

    setup(&mut pm);
    let sys = format!("sys.app;1;S;Ls/A;;{HEX}\n");
    assert_eq!(put_file(SYSREG, sys.as_bytes()), 0);
    assert_eq!(fake_install(&mut pm, "sys.app", 9, 0x01), E_PERM);
    assert_eq!(fake_install(&mut pm, "SYS.app", 9, 1), E_PERM);
    with_fake(|f| f.insp_err = E_NOTSUP);
    assert_eq!(fake_install(&mut pm, "com.a", 1, 1), E_NOTSUP);
    with_fake(|f| f.insp_err = 0);
    assert_eq!(fake_install(&mut pm, "../x.y", 1, 1), E_INVAL);
    pm.max_apk = 4;
    assert_eq!(fake_install(&mut pm, "com.a", 1, 1), E_RANGE);
    pm.max_apk = PM_APK_MAX;
    pm.max_packages = 3;
    assert_eq!(fake_install(&mut pm, "com.a", 1, 1), 0);
    assert_eq!(fake_install(&mut pm, "com.b", 1, 1), 0);
    assert_eq!(fake_install(&mut pm, "com.c", 1, 1), E_NOSPC);
    assert_eq!(fake_install(&mut pm, "com.b", 2, 1), 0);
    assert!(file_data("/data/apps/com.c.apk").is_none());
    assert_eq!(listed_ok(&mut pm), 3);
    pm.max_packages = 65;
    assert_eq!(fake_install(&mut pm, "com.d", 1, 1), E_INVAL);

    setup(&mut pm);
    let sys = format!("sys.app;1;S;Ls/A;;{HEX}\n");
    assert_eq!(put_file(SYSREG, sys.as_bytes()), 0);
    let reg = format!("sys.app;9;Faux;Lf/F;;{HEX}\ncom.b;1;B;Lb/B;;{HEX}\n");
    assert_eq!(put_file(REG, reg.as_bytes()), 0);
    assert_eq!(put_file("/data/apps/com.b.apk", b"x"), 0);
    let mut list = [t_pmentry::zero(); 8];
    // SAFETY: pm exclusif, list a 8 places.
    assert_eq!(unsafe { pm_list(&mut pm, list.as_mut_ptr(), 8) }, 2);
    assert_eq!(list[0].origin, PM_ORIGIN_SYSTEM);
    assert_eq!(list[0].info.version_code, 1);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_list(&mut pm, list.as_mut_ptr(), 1) }, 1);
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_find(&mut pm, c"sys.app".as_ptr(), &mut list[0]) },
        0
    );
    assert_eq!(cstr_buf(&list[0].path), "/system/apps/sys.app.apk");
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_find(&mut pm, c"no.pe".as_ptr(), &mut list[0]) },
        E_NOENT
    );
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_remove(&mut pm, c"sys.app".as_ptr()) },
        E_PERM
    );
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_remove(&mut pm, c"no.pe".as_ptr()) },
        E_NOENT
    );
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_remove(&mut pm, c"com.b".as_ptr()) }, 0);
    assert!(file_data("/data/apps/com.b.apk").is_none());
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_find(&mut pm, c"com.b".as_ptr(), &mut list[0]) },
        E_NOENT
    );
    pm.max_registry = 16;
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_list(&mut pm, list.as_mut_ptr(), 8) },
        E_OVERFLOW
    );

    setup(&mut pm);
    assert_eq!(fake_install(&mut pm, "com.a", 1, 1), 0);
    reset_alloc(1);
    assert_eq!(fake_install(&mut pm, "com.b", 1, 1), E_NOMEM);
    reset_alloc(1);
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_remove(&mut pm, c"com.a".as_ptr()) },
        E_NOMEM
    );
    reset_alloc(1);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    assert_eq!(unsafe { pm_list(&mut pm, list.as_mut_ptr(), 1) }, E_NOMEM);
    reset_alloc(1);
    assert_eq!(
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_find(&mut pm, c"com.a".as_ptr(), &mut list[0]) },
        E_NOMEM
    );
    reset_alloc(0);
    assert_eq!(listed_ok(&mut pm), 1);
    assert_eq!(ALLOC_CALLS.load(Ordering::SeqCst), 1);
    assert_eq!(ALLOC_LIVE.load(Ordering::SeqCst), 0);
}

fn setup_sc(pm: &mut t_pm, sc: i32) {
    setup(pm);
    if sc >= 1 {
        assert_eq!(fake_install(pm, "com.a", 1, 0xaa), 0);
    }
    if sc == 3 {
        assert_eq!(fake_install(pm, "com.b", 1, 0xbb), 0);
    }
    with_fake(|f| f.ops = 0);
}

fn run_sc(pm: &mut t_pm, sc: i32) -> c_int {
    match sc {
        0 => fake_install(pm, "com.a", 1, 0xaa),
        1 => fake_install(pm, "com.a", 2, 0xaa),
        2 => fake_install(pm, "com.b", 1, 0xbb),
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        _ => unsafe { pm_remove(pm, c"com.a".as_ptr()) },
    }
}

fn check_point(sc: i32, n: i32, crash: bool, old: &[u8], neu: &[u8]) {
    let mut pm = t_pm {
        sys_root: core::ptr::null(),
        data_root: core::ptr::null(),
        fs: core::ptr::null(),
        inspect: None,
        ctx: core::ptr::null_mut(),
        max_packages: 0,
        max_apk: 0,
        max_registry: 0,
    };
    setup_sc(&mut pm, sc);
    with_fake(|f| {
        f.fail_at = n;
        f.crash = crash;
    });
    let r = run_sc(&mut pm, sc);
    with_fake(|f| f.fail_at = 0);
    let now = reg_text();
    assert!(
        now.as_slice() == old || now.as_slice() == neu,
        "registre ancien ou nouveau"
    );
    if r >= 0 {
        assert_eq!(now.as_slice(), neu);
    }
    assert!(listed_ok(&mut pm) >= 0);
    let _ = run_sc(&mut pm, sc);
    assert_eq!(reg_text().as_slice(), neu);
    assert!(listed_ok(&mut pm) >= 0);
    assert!(file_data("/data/apps/paquets.tmp").is_none());
}

fn sweep(sc: i32) {
    let mut pm = t_pm {
        sys_root: core::ptr::null(),
        data_root: core::ptr::null(),
        fs: core::ptr::null(),
        inspect: None,
        ctx: core::ptr::null_mut(),
        max_packages: 0,
        max_apk: 0,
        max_registry: 0,
    };
    setup_sc(&mut pm, sc);
    let old = reg_text();
    assert!(run_sc(&mut pm, sc) >= 0);
    let ops = with_fake(|f| f.ops);
    let neu = reg_text();
    assert_ne!(old, neu);
    assert!(ops >= 4, "ops {ops}");
    let mut n = 1;
    while n <= ops {
        check_point(sc, n, false, &old, &neu);
        check_point(sc, n, true, &old, &neu);
        n += 1;
    }
}

#[test]
fn pannes_installation_maj_retrait() {
    let _g = lock();
    sweep(0);
    sweep(1);
    sweep(2);
    sweep(3);
}
