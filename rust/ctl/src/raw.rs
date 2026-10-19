use core::ffi::{c_char, c_void};

use crate::ffi::{memcpy, memmove, memset, strcmp, strlen};

pub(crate) fn slen(s: *const c_char) -> u32 {
    if s.is_null() {
        return 0;
    }
    // SAFETY: s n'est pas nul. Le contrat C exige un octet NUL terminal.
    unsafe { strlen(s) as u32 }
}

pub(crate) fn scmp(a: *const c_char, b: *const c_char) -> i32 {
    // SAFETY: a et b sont des chaînes C non nulles, terminées.
    unsafe { strcmp(a, b) }
}

pub(crate) fn scopy(dst: *mut c_void, src: *const c_void, n: usize) {
    if n == 0 {
        return;
    }
    // SAFETY: n > 0. dst et src couvrent n octets. Ils peuvent se chevaucher.
    unsafe {
        memmove(dst, src, n);
    }
}

pub(crate) fn scopy_distinct(dst: *mut c_void, src: *const c_void, n: usize) {
    if n == 0 {
        return;
    }
    // SAFETY: n > 0. dst et src couvrent n octets et ne se chevauchent pas.
    unsafe {
        memcpy(dst, src, n);
    }
}

pub(crate) fn szero(dst: *mut c_void, n: usize) {
    if n == 0 || dst.is_null() {
        return;
    }
    // SAFETY: dst couvre n octets.
    unsafe {
        memset(dst, 0, n);
    }
}

pub(crate) fn fill_byte(dst: *mut c_void, byte: u8, n: usize) {
    if n == 0 || dst.is_null() {
        return;
    }
    // SAFETY: dst couvre n octets.
    unsafe {
        memset(dst, i32::from(byte), n);
    }
}

pub(crate) fn bytes<'a>(s: *const c_char, n: u32) -> &'a [u8] {
    if n == 0 || s.is_null() {
        return &[];
    }
    // SAFETY: n > 0 et s n'est pas nul. L'appelant garantit n octets.
    unsafe { core::slice::from_raw_parts(s.cast::<u8>(), n as usize) }
}
