#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod file;
mod modes;
mod parse;

#[cfg(test)]
mod fake;
#[cfg(test)]
mod tests;

use core::ffi::{c_char, c_int};

pub(crate) const E_OK: c_int = 0;
pub(crate) const E_IO: c_int = -5;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_RANGE: c_int = -34;
pub(crate) const E_OVERFLOW: c_int = -75;

pub(crate) const SET_THEME_LIGHT: u32 = 0;
pub(crate) const SET_THEME_DARK: u32 = 1;
pub(crate) const SET_THEME_AUTO: u32 = 2;
pub(crate) const SET_LAYOUT_AZERTY: u32 = 0;
pub(crate) const SET_LAYOUT_QWERTY: u32 = 1;
pub(crate) const SET_FILE_MAX: usize = 512;
pub(crate) const SET_NIGHT_FROM: u32 = 19;
pub(crate) const SET_NIGHT_TO: u32 = 7;
pub(crate) const SET_MODES: u32 = 5;

pub(crate) const O_RDONLY: u32 = 0x0000;
pub(crate) const O_WRONLY: u32 = 0x0001;
pub(crate) const O_CREAT: u32 = 0x0040;
pub(crate) const O_TRUNC: u32 = 0x0200;
pub(crate) const MODE_0644: u32 = 0o644;

const _: () = assert!(SET_THEME_LIGHT == 0 && SET_THEME_DARK == 1 && SET_THEME_AUTO == 2);
const _: () = assert!(SET_LAYOUT_AZERTY == 0 && SET_LAYOUT_QWERTY == 1);
const _: () = assert!(SET_FILE_MAX == 512 && SET_NIGHT_FROM == 19 && SET_NIGHT_TO == 7);
const _: () = assert!(SET_MODES == 5);
const _: () = assert!(O_RDONLY == 0 && (O_WRONLY | O_CREAT | O_TRUNC) == 0x241);
const _: () = assert!(MODE_0644 == 420);

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_setmode {
    pub w: u32,
    pub h: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_settings {
    pub theme: u32,
    pub reduced: u32,
    pub layout: u32,
    pub mode: u32,
}

const _: () = assert!(core::mem::size_of::<t_setmode>() == 8);
const _: () = assert!(core::mem::align_of::<t_setmode>() == 4);
const _: () = assert!(core::mem::offset_of!(t_setmode, w) == 0);
const _: () = assert!(core::mem::offset_of!(t_setmode, h) == 4);
const _: () = assert!(core::mem::size_of::<t_settings>() == 16);
const _: () = assert!(core::mem::align_of::<t_settings>() == 4);
const _: () = assert!(core::mem::offset_of!(t_settings, theme) == 0);
const _: () = assert!(core::mem::offset_of!(t_settings, reduced) == 4);
const _: () = assert!(core::mem::offset_of!(t_settings, layout) == 8);
const _: () = assert!(core::mem::offset_of!(t_settings, mode) == 12);

/// # Safety
/// `s` est nul, ou bien aligné et exclusif le temps de l'appel.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn set_default(s: *mut t_settings) {
    if s.is_null() || !s.is_aligned() {
        return;
    }
    // SAFETY: s non nul et aligné, accès exclusif pour l'appel.
    let s = unsafe { &mut *s };
    modes::apply_default(s);
}

/// # Safety
/// `text` est nul seulement si `len` vaut 0, sinon il pointe `len` octets initialisés.
/// `s` est nul, ou bien aligné et exclusif. Au-delà de `SET_FILE_MAX`, le texte n'est pas lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn set_parse(text: *const c_char, len: usize, s: *mut t_settings) -> c_int {
    if s.is_null() || !s.is_aligned() {
        return E_INVAL;
    }
    if text.is_null() && len != 0 {
        return E_INVAL;
    }
    // SAFETY: s non nul et aligné, accès exclusif pour l'appel.
    let settings = unsafe { &mut *s };
    if len > SET_FILE_MAX {
        modes::apply_default(settings);
        return E_RANGE;
    }
    let bytes = if len == 0 {
        &[][..]
    } else {
        // SAFETY: text non nul, len <= SET_FILE_MAX, octets initialisés, alignement 1.
        unsafe { core::slice::from_raw_parts(text.cast::<u8>(), len) }
    };
    parse::parse(bytes, settings)
}

/// # Safety
/// `s` est nul, ou bien aligné et lisible. `out` est nul, ou bien `max` octets exclusifs.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn set_format(s: *const t_settings, out: *mut c_char, max: usize) -> c_int {
    if s.is_null() || !s.is_aligned() || out.is_null() || max == 0 {
        return E_INVAL;
    }
    if max > isize::MAX as usize {
        return E_OVERFLOW;
    }
    // SAFETY: s non nul et aligné, lecture seule le temps de l'appel.
    let settings = unsafe { &*s };
    // SAFETY: out non nul, max octets exclusifs, max tient dans isize.
    let buf = unsafe { core::slice::from_raw_parts_mut(out.cast::<u8>(), max) };
    parse::format(settings, buf)
}

/// # Safety
/// `s` est nul, ou bien aligné et lisible le temps de l'appel.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn set_dark_at(s: *const t_settings, hour: u32) -> bool {
    if s.is_null() || !s.is_aligned() {
        return false;
    }
    // SAFETY: s non nul et aligné, lecture seule le temps de l'appel.
    let settings = unsafe { &*s };
    modes::dark_at(settings, hour)
}

/// # Safety
/// Aucun pointeur. Un indice >= SET_MODES renvoie 1024x768.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn set_mode_get(index: u32) -> t_setmode {
    modes::mode_at(index)
}

/// # Safety
/// `s` est nul, ou bien aligné et exclusif. `path` est nul, ou bien une chaîne C.
/// `v_open` est appelé avec O_RDONLY et le mode 0. `v_read` puis `v_close` suivent.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn set_load(path: *const c_char, s: *mut t_settings) -> c_int {
    if s.is_null() || !s.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: s non nul et aligné, accès exclusif pour l'appel.
    let settings = unsafe { &mut *s };
    file::load(path, settings)
}

/// # Safety
/// `path` est nul, ou bien une chaîne C. `s` est nul, ou bien aligné et lisible.
/// Le temporaire est ouvert avec O_WRONLY | O_CREAT | O_TRUNC et le mode 0644.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn set_save(path: *const c_char, s: *const t_settings) -> c_int {
    if path.is_null() {
        return E_INVAL;
    }
    if s.is_null() || !s.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: s non nul et aligné, lecture seule le temps de l'appel.
    let settings = unsafe { &*s };
    file::save(path, settings)
}
