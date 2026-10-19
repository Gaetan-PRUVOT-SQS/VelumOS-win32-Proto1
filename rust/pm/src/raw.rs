use core::ffi::c_char;

use crate::E_INVAL;

pub(crate) fn nlen(s: *const c_char, max: usize) -> usize {
    if s.is_null() {
        return 0;
    }
    let mut n = 0usize;
    while n < max {
        if n > isize::MAX as usize {
            break;
        }
        // SAFETY: s non nul, n < max et n tient dans isize. Lecture d'un octet.
        let c = unsafe { s.add(n).read() };
        if c == 0 {
            break;
        }
        n += 1;
    }
    n
}

pub(crate) fn slice_n<'a>(p: *const u8, n: usize) -> Result<&'a [u8], i32> {
    if n == 0 {
        return Ok(&[]);
    }
    if p.is_null() || n > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: p non nul, n borné par isize::MAX. Le contrat FFI fournit n octets.
    Ok(unsafe { core::slice::from_raw_parts(p, n) })
}

pub(crate) fn bytes_at(field: &[u8], n: usize) -> &[u8] {
    let n = n.min(field.len());
    &field[..n]
}

pub(crate) fn copy_bytes(dst: &mut [u8], src: &[u8]) {
    let n = src.len().min(dst.len());
    dst[..n].copy_from_slice(&src[..n]);
}

pub(crate) fn copy_ptr(dst: &mut [u8], src: *const u8, n: usize) {
    let n = n.min(dst.len());
    if n == 0 || src.is_null() {
        return;
    }
    // SAFETY: n borné par dst. L'appelant garantit n octets lisibles en src, sans recouvrement.
    unsafe { core::ptr::copy_nonoverlapping(src, dst.as_mut_ptr(), n) };
}

pub(crate) fn lit(s: &[u8]) -> *const c_char {
    s.as_ptr().cast()
}
