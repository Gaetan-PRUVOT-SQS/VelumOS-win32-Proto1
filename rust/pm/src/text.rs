use core::ffi::{c_char, c_int};

use crate::check::{act_ok, dec_u32, hex_cert, info_ok, label_ok, pkg_ok, same_pkg};
use crate::raw::{bytes_at, copy_bytes, nlen};
use crate::{
    E_INVAL, E_NOENT, E_OK, E_OVERFLOW, E_RANGE, PM_ACT_MAX, PM_CERT_LEN, PM_LABEL_MAX,
    PM_LINE_MAX, PM_MAX_PACKAGES, PM_PATH_MAX, PM_PKG_MAX, PM_ROOT_MAX, t_pmentry, t_pminfo,
    t_pmreg, t_span, t_text,
};

fn field_end(s: &[u8], from: usize) -> usize {
    let mut i = from;
    while i < s.len() && s[i] != b';' {
        i += 1;
    }
    i
}

fn parse_tail(s: &[u8], out: &mut t_pminfo) -> c_int {
    let need = 3 + 1 + 2 * PM_CERT_LEN;
    if s.len() < need {
        return E_INVAL;
    }
    let Some(act_n) = s.len().checked_sub(1 + 2 * PM_CERT_LEN) else {
        return E_INVAL;
    };
    if s[act_n] != b';' || !act_ok(bytes_at(s, act_n)) {
        return E_INVAL;
    }
    copy_bytes(&mut out.activity, bytes_at(s, act_n));
    let hex = &s[act_n + 1..act_n + 1 + 2 * PM_CERT_LEN];
    match hex_cert(hex) {
        Ok(v) => {
            out.cert = v;
            E_OK
        }
        Err(e) => e,
    }
}

fn parse_line(s: &[u8], out: &mut t_pminfo) -> c_int {
    *out = t_pminfo::zero();
    let a = field_end(s, 0);
    if a >= s.len() || !pkg_ok(bytes_at(s, a)) {
        return E_INVAL;
    }
    let Some(b_from) = a.checked_add(1) else {
        return E_INVAL;
    };
    let b = field_end(s, b_from);
    if b >= s.len() || dec_u32(&s[b_from..b], &mut out.version_code) < 0 {
        return E_INVAL;
    }
    let Some(c_from) = b.checked_add(1) else {
        return E_INVAL;
    };
    let c = field_end(s, c_from);
    if c >= s.len() || !label_ok(&s[c_from..c]) {
        return E_INVAL;
    }
    copy_bytes(&mut out.package, bytes_at(s, a));
    copy_bytes(&mut out.label, &s[c_from..c]);
    let Some(tail) = c.checked_add(1) else {
        return E_INVAL;
    };
    if tail > s.len() {
        return E_INVAL;
    }
    parse_tail(&s[tail..], out)
}

fn take_line(r: &mut t_pmreg, mut line: &[u8]) {
    if line.last() == Some(&b'\r') {
        line = &line[..line.len() - 1];
    }
    if line.is_empty() {
        return;
    }
    if line.len() > PM_LINE_MAX {
        r.rejected = r.rejected.wrapping_add(1);
        return;
    }
    let mut info = t_pminfo::zero();
    if parse_line(line, &mut info) < 0 {
        r.rejected = r.rejected.wrapping_add(1);
        return;
    }
    if r.n >= r.max {
        r.rejected = r.rejected.wrapping_add(1);
        return;
    }
    if index_of(r.e, r.n, info.package.as_ptr().cast()) >= 0 {
        r.rejected = r.rejected.wrapping_add(1);
        return;
    }
    let Ok(i) = usize::try_from(r.n) else {
        r.rejected = r.rejected.wrapping_add(1);
        return;
    };
    if r.e.is_null() {
        r.rejected = r.rejected.wrapping_add(1);
        return;
    }
    // SAFETY: n < max, e non nul, l'appelant a fourni max entrées alignées.
    let slot = unsafe { &mut *r.e.add(i) };
    *slot = t_pmentry::zero();
    slot.info = info;
    r.n = r.n.wrapping_add(1);
}

