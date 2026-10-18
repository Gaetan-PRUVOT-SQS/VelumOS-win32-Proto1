use core::ffi::{c_char, c_int};

use crate::raw::{bytes_at, nlen, slice_n};
use crate::{E_INVAL, E_OK, PM_ACT_MAX, PM_CERT_LEN, PM_LABEL_MAX, PM_PKG_MAX, t_pminfo};

fn seq_len(c: u8) -> usize {
    if c < 0x80 {
        1
    } else if (0xc2..=0xdf).contains(&c) {
        2
    } else if (0xe0..=0xef).contains(&c) {
        3
    } else if (0xf0..=0xf4).contains(&c) {
        4
    } else {
        0
    }
}

fn second_ok(lead: u8, c: u8) -> bool {
    match lead {
        0xe0 => (0xa0..=0xbf).contains(&c),
        0xed => (0x80..=0x9f).contains(&c),
        0xf0 => (0x90..=0xbf).contains(&c),
        0xf4 => (0x80..=0x8f).contains(&c),
        _ => (0x80..=0xbf).contains(&c),
    }
}

pub(crate) fn utf8_ok(s: &[u8]) -> bool {
    let mut i = 0usize;
    while i < s.len() {
        let len = seq_len(s[i]);
        let Some(end) = i.checked_add(len) else {
            return false;
        };
        if len == 0 || end > s.len() {
            return false;
        }
        if len > 1 && !second_ok(s[i], s[i + 1]) {
            return false;
        }
        let mut k = 2usize;
        while k < len {
            if s[i + k] & 0xc0 != 0x80 {
                return false;
            }
            k += 1;
        }
        i = end;
    }
    true
}

fn pkg_char(c: u8) -> bool {
    c.is_ascii_alphanumeric() || c == b'_' || c == b'.'
}

pub(crate) fn pkg_ok(s: &[u8]) -> bool {
    if s.is_empty() || s.len() >= PM_PKG_MAX || s[0] == b'.' || s[s.len() - 1] == b'.' {
        return false;
    }
    let mut dots = 0i32;
    for i in 0..s.len() {
        let c = s[i];
        if !pkg_char(c) {
            return false;
        }
        if c == b'.' {
            if i + 1 >= s.len() || s[i + 1] == b'.' {
                return false;
            }
            dots += 1;
        }
    }
    dots > 0
}

pub(crate) fn label_ok(s: &[u8]) -> bool {
    if s.len() >= PM_LABEL_MAX || !utf8_ok(s) {
        return false;
    }
    for &c in s {
        if c < 0x20 || c == 0x7f || c == b';' {
            return false;
        }
    }
    true
}

pub(crate) fn act_ok(s: &[u8]) -> bool {
    if s.len() < 3 || s.len() >= PM_ACT_MAX || s[0] != b'L' || s[s.len() - 1] != b';' {
        return false;
    }
    for &c in &s[1..s.len() - 1] {
        if c == b'.' || (!pkg_char(c) && c != b'/' && c != b'$') {
            return false;
        }
    }
    true
}

pub(crate) fn info_ok(i: &t_pminfo) -> bool {
    let pn = nlen(i.package.as_ptr().cast(), PM_PKG_MAX);
    let ln = nlen(i.label.as_ptr().cast(), PM_LABEL_MAX);
    let an = nlen(i.activity.as_ptr().cast(), PM_ACT_MAX);
    pkg_ok(bytes_at(&i.package, pn))
        && label_ok(bytes_at(&i.label, ln))
        && act_ok(bytes_at(&i.activity, an))
}

fn fold(c: u8) -> u8 {
    if c.is_ascii_uppercase() { c + 32 } else { c }
}

