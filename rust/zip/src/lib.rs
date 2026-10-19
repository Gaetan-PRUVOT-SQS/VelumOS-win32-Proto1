#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

use core::ffi::{c_char, c_int};

mod crc;
mod inflate;
mod name;
mod zipfile;

pub(crate) const E_OK: c_int = 0;
pub(crate) const E_NOENT: c_int = -2;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_RANGE: c_int = -34;
pub(crate) const E_OVERFLOW: c_int = -75;
pub(crate) const E_NOTSUP: c_int = -95;
pub(crate) const ZIP_MAX_ENTRIES: u32 = 4096;
pub(crate) const ZIP_NAME_MAX: u32 = 255;

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_span {
    pub p: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_zip {
    pub file: t_span,
    pub cd_off: u32,
    pub cd_size: u32,
    pub count: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_zipent {
    pub name: *const c_char,
    pub name_len: u32,
    pub method: u32,
    pub crc: u32,
    pub csize: u32,
    pub r#usize: u32,
    pub data_off: u32,
    pub lfh_off: u32,
}

fn bytes_of<'a>(p: *const u8, len: usize) -> Result<&'a [u8], c_int> {
    if len == 0 {
        return Ok(&[]);
    }
    if p.is_null() || len > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: p non nul, len borné par isize::MAX. Le contrat FFI exige len octets initialisés.
    Ok(unsafe { core::slice::from_raw_parts(p, len) })
}

/// # Safety
/// `z` est nul, ou bien il est aligné et pointe une `t_zip` exclusive le temps de l'appel.
/// `file.p` est nul, ou bien il pointe `file.len` octets initialisés. Au-delà de 4 Gio, `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn zip_open(z: *mut t_zip, file: t_span) -> c_int {
    if z.is_null() || !z.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: z non nul et aligné, accès exclusif pour l'appel.
    let z = unsafe { &mut *z };
    zipfile::open_mut(z, file)
}

/// # Safety
/// `z` est nul, ou bien il est aligné et sa `file` reste valide. `out` est nul, ou bien il est aligné et exclusive.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn zip_entry(z: *const t_zip, index: u32, out: *mut t_zipent) -> c_int {
    if z.is_null() || !z.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: z non nul et aligné, lecture seule le temps de l'appel.
    let z = unsafe { &*z };
    if index >= z.count {
        return E_RANGE;
    }
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: out non nul et aligné, exclusif le temps de l'appel.
    let out = unsafe { &mut *out };
    match zipfile::entry_at(z, index, out) {
        Ok(()) => E_OK,
        Err(e) => e,
    }
}

/// # Safety
/// `z` comme pour `zip_entry`. `name` est nul, ou bien une chaîne C terminée par NUL.
/// `out` est nul, ou bien aligné et exclusif. Un nom d'entrée n'est pas forcément terminé par NUL.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn zip_find(
    z: *const t_zip,
    name: *const c_char,
    out: *mut t_zipent,
) -> c_int {
    if z.is_null() || !z.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: z non nul et aligné, lecture seule le temps de l'appel.
    let z = unsafe { &*z };
    if z.count == 0 {
        return E_NOENT;
    }
    if name.is_null() || out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: out non nul et aligné, exclusif le temps de l'appel.
    let out = unsafe { &mut *out };
    match zipfile::find_name(z, name, out) {
        Ok(()) => E_OK,
        Err(e) => e,
    }
}

/// # Safety
/// `z` et `e` sont nuls, ou bien alignés et valides. `z.file` reste lisible.
/// `out` est nul, ou bien il pointe `cap` octets inscriptibles qui ne chevauchent pas `z.file`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn zip_extract(
    z: *const t_zip,
    e: *const t_zipent,
    out: *mut u8,
    cap: usize,
) -> i64 {
    if z.is_null() || !z.is_aligned() || e.is_null() || !e.is_aligned() {
        return i64::from(E_INVAL);
    }
    // SAFETY: z non nul et aligné, lecture seule le temps de l'appel.
    let z = unsafe { &*z };
    // SAFETY: e non nul et aligné, lecture seule le temps de l'appel.
    let e = unsafe { &*e };
    match zipfile::extract_ref(z, e, out, cap) {
        Ok(n) => n,
        Err(err) => i64::from(err),
    }
}

/// # Safety
/// `name` est nul, ou bien il pointe `len` octets lisibles. Un pointeur nul ou une longueur nulle donne 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn zip_name_safe(name: *const c_char, len: usize) -> c_int {
    if name::is_safe(name, len) { 1 } else { 0 }
}

/// # Safety
/// `in.p` est nul, ou bien il pointe `in.len` octets initialisés. Pas d'en-tête zlib.
/// `out` est nul, ou bien il pointe `cap` octets inscriptibles qui ne chevauchent pas `in`.
/// Un flux tronqué ou une distance invalide donne un code négatif, pas un panic.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn inflate_raw(input: t_span, out: *mut u8, cap: usize) -> i64 {
    let src = match bytes_of(input.p, input.len) {
        Ok(s) => s,
        Err(e) => return i64::from(e),
    };
    match inflate::inflate_slice(src, out, cap) {
        Ok(n) => i64::try_from(n).unwrap_or(i64::from(E_OVERFLOW)),
        Err(e) => i64::from(e),
    }
}

/// # Safety
/// `p` est nul, ou bien il pointe `n` octets lisibles. `n == 0` ne lit rien.
/// `p` nul et `n > 0` ne lit rien et renvoie `crc` inchangé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn zip_crc32(crc: u32, p: *const u8, n: usize) -> u32 {
    if n == 0 || p.is_null() || n > isize::MAX as usize {
        return crc::crc32(crc, &[]);
    }
    // SAFETY: p non nul, n borné, n octets initialisés.
    let data = unsafe { core::slice::from_raw_parts(p, n) };
    crc::crc32(crc, data)
}

#[cfg(test)]
mod tests;
