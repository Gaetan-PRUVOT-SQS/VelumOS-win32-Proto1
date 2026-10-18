use crate::read::{rd32_at, unit_at};
use crate::tab::{DP_ARRAY, DP_PACKED, DP_SPARSE};
use crate::{E_INVAL, E_OK};

pub(crate) struct Payload {
    pub ident: u32,
    pub count: u32,
    pub width: u32,
    pub units: u64,
}

pub(crate) struct SwitchView {
    pub sparse: u8,
    pub count: u32,
    pub first_key: i32,
    pub keys_off: Option<usize>,
    pub targets_off: usize,
}

pub(crate) struct ArrayView {
    pub width: u32,
    pub count: u32,
    pub data_off: usize,
}

fn byte_off(pc: u32) -> Option<usize> {
    usize::try_from(u64::from(pc).checked_mul(2)?).ok()
}

fn array_shape(code: &[u8], pc: u32, left: u64) -> Result<(u32, u32, u64), i32> {
    if left < 4 {
        return Err(E_INVAL);
    }
    let width = unit_at(code, pc.checked_add(1).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    let lo = unit_at(code, pc.checked_add(2).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    let hi = unit_at(code, pc.checked_add(3).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    let count = lo | (hi << 16);
    if width != 1 && width != 2 && width != 4 && width != 8 {
        return Err(E_INVAL);
    }
    let prod = u64::from(count)
        .checked_mul(u64::from(width))
        .ok_or(E_INVAL)?;
    let units = prod.checked_add(1).ok_or(E_INVAL)? / 2;
    Ok((width, count, units.checked_add(4).ok_or(E_INVAL)?))
}

pub(crate) fn payload_at(code: &[u8], pc: u32) -> Result<Payload, i32> {
    if pc & 1 != 0 {
        return Err(E_INVAL);
    }
    let n = (code.len() / 2) as u64;
    let pc64 = u64::from(pc);
    if pc64.checked_add(2).ok_or(E_INVAL)? > n {
        return Err(E_INVAL);
    }
    let left = n.checked_sub(pc64).ok_or(E_INVAL)?;
    let ident = unit_at(code, pc).ok_or(E_INVAL)?;
    let count = unit_at(code, pc.checked_add(1).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    let mut pl = Payload {
        ident,
        count,
        width: 0,
        units: 0,
    };
    let mut err = E_OK;
    if ident == DP_PACKED {
        pl.units = 4 + 2 * u64::from(count);
    } else if ident == DP_SPARSE {
        pl.units = 2 + 4 * u64::from(count);
    } else if ident == DP_ARRAY {
        match array_shape(code, pc, left) {
            Ok((width, count, units)) => {
                pl.width = width;
                pl.count = count;
                pl.units = units;
            }
            Err(e) => err = e,
        }
    } else {
        err = E_INVAL;
    }
    if err < 0 || pl.units > left {
        return Err(E_INVAL);
    }
    Ok(pl)
}

pub(crate) fn keys_ok(code: &[u8], pc: u32, pl: &Payload) -> bool {
    let Some(base) = byte_off(pc).and_then(|off| off.checked_add(4)) else {
        return false;
    };
    if pl.ident == DP_PACKED {
        if pl.count == 0 {
            return true;
        }
        let Some(key) = rd32_at(code, base) else {
            return false;
        };
        let Some(last) = i64::from(key)
            .checked_add(i64::from(pl.count))
            .and_then(|v| v.checked_sub(1))
        else {
            return false;
        };
        return last <= i64::from(i32::MAX);
    }
    let mut i = 1u32;
    while pl.ident == DP_SPARSE && i < pl.count {
        let Some(step) = usize::try_from(i).ok().and_then(|n| n.checked_mul(4)) else {
            return false;
        };
        let Some(cur) = base.checked_add(step) else {
            return false;
        };
        let Some(prev) = cur.checked_sub(4) else {
            return false;
        };
        let (Some(a), Some(b)) = (rd32_at(code, cur), rd32_at(code, prev)) else {
            return false;
        };
        if a <= b {
            return false;
        }
        i += 1;
    }
    true
}

pub(crate) fn switch_at(code: &[u8], pc: u32) -> Result<SwitchView, i32> {
    let pl = payload_at(code, pc)?;
    if pl.ident == DP_ARRAY {
        return Err(E_INVAL);
    }
    let base = byte_off(pc).ok_or(E_INVAL)?;
    let mut view = SwitchView {
        sparse: u8::from(pl.ident == DP_SPARSE),
        count: pl.count,
        first_key: 0,
        keys_off: None,
        targets_off: base.checked_add(8).ok_or(E_INVAL)?,
    };
    if view.sparse == 0 || pl.count > 0 {
        let off = base.checked_add(4).ok_or(E_INVAL)?;
        view.first_key = rd32_at(code, off).ok_or(E_INVAL)?;
    }
    if view.sparse != 0 {
        let keys = base.checked_add(4).ok_or(E_INVAL)?;
        let span = usize::try_from(pl.count)
            .map_err(|_| E_INVAL)?
            .checked_mul(4)
            .ok_or(E_INVAL)?;
        view.keys_off = Some(keys);
        view.targets_off = keys.checked_add(span).ok_or(E_INVAL)?;
    }
    if view.targets_off > code.len() {
        return Err(E_INVAL);
    }
    if view.keys_off.is_some_and(|off| off > code.len()) {
        return Err(E_INVAL);
    }
    Ok(view)
}

pub(crate) fn array_at(code: &[u8], pc: u32) -> Result<ArrayView, i32> {
    let pl = payload_at(code, pc)?;
    if pl.ident != DP_ARRAY {
        return Err(E_INVAL);
    }
    let data_off = byte_off(pc).ok_or(E_INVAL)?.checked_add(8).ok_or(E_INVAL)?;
    if data_off > code.len() {
        return Err(E_INVAL);
    }
    Ok(ArrayView {
        width: pl.width,
        count: pl.count,
        data_off,
    })
}
