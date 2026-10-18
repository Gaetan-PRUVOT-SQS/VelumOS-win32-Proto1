use core::ffi::c_int;

use crate::rd::{bytes, rd16, rd32};
use crate::utf::{put, seq};
use crate::{
    E_INVAL, E_OK, E_OVERFLOW, E_RANGE, RES_POOL_HEADER, RES_POOL_UTF8, RES_SIZE_MAX,
    RES_STRING_POOL, t_respool, t_span, t_text,
};

pub(crate) fn open(p: &mut t_respool, chunk: t_span) -> c_int {
    let b = match bytes(chunk) {
        Ok(b) => b,
        Err(e) => return e,
    };
    let c = match crate::rd::chunk(b, 0) {
        Ok(c) => c,
        Err(e) => return e,
    };
    if c.r#type != RES_STRING_POOL || c.hsize < RES_POOL_HEADER {
        return E_INVAL;
    }
    let Some(count) = rd32(b, 8) else {
        return E_INVAL;
    };
    let Some(flags) = rd32(b, 16) else {
        return E_INVAL;
    };
    let Some(strings) = rd32(b, 20) else {
        return E_INVAL;
    };
    p.p = chunk.p;
    p.size = c.size;
    p.hsize = c.hsize;
    p.count = count;
    p.utf8 = u32::from((flags & RES_POOL_UTF8) != 0);
    p.strings = strings;
    let need = u64::from(c.hsize).saturating_add(u64::from(count).saturating_mul(4));
    if need > u64::from(c.size) {
        return E_INVAL;
    }
    if count != 0 && (u64::from(strings) < need || strings >= c.size) {
        return E_INVAL;
    }
    E_OK
}

fn out_bytes(out: t_text, cap: usize) -> Result<&'static mut [u8], c_int> {
    if out.p.is_null() || cap == 0 || cap > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: cap octets inscriptibles, hors de la table. Le contrat FFI le garantit.
    Ok(unsafe { core::slice::from_raw_parts_mut(out.p.cast::<u8>(), cap) })
}

fn len8(b: &[u8], off: &mut u32) -> Result<u32, c_int> {
    let o = usize::try_from(*off).map_err(|_| E_INVAL)?;
    if o >= b.len() {
        return Err(E_INVAL);
    }
    let byte = b[o];
    *off = off.wrapping_add(1);
    if byte & 0x80 == 0 {
        return Ok(u32::from(byte));
    }
    let o2 = usize::try_from(*off).map_err(|_| E_INVAL)?;
    if o2 >= b.len() {
        return Err(E_INVAL);
    }
    let next = b[o2];
    *off = off.wrapping_add(1);
    Ok((u32::from(byte & 0x7f) << 8) | u32::from(next))
}

fn decode_u8(b: &[u8], mut off: u32, out: &mut [u8]) -> c_int {
    if len8(b, &mut off).is_err() {
        return E_INVAL;
    }
    let n = match len8(b, &mut off) {
        Ok(n) => n,
        Err(e) => return e,
    };
    let o = match usize::try_from(off) {
        Ok(o) => o,
        Err(_) => return E_INVAL,
    };
    let nu = match usize::try_from(n) {
        Ok(n) => n,
        Err(_) => return E_INVAL,
    };
    if o >= b.len() || nu >= b.len() - o {
        return E_INVAL;
    }
    if b[o + nu] != 0 {
        return E_INVAL;
    }
    if nu.checked_add(1).map(|m| m > out.len()).unwrap_or(true) {
        return E_OVERFLOW;
    }
    let mut i = 0usize;
    while i < nu {
        let k = seq(&b[o + i..o + nu]);
        if k == 0 {
            return E_INVAL;
        }
        let ku = usize::try_from(k).unwrap_or(0);
        if ku == 0 || i.checked_add(ku).map(|e| e > nu).unwrap_or(true) {
            return E_INVAL;
        }
        let mut left = ku;
        while left > 0 {
            out[i] = b[o + i];
            i = i.saturating_add(1);
            left -= 1;
        }
    }
    out[nu] = 0;
    i32::try_from(nu).unwrap_or(E_OVERFLOW)
}