pub(crate) fn index_of(e: *const t_pmentry, n: u32, package: *const c_char) -> c_int {
    if n == 0 {
        return E_NOENT;
    }
    if e.is_null() || !e.is_aligned() || package.is_null() {
        return E_INVAL;
    }
    let mut i = 0u32;
    while i < n {
        let Ok(at) = usize::try_from(i) else {
            return E_INVAL;
        };
        // SAFETY: i < n, e non nul et aligné, n entrées lisibles.
        let ent = unsafe { &*e.add(at) };
        if same_pkg(ent.info.package.as_ptr().cast(), package) {
            return c_int::try_from(i).unwrap_or(E_INVAL);
        }
        i += 1;
    }
    E_NOENT
}

fn put(out: t_text, at: &mut usize, src: &[u8]) -> c_int {
    let Some(room) = out.cap.checked_sub(*at) else {
        return E_OVERFLOW;
    };
    if src.len() >= room {
        return E_OVERFLOW;
    }
    if !src.is_empty() {
        if out.p.is_null() {
            return E_INVAL;
        }
        // SAFETY: out.p non nul, at + src.len() < cap. Pas de recouvrement avec src.
        unsafe {
            core::ptr::copy_nonoverlapping(src.as_ptr(), out.p.add(*at).cast(), src.len());
        }
    }
    *at += src.len();
    E_OK
}

fn put_u32(out: t_text, at: &mut usize, mut v: u32) -> c_int {
    let mut d = [0u8; 10];
    let mut i = 10usize;
    while i == 10 || v != 0 {
        i -= 1;
        d[i] = b'0' + u8::try_from(v % 10).unwrap_or(0);
        v /= 10;
    }
    put(out, at, &d[i..])
}

fn put_hex(out: t_text, at: &mut usize, cert: &[u8; PM_CERT_LEN]) -> c_int {
    const HEX: &[u8] = b"0123456789abcdef";
    let mut h = [0u8; PM_CERT_LEN * 2];
    for i in 0..PM_CERT_LEN {
        h[2 * i] = HEX[usize::from(cert[i] >> 4)];
        h[2 * i + 1] = HEX[usize::from(cert[i] & 15)];
    }
    put(out, at, &h)
}

fn put_entry(out: t_text, at: &mut usize, i: &t_pminfo) -> c_int {
    if !info_ok(i) {
        return E_INVAL;
    }
    let pkg = bytes_at(&i.package, nlen(i.package.as_ptr().cast(), PM_PKG_MAX));
    let label = bytes_at(&i.label, nlen(i.label.as_ptr().cast(), PM_LABEL_MAX));
    let act = bytes_at(&i.activity, nlen(i.activity.as_ptr().cast(), PM_ACT_MAX));
    if put(out, at, pkg) < 0
        || put(out, at, b";") < 0
        || put_u32(out, at, i.version_code) < 0
        || put(out, at, b";") < 0
        || put(out, at, label) < 0
        || put(out, at, b";") < 0
        || put(out, at, act) < 0
        || put(out, at, b";") < 0
        || put_hex(out, at, &i.cert) < 0
        || put(out, at, b"\n") < 0
    {
        return E_OVERFLOW;
    }
    E_OK
}

pub(crate) fn path_at(
    dst: *mut c_char,
    root: *const c_char,
    name: *const c_char,
    suf: *const c_char,
) -> c_int {
    if dst.is_null() || root.is_null() || name.is_null() || suf.is_null() {
        return E_INVAL;
    }
    let r = nlen(root, PM_ROOT_MAX);
    let n = nlen(name, PM_PATH_MAX);
    let s = nlen(suf, PM_PATH_MAX);
    let Some(sum) = r
        .checked_add(n)
        .and_then(|v| v.checked_add(s))
        .and_then(|v| v.checked_add(2))
    else {
        return E_RANGE;
    };
    if r == 0 || r >= PM_ROOT_MAX || sum > PM_PATH_MAX {
        return E_RANGE;
    }
    // SAFETY: dst a au moins PM_PATH_MAX octets. sum <= PM_PATH_MAX. Sources bornées par nlen.
    unsafe {
        let dst = dst.cast::<u8>();
        core::ptr::copy_nonoverlapping(root.cast::<u8>(), dst, r);
        dst.add(r).write(b'/');
        core::ptr::copy_nonoverlapping(name.cast::<u8>(), dst.add(r + 1), n);
        core::ptr::copy_nonoverlapping(suf.cast::<u8>(), dst.add(r + 1 + n), s);
        dst.add(r + 1 + n + s).write(0);
    }
    E_OK
}

