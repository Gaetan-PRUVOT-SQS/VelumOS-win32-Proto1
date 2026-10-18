use crate::tab::{
    DL_32, DL_64, DL_AA, DL_B4, DL_W1, DL_W1H, DS_A4, DS_AA, DS_B4, DS_L32, DS_W1, DS_W1H, DS_W1L,
    DS_W2, DS_W3,
};

pub(crate) fn low_u8(v: u32) -> u8 {
    u8::try_from(v & 0xff).unwrap_or_default()
}

pub(crate) fn low_u16(v: u32) -> u16 {
    u16::try_from(v & 0xffff).unwrap_or_default()
}

pub(crate) fn unit(code: &[u8], index: u64) -> Option<u32> {
    let off = usize::try_from(index.checked_mul(2)?).ok()?;
    let b0 = *code.get(off)?;
    let b1 = *code.get(off.checked_add(1)?)?;
    Some(u32::from(b0) | (u32::from(b1) << 8))
}

pub(crate) fn unit_at(code: &[u8], pc: u32) -> Option<u32> {
    unit(code, u64::from(pc))
}

pub(crate) fn field(code: &[u8], src: u8) -> Option<u32> {
    match src {
        DS_AA => Some(unit(code, 0)? >> 8),
        DS_A4 => Some((unit(code, 0)? >> 8) & 15),
        DS_B4 => Some(unit(code, 0)? >> 12),
        DS_W1 => unit(code, 1),
        DS_W2 => unit(code, 2),
        DS_W3 => unit(code, 3),
        DS_W1L => Some(unit(code, 1)? & 0xff),
        DS_W1H => Some(unit(code, 1)? >> 8),
        DS_L32 => Some(unit(code, 1)? | (unit(code, 2)? << 16)),
        _ => Some(0),
    }
}

pub(crate) fn sext(v: u32, sign: u32) -> i64 {
    let x = u64::from(v ^ sign);
    let s = u64::from(sign);
    i64::from_ne_bytes(x.wrapping_sub(s).to_ne_bytes())
}

pub(crate) fn literal(code: &[u8], src: u8) -> Option<i64> {
    match src {
        DL_B4 => Some(sext(field(code, DS_B4)?, 8)),
        DL_AA => Some(sext(field(code, DS_AA)?, 0x80)),
        DL_W1 => Some(sext(field(code, DS_W1)?, 0x8000)),
        DL_W1H => Some(sext(field(code, DS_W1H)?, 0x80)),
        DL_32 => Some(sext(field(code, DS_L32)?, 0x8000_0000)),
        DL_64 => {
            let hi = u64::from(unit(code, 3)?) | (u64::from(unit(code, 4)?) << 16);
            let low = u64::from(field(code, DS_L32)?);
            Some(i64::from_ne_bytes((low | (hi << 32)).to_ne_bytes()))
        }
        _ => Some(0),
    }
}

pub(crate) fn rd32_bytes(code: &[u8]) -> Option<i32> {
    let v = unit(code, 0)? | (unit(code, 1)? << 16);
    i32::try_from(sext(v, 0x8000_0000)).ok()
}

pub(crate) fn rd32_at(code: &[u8], off: usize) -> Option<i32> {
    rd32_bytes(code.get(off..)?)
}