fn len16(b: &[u8], off: &mut u32) -> Result<u32, c_int> {
    let o = usize::try_from(*off).map_err(|_| E_INVAL)?;
    if o > b.len() || b.len() - o < 2 {
        return Err(E_INVAL);
    }
    let mut n = rd16(b, o).ok_or(E_INVAL)?;
    *off = off.wrapping_add(2);
    if n & 0x8000 == 0 {
        return Ok(n);
    }
    let o2 = usize::try_from(*off).map_err(|_| E_INVAL)?;
    if o2 > b.len() || b.len() - o2 < 2 {
        return Err(E_INVAL);
    }
    let lo = rd16(b, o2).ok_or(E_INVAL)?;
    *off = off.wrapping_add(2);
    n = ((n & 0x7fff) << 16) | lo;
    Ok(n)
}

fn codepoint(s: &[u8], left: u32) -> Result<(u32, u32), c_int> {
    let cp = rd16(s, 0).ok_or(E_INVAL)?;
    if cp == 0 || (0xdc00..=0xdfff).contains(&cp) {
        return Err(E_INVAL);
    }
    if !(0xd800..=0xdbff).contains(&cp) {
        return Ok((cp, 1));
    }
    if left < 2 {
        return Err(E_INVAL);
    }
    let lo = rd16(s, 2).ok_or(E_INVAL)?;
    if !(0xdc00..=0xdfff).contains(&lo) {
        return Err(E_INVAL);
    }
    let cp = 0x1_0000
        + u32::from(u16::try_from(cp - 0xd800).unwrap_or(0)) * 1024
        + u32::from(u16::try_from(lo - 0xdc00).unwrap_or(0));
    Ok((cp, 2))
}

fn decode_u16(b: &[u8], mut off: u32, out: &mut [u8]) -> c_int {
    let n = match len16(b, &mut off) {
        Ok(n) => n,
        Err(e) => return e,
    };
    let o = u64::from(off);
    let len = u64::try_from(b.len()).unwrap_or(0);
    let left = len.saturating_sub(o);
    if u64::from(n) >= left / 2 {
        return E_INVAL;
    }
    let nul_at = o.saturating_add(u64::from(n).saturating_mul(2));
    if crate::rd::rd16_at(b, nul_at) != Some(0) {
        return E_INVAL;
    }
    let mut i = 0u32;
    let mut w = 0usize;
    while i < n {
        let at = o.saturating_add(u64::from(i).saturating_mul(2));
        let Some(start) = usize::try_from(at).ok() else {
            return E_INVAL;
        };
        let Some(rest) = b.get(start..) else {
            return E_INVAL;
        };
        let left_units = n.wrapping_sub(i);
        let (cp, k) = match codepoint(rest, left_units) {
            Ok(v) => v,
            Err(e) => return e,
        };
        let wrote = put(cp, out, w);
        if wrote == 0 {
            return E_OVERFLOW;
        }
        w = w.saturating_add(usize::try_from(wrote).unwrap_or(0));
        i = match i.checked_add(k) {
            Some(v) => v,
            None => return E_INVAL,
        };
    }
    let Some(slot) = out.get_mut(w) else {
        return E_OVERFLOW;
    };
    *slot = 0;
    i32::try_from(w).unwrap_or(E_OVERFLOW)
}

pub(crate) fn get_copied(p: t_respool, index: u32, out: t_text) -> c_int {
    if p.p.is_null() || out.p.is_null() || out.cap == 0 {
        return E_INVAL;
    }
    if index >= p.count {
        return E_RANGE;
    }
    let cap = if out.cap > RES_SIZE_MAX as usize {
        RES_SIZE_MAX as usize
    } else {
        out.cap
    };
    let b = match bytes(t_span {
        p: p.p,
        len: p.size as usize,
    }) {
        Ok(b) => b,
        Err(e) => return e,
    };
    let at = u64::from(p.hsize).saturating_add(u64::from(index).saturating_mul(4));
    let Some(rel) = rd32(b, usize::try_from(at).unwrap_or(usize::MAX)) else {
        return E_INVAL;
    };
    let abs = u64::from(rel).saturating_add(u64::from(p.strings));
    if abs >= u64::from(p.size) {
        return E_INVAL;
    }
    let Some(off) = u32::try_from(abs).ok() else {
        return E_INVAL;
    };
    let dest = match out_bytes(out, cap) {
        Ok(d) => d,
        Err(e) => return e,
    };
    if p.utf8 != 0 {
        decode_u8(b, off, dest)
    } else {
        decode_u16(b, off, dest)
    }
}
