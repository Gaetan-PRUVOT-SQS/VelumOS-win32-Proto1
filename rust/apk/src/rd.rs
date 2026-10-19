use crate::E_ACCES;
use core::ffi::c_int;

pub(crate) fn rd32(b: &[u8]) -> Option<u32> {
    let s = b.get(..4)?;
    Some(u32::from_le_bytes([s[0], s[1], s[2], s[3]]))
}

pub(crate) fn rd32_at(b: &[u8], i: usize) -> Option<u32> {
    let end = i.checked_add(4)?;
    rd32(b.get(i..end)?)
}

pub(crate) fn rd64(b: &[u8]) -> Option<u64> {
    let lo = u64::from(rd32(b)?);
    let hi = u64::from(rd32(b.get(4..)?)?);
    Some(lo | (hi << 32))
}

pub(crate) fn rd64_at(b: &[u8], i: usize) -> Option<u64> {
    let end = i.checked_add(8)?;
    rd64(b.get(i..end)?)
}

pub(crate) fn apk_lp<'a>(input: &mut &'a [u8]) -> Result<&'a [u8], c_int> {
    if input.len() < 4 {
        return Err(E_ACCES);
    }
    let Some(n32) = rd32(input) else {
        return Err(E_ACCES);
    };
    let Ok(n) = usize::try_from(n32) else {
        return Err(E_ACCES);
    };
    let rest = input.len() - 4;
    if n > rest {
        return Err(E_ACCES);
    }
    let body = &input[4..4 + n];
    *input = &input[4 + n..];
    Ok(body)
}

pub(crate) fn cstr_len(s: &[u8]) -> usize {
    match s.iter().position(|&c| c == 0) {
        Some(i) => i,
        None => s.len(),
    }
}

pub(crate) fn cstr_eq(buf: &[u8], lit: &[u8]) -> bool {
    let n = cstr_len(buf);
    n == lit.len() && &buf[..n] == lit
}

pub(crate) fn cstr_has(buf: &[u8], c: u8) -> bool {
    let n = cstr_len(buf);
    buf[..n].contains(&c)
}

pub(crate) fn text_clean(s: &[u8]) -> c_int {
    let n = cstr_len(s);
    let mut i = 0usize;
    while i < n {
        let c = s[i];
        if c < 0x20 || c == 0x7f {
            return crate::E_INVAL;
        }
        i += 1;
    }
    crate::E_OK
}

pub(crate) fn strlcpy(dst: &mut [u8], src: &[u8]) -> usize {
    let n = cstr_len(src);
    if dst.is_empty() {
        return n;
    }
    let copy = n.min(dst.len() - 1);
    dst[..copy].copy_from_slice(&src[..copy]);
    dst[copy] = 0;
    n
}

pub(crate) fn strlcat(dst: &mut [u8], src: &[u8]) -> usize {
    let slen = cstr_len(src);
    let dlen = cstr_len(dst);
    if dlen >= dst.len() {
        return dst.len().saturating_add(slen);
    }
    let room = dst.len() - dlen;
    let copy = slen.min(room - 1);
    dst[dlen..dlen + copy].copy_from_slice(&src[..copy]);
    dst[dlen + copy] = 0;
    match dlen.checked_add(slen) {
        Some(v) => v,
        None => usize::MAX,
    }
}

pub(crate) fn low32(n: usize) -> u32 {
    (n & 0xffff_ffff) as u32
}

pub(crate) fn uz(n: u32) -> usize {
    usize::try_from(n).unwrap_or(usize::MAX)
}