pub(crate) fn same_pkg(a: *const c_char, b: *const c_char) -> bool {
    if a.is_null() || b.is_null() {
        return false;
    }
    let mut i = 0usize;
    while i < PM_PKG_MAX {
        // SAFETY: a et b non nuls, i < PM_PKG_MAX. Lecture d'un octet de chaque.
        let ca = unsafe { a.add(i).read() } as u8;
        if ca == 0 {
            break;
        }
        // SAFETY: b non nul, i < PM_PKG_MAX, a[i] n'est pas NUL.
        let cb = unsafe { b.add(i).read() } as u8;
        if fold(ca) != fold(cb) {
            return false;
        }
        i += 1;
    }
    if i >= PM_PKG_MAX {
        return false;
    }
    // SAFETY: i < PM_PKG_MAX, a et b non nuls. Lecture du NUL de fin.
    let ca = unsafe { a.add(i).read() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let cb = unsafe { b.add(i).read() };
    ca == 0 && cb == 0
}

pub(crate) fn dec_u32(s: &[u8], out: &mut u32) -> c_int {
    if s.is_empty() || s.len() > 10 || (s.len() > 1 && s[0] == b'0') {
        return E_INVAL;
    }
    let mut v: u64 = 0;
    for &c in s {
        if !c.is_ascii_digit() {
            return E_INVAL;
        }
        let Some(next) = v
            .checked_mul(10)
            .and_then(|n| n.checked_add(u64::from(c - b'0')))
        else {
            return E_INVAL;
        };
        v = next;
    }
    match u32::try_from(v) {
        Ok(n) => {
            *out = n;
            E_OK
        }
        Err(_) => E_INVAL,
    }
}

fn hex_val(c: u8) -> Option<u8> {
    match c {
        b'0'..=b'9' => Some(c - b'0'),
        b'a'..=b'f' => Some(c - b'a' + 10),
        b'A'..=b'F' => Some(c - b'A' + 10),
        _ => None,
    }
}

pub(crate) fn hex_cert(s: &[u8]) -> Result<[u8; PM_CERT_LEN], c_int> {
    if s.len() < PM_CERT_LEN * 2 {
        return Err(E_INVAL);
    }
    let mut out = [0u8; PM_CERT_LEN];
    for i in 0..PM_CERT_LEN {
        let Some(hi) = hex_val(s[2 * i]) else {
            return Err(E_INVAL);
        };
        let Some(lo) = hex_val(s[2 * i + 1]) else {
            return Err(E_INVAL);
        };
        out[i] = hi * 16 + lo;
    }
    Ok(out)
}

fn flag(v: bool) -> c_int {
    c_int::from(v)
}

/// # Safety
/// `s` est nul, ou bien il pointe `n` octets. `n == 0` ne lit rien.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_utf8_ok(s: *const c_char, n: usize) -> c_int {
    match slice_n(s.cast(), n) {
        Ok(b) => flag(utf8_ok(b)),
        Err(_) => 0,
    }
}

/// # Safety
/// `s` est une chaîne C, ou nul. La lecture s'arrête au premier NUL ou à `max`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_nlen(s: *const c_char, max: usize) -> usize {
    nlen(s, max)
}

/// # Safety
/// `s` est nul, ou bien il pointe `n` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_pkg_ok(s: *const c_char, n: usize) -> c_int {
    match slice_n(s.cast(), n) {
        Ok(b) => flag(pkg_ok(b)),
        Err(_) => 0,
    }
}

/// # Safety
/// `s` est nul, ou bien il pointe `n` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_label_ok(s: *const c_char, n: usize) -> c_int {
    match slice_n(s.cast(), n) {
        Ok(b) => flag(label_ok(b)),
        Err(_) => 0,
    }
}

/// # Safety
/// `s` est nul, ou bien il pointe `n` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_act_ok(s: *const c_char, n: usize) -> c_int {
    match slice_n(s.cast(), n) {
        Ok(b) => flag(act_ok(b)),
        Err(_) => 0,
    }
}

/// # Safety
/// `i` est nul, ou bien il est aligné et pointe une `t_pminfo` lisible.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_info_ok(i: *const t_pminfo) -> c_int {
    if i.is_null() || !i.is_aligned() {
        return 0;
    }
    // SAFETY: i non nul et aligné, lecture seule le temps de l'appel.
    flag(info_ok(unsafe { &*i }))
}

/// # Safety
/// `a` et `b` sont nuls, ou bien des chaînes C lisibles sur au plus `PM_PKG_MAX` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_same_pkg(a: *const c_char, b: *const c_char) -> c_int {
    flag(same_pkg(a, b))
}

/// # Safety
/// `s` est nul, ou bien il pointe `n` octets. `out` est nul, ou bien aligné et exclusif.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_dec_u32(s: *const c_char, n: usize, out: *mut u32) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    let Ok(bytes) = slice_n(s.cast(), n) else {
        return E_INVAL;
    };
    // SAFETY: out non nul et aligné, exclusif le temps de l'appel.
    dec_u32(bytes, unsafe { &mut *out })
}

/// # Safety
/// `s` est nul, ou bien il pointe 64 caractères. `out` est nul, ou bien il pointe 32 octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_hex_cert(s: *const c_char, out: *mut u8) -> c_int {
    if s.is_null() || out.is_null() {
        return E_INVAL;
    }
    // SAFETY: s non nul, 64 octets lus. PM_CERT_LEN * 2 tient dans isize.
    let bytes = unsafe { core::slice::from_raw_parts(s.cast::<u8>(), PM_CERT_LEN * 2) };
    match hex_cert(bytes) {
        Ok(v) => {
            // SAFETY: out non nul, 32 octets exclusifs.
            unsafe { core::ptr::copy_nonoverlapping(v.as_ptr(), out, PM_CERT_LEN) };
            E_OK
        }
        Err(e) => e,
    }
}
