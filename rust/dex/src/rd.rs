use crate::{t_dex, t_dexcur};

const ITEM: [u32; 6] = [4, 4, 12, 8, 8, 32];

pub(crate) fn u16_ptr(p: *const u8) -> Option<u32> {
    if p.is_null() {
        return None;
    }
    // SAFETY: p non nul. dex_u16 exige deux octets lisibles.
    let b = unsafe { core::slice::from_raw_parts(p, 2) };
    Some(u32::from(b[0]) | (u32::from(b[1]) << 8))
}

pub(crate) fn u32_ptr(p: *const u8) -> Option<u32> {
    if p.is_null() {
        return None;
    }
    // SAFETY: p non nul. dex_u32 exige quatre octets lisibles.
    let b = unsafe { core::slice::from_raw_parts(p, 4) };
    Some(u32::from_le_bytes([b[0], b[1], b[2], b[3]]))
}

pub(crate) fn file(d: &t_dex) -> Option<&[u8]> {
    if d.len > isize::MAX as usize {
        return None;
    }
    if d.p.is_null() {
        return if d.len == 0 { Some(&[]) } else { None };
    }
    // SAFETY: p non nul, len borné par isize::MAX. t_dex décrit ce tampon.
    Some(unsafe { core::slice::from_raw_parts(d.p, d.len) })
}

pub(crate) fn fits(len: usize, off: u64, n: u64, size: u64) -> bool {
    if size == 0 {
        return false;
    }
    let len = len as u64;
    if off > len {
        return false;
    }
    n <= (len - off) / size
}

pub(crate) fn at_u16(d: &t_dex, off: usize) -> Option<u32> {
    let b = file(d)?;
    let end = off.checked_add(2)?;
    if end > b.len() {
        return None;
    }
    Some(u32::from(b[off]) | (u32::from(b[off + 1]) << 8))
}

pub(crate) fn at_u32(d: &t_dex, off: usize) -> Option<u32> {
    let b = file(d)?;
    let end = off.checked_add(4)?;
    if end > b.len() {
        return None;
    }
    Some(u32::from_le_bytes([
        b[off],
        b[off + 1],
        b[off + 2],
        b[off + 3],
    ]))
}

pub(crate) fn ptr_at(p: *const u8, len: usize, off: usize) -> Option<*const u8> {
    if off > len || off > isize::MAX as usize {
        return None;
    }
    if p.is_null() {
        return if len == 0 { Some(p) } else { None };
    }
    // SAFETY: off <= len et tient dans isize. Pointeur dans l'objet ou juste après.
    Some(unsafe { p.add(off) })
}

pub(crate) fn cur(d: &t_dex, off: u64) -> t_dexcur {
    let mut c = t_dexcur {
        p: d.p,
        len: d.len,
        pos: 0,
        err: 0,
    };
    if off >= d.len as u64 {
        c.err = 1;
    } else {
        c.pos = off as usize;
    }
    c
}

pub(crate) fn byte(c: &mut t_dexcur) -> u32 {
    if c.err != 0 || c.pos >= c.len || c.p.is_null() || c.pos > isize::MAX as usize {
        c.err = 1;
        return 0;
    }
    // SAFETY: pos < len, pos tient dans isize, p non nul. Le curseur décrit ce tampon.
    let b = unsafe { *c.p.add(c.pos) };
    c.pos += 1;
    u32::from(b)
}

pub(crate) fn uleb(c: &mut t_dexcur) -> u32 {
    let mut v = 0u32;
    let mut shift = 0u32;
    while shift < 35 {
        let b = byte(c);
        if c.err != 0 || (shift == 28 && b > 0x0f) {
            c.err = 1;
            return 0;
        }
        v |= (b & 0x7f).wrapping_shl(shift);
        if b & 0x80 == 0 {
            return v;
        }
        shift += 7;
    }
    c.err = 1;
    0
}

pub(crate) fn sleb(c: &mut t_dexcur) -> i32 {
    let mut v = 0u32;
    let mut b = 0x80u32;
    let mut shift = 0u32;
    while shift < 35 && b & 0x80 != 0 {
        b = byte(c);
        if c.err != 0 || (shift == 28 && b & 0x80 != 0) {
            c.err = 1;
            return 0;
        }
        v |= (b & 0x7f).wrapping_shl(shift);
        shift += 7;
    }
    if shift < 32 && b & 0x40 != 0 {
        v |= u32::MAX.wrapping_shl(shift);
    }
    v as i32
}

pub(crate) fn ulebp1(c: &mut t_dexcur) -> u32 {
    uleb(c).wrapping_sub(1)
}

pub(crate) fn item_size(table: u32) -> Option<u64> {
    let table = usize::try_from(table).ok()?;
    ITEM.get(table).copied().map(u64::from)
}

pub(crate) fn item_off(d: &t_dex, table: u32, i: u32) -> Option<usize> {
    let size = item_size(table)?;
    let base = usize::try_from(*d.off.get(usize::try_from(table).ok()?)?).ok()?;
    let step = usize::try_from(u64::from(i).checked_mul(size)?).ok()?;
    base.checked_add(step)
}
