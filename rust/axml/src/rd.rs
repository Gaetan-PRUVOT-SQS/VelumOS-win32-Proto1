use crate::{E_INVAL, RES_SIZE_MAX, t_reschunk, t_span};

pub(crate) fn bytes(span: t_span) -> Result<&'static [u8], i32> {
    if span.p.is_null() {
        return Err(E_INVAL);
    }
    if span.len == 0 {
        return Ok(&[]);
    }
    if span.len > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: p non nul, len borné par isize::MAX. Le contrat FFI exige len octets initialisés.
    Ok(unsafe { core::slice::from_raw_parts(span.p, span.len) })
}

pub(crate) fn rd16(b: &[u8], off: usize) -> Option<u32> {
    let end = off.checked_add(2)?;
    let s = b.get(off..end)?;
    Some(u32::from(s[0]) | (u32::from(s[1]) << 8))
}

pub(crate) fn rd32(b: &[u8], off: usize) -> Option<u32> {
    let lo = rd16(b, off)?;
    let hi = rd16(b, off.checked_add(2)?)?;
    Some(lo | (hi << 16))
}

pub(crate) fn byte_at(b: &[u8], off: u64) -> Option<u8> {
    let i = usize::try_from(off).ok()?;
    b.get(i).copied()
}

pub(crate) fn rd16_at(b: &[u8], off: u64) -> Option<u32> {
    rd16(b, usize::try_from(off).ok()?)
}

pub(crate) fn rd32_at(b: &[u8], off: u64) -> Option<u32> {
    rd32(b, usize::try_from(off).ok()?)
}

pub(crate) fn chunk(b: &[u8], off: usize) -> Result<t_reschunk, i32> {
    if off > b.len()
        || b.len() - off < 8
        || u64::try_from(off).unwrap_or(u64::MAX) > u64::from(RES_SIZE_MAX)
    {
        return Err(E_INVAL);
    }
    let kind = rd16(b, off).ok_or(E_INVAL)?;
    let hsize = rd16(b, off.checked_add(2).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    let size = rd32(b, off.checked_add(4).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    if hsize < 8 || hsize > size || size > RES_SIZE_MAX {
        return Err(E_INVAL);
    }
    let remain = b.len() - off;
    if usize::try_from(size).map(|n| n > remain).unwrap_or(true) {
        return Err(E_INVAL);
    }
    let off_u = u32::try_from(off).map_err(|_| E_INVAL)?;
    Ok(t_reschunk {
        off: off_u,
        r#type: kind,
        hsize,
        size,
    })
}

pub(crate) fn next(b: &[u8], pos: &mut u32) -> Result<Option<t_reschunk>, i32> {
    let cur = usize::try_from(*pos).map_err(|_| E_INVAL)?;
    if cur >= b.len() {
        return Ok(None);
    }
    let c = chunk(b, cur)?;
    *pos = pos.checked_add(c.size).ok_or(E_INVAL)?;
    Ok(Some(c))
}

pub(crate) fn span_at(base: *const u8, off: u32, len: u32) -> t_span {
    t_span {
        p: base.wrapping_add(off as usize),
        len: len as usize,
    }
}