/// # Safety
/// `text.p` est nul seulement si `text.len` vaut 0, sinon il pointe `text.len` octets.
/// `r` est nul, ou bien aligné et exclusif. `r.e` pointe `r.max` entrées si `r.max` n'est pas 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_registry_scan(text: t_span, r: *mut t_pmreg) -> c_int {
    if r.is_null() || !r.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: r non nul et aligné, exclusif le temps de l'appel.
    let reg = unsafe { &mut *r };
    if reg.e.is_null() && reg.max != 0 {
        return E_INVAL;
    }
    if !reg.e.is_null() && !reg.e.is_aligned() {
        return E_INVAL;
    }
    if text.p.is_null() && text.len != 0 {
        return E_INVAL;
    }
    if text.len > isize::MAX as usize {
        return E_INVAL;
    }
    reg.n = 0;
    reg.rejected = 0;
    let bytes = if text.len == 0 {
        &[][..]
    } else {
        // SAFETY: p non nul, len borné. Le contrat fournit len octets.
        unsafe { core::slice::from_raw_parts(text.p, text.len) }
    };
    let mut start = 0usize;
    while start < bytes.len() {
        let mut end = start;
        while end < bytes.len() && bytes[end] != b'\n' {
            end += 1;
        }
        take_line(reg, &bytes[start..end]);
        start = end + 1;
    }
    c_int::try_from(reg.n).unwrap_or(E_OVERFLOW)
}

/// # Safety
/// `text` comme pour `pm_registry_scan`. `out` est nul seulement si `max` vaut 0,
/// sinon il pointe `max` entrées alignées et exclusives.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_registry_parse(text: t_span, out: *mut t_pmentry, max: u32) -> c_int {
    let mut reg = t_pmreg {
        e: out,
        max,
        n: 0,
        rejected: 0,
    };
    // SAFETY: reg est local et exclusif. out suit le contrat de pm_registry_parse.
    unsafe { pm_registry_scan(text, &mut reg) }
}

/// # Safety
/// `e` est nul seulement si `n` vaut 0, sinon il pointe `n` entrées. `package` est une chaîne C ou nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_index(e: *const t_pmentry, n: u32, package: *const c_char) -> c_int {
    index_of(e, n, package)
}

/// # Safety
/// `dst` est nul, ou bien il pointe `PM_PATH_MAX` octets exclusifs.
/// `root`, `name` et `suf` sont nuls ou des chaînes C.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_path(
    dst: *mut c_char,
    root: *const c_char,
    name: *const c_char,
    suf: *const c_char,
) -> c_int {
    path_at(dst, root, name, suf)
}

/// # Safety
/// `e` est nul seulement si `n` vaut 0, sinon il pointe `n` entrées lisibles.
/// `out.p` est nul, ou bien il pointe `out.cap` octets exclusifs. `out.cap == 0` est refusé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_registry_format(e: *const t_pmentry, n: u32, out: t_text) -> c_int {
    if out.p.is_null() || out.cap == 0 || (e.is_null() && n != 0) || n > PM_MAX_PACKAGES {
        return E_INVAL;
    }
    if !e.is_null() && !e.is_aligned() {
        return E_INVAL;
    }
    let mut at = 0usize;
    let mut i = 0u32;
    while i < n {
        let Ok(at_i) = usize::try_from(i) else {
            return E_INVAL;
        };
        // SAFETY: i < n, e non nul et aligné, n entrées lisibles.
        let ent = unsafe { &*e.add(at_i) };
        let r = put_entry(out, &mut at, &ent.info);
        if r < 0 {
            return r;
        }
        i += 1;
    }
    if at >= out.cap {
        return E_OVERFLOW;
    }
    // SAFETY: out.p non nul, at < cap. Écriture du NUL final.
    unsafe { out.p.add(at).write(0) };
    c_int::try_from(at).unwrap_or(E_OVERFLOW)
}

/// # Safety
/// `pm` est nul, ou bien il est aligné. Seuls les trois plafonds sont écrits.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_defaults(pm: *mut crate::t_pm) {
    if pm.is_null() || !pm.is_aligned() {
        return;
    }
    // SAFETY: pm non nul et aligné, exclusif pour les trois champs.
    unsafe {
        (*pm).max_packages = PM_MAX_PACKAGES;
        (*pm).max_apk = crate::PM_APK_MAX;
        (*pm).max_registry = crate::PM_REG_MAX;
    }
}
