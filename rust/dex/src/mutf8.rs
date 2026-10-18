use crate::{DEX_NO_INDEX, E_INVAL, E_NOTSUP, E_OK, E_OVERFLOW, t_span, t_text};
use core::ffi::{c_char, c_int};

fn cont(p: &[u8], i: usize) -> bool {
    i < p.len() && p[i] & 0xc0 == 0x80
}

fn seq_len(p: &[u8], i: usize) -> usize {
    let b0 = p[i];
    if b0 < 0x80 {
        return 1;
    }
    if b0 & 0xe0 == 0xc0 && cont(p, i + 1) {
        let v = (u32::from(b0 & 0x1f) << 6) | u32::from(p[i + 1] & 0x3f);
        if v >= 0x80 || v == 0 {
            return 2;
        }
        return 0;
    }
    if b0 & 0xf0 == 0xe0 && cont(p, i + 1) && cont(p, i + 2) {
        let v = (u32::from(b0 & 0x0f) << 12) | (u32::from(p[i + 1] & 0x3f) << 6);
        if v >= 0x800 {
            return 3;
        }
    }
    0
}

pub(crate) fn check(s: t_span, size: &mut u32, units: &mut u32) -> c_int {
    if s.len > 0 && (s.p.is_null() || s.len > isize::MAX as usize) {
        return E_INVAL;
    }
    let bytes = if s.len == 0 {
        &[]
    } else {
        // SAFETY: p non nul, len > 0 et borné. Le contrat de dex_mutf8_check exige ces octets.
        unsafe { core::slice::from_raw_parts(s.p, s.len) }
    };
    let mut i = 0usize;
    let mut u = 0u32;
    while i < bytes.len() && bytes[i] != 0 {
        let n = seq_len(bytes, i);
        if n == 0 {
            return E_INVAL;
        }
        i += n;
        u = u.wrapping_add(1);
    }
    if i >= bytes.len() || i > 0x7fff_ffff {
        return E_INVAL;
    }
    let Ok(n) = u32::try_from(i) else {
        return E_INVAL;
    };
    *size = n;
    *units = u;
    E_OK
}

fn at(data: &[u8], i: usize) -> u8 {
    if i < data.len() { data[i] } else { 0 }
}

fn unit(data: &[u8], i: &mut usize) -> u32 {
    if *i >= data.len() {
        return DEX_NO_INDEX;
    }
    let b0 = data[*i];
    if b0 < 0x80 {
        *i += 1;
        return u32::from(b0);
    }
    let b1 = at(data, *i + 1);
    if b0 & 0xe0 == 0xc0 && b1 & 0xc0 == 0x80 {
        let v = (u32::from(b0 & 0x1f) << 6) | u32::from(b1 & 0x3f);
        *i += 2;
        if v != 0 && v < 0x80 {
            return DEX_NO_INDEX;
        }
        return v;
    }
    let b2 = if b1 & 0xc0 == 0x80 {
        at(data, *i + 2)
    } else {
        0
    };
    if b0 & 0xf0 == 0xe0 && b1 & 0xc0 == 0x80 && b2 & 0xc0 == 0x80 {
        let v = (u32::from(b0 & 0x0f) << 12) | (u32::from(b1 & 0x3f) << 6) | u32::from(b2 & 0x3f);
        *i += 3;
        if v >= 0x800 {
            return v;
        }
    }
    DEX_NO_INDEX
}

fn point(data: &[u8], i: &mut usize) -> u32 {
    let hi = unit(data, i);
    if !(0xd800..=0xdfff).contains(&hi) {
        return hi;
    }
    if hi > 0xdbff {
        return DEX_NO_INDEX;
    }
    let lo = unit(data, i);
    if !(0xdc00..=0xdfff).contains(&lo) {
        return DEX_NO_INDEX;
    }
    0x1_0000 + ((hi - 0xd800) << 10) + (lo - 0xdc00)
}

fn put(cp: u32, tmp: &mut [u8; 4]) -> usize {
    if cp < 0x80 {
        tmp[0] = cp as u8;
        return 1;
    }
    if cp < 0x800 {
        tmp[0] = 0xc0 | (cp >> 6) as u8;
        tmp[1] = 0x80 | (cp & 0x3f) as u8;
        return 2;
    }
    if cp < 0x1_0000 {
        tmp[0] = 0xe0 | (cp >> 12) as u8;
        tmp[1] = 0x80 | ((cp >> 6) & 0x3f) as u8;
        tmp[2] = 0x80 | (cp & 0x3f) as u8;
        return 3;
    }
    tmp[0] = 0xf0 | (cp >> 18) as u8;
    tmp[1] = 0x80 | ((cp >> 12) & 0x3f) as u8;
    tmp[2] = 0x80 | ((cp >> 6) & 0x3f) as u8;
    tmp[3] = 0x80 | (cp & 0x3f) as u8;
    4
}

fn emit(out: *mut u8, cap: usize, o: &mut usize, cp: u32) -> c_int {
    let mut tmp = [0u8; 4];
    let n = put(cp, &mut tmp);
    let Some(room) = cap.checked_sub(*o) else {
        return E_OVERFLOW;
    };
    if n >= room {
        return E_OVERFLOW;
    }
    let at = *o;
    // SAFETY: at + n < cap, out non nul. cap tient dans isize, l'appelant possède le tampon.
    unsafe {
        core::ptr::copy_nonoverlapping(tmp.as_ptr(), out.add(at), n);
        *out.add(at + n) = 0;
    }
    *o += n;
    E_OK
}

pub(crate) fn to_utf8(input: *const c_char, out: t_text) -> c_int {
    if input.is_null() || out.p.is_null() || out.cap == 0 || out.cap > 0x7fff_ffff {
        return E_INVAL;
    }
    // SAFETY: input non nul et terminé par NUL. Contrat de mutf8_to_utf8.
    let data = unsafe { core::ffi::CStr::from_ptr(input) }.to_bytes();
    let dst = out.p.cast::<u8>();
    // SAFETY: cap >= 1 et tient dans isize, dst non nul. Tampon de sortie exclusif.
    unsafe { *dst = 0 };
    let mut i = 0usize;
    let mut o = 0usize;
    while i < data.len() {
        let cp = point(data, &mut i);
        if cp == DEX_NO_INDEX {
            return E_INVAL;
        }
        if cp == 0 {
            return E_NOTSUP;
        }
        if emit(dst, out.cap, &mut o, cp) != E_OK {
            return E_OVERFLOW;
        }
    }
    c_int::try_from(o).unwrap_or(E_OVERFLOW)
}
