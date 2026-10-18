use crate::mutf8::check;
use crate::rd::{at_u16, at_u32, cur, fits, item_off, ptr_at, u16_ptr, uleb};
use crate::{
    DEX_NO_INDEX, DEX_T_FIELD, DEX_T_METHOD, DEX_T_PROTO, DEX_T_STRING, DEX_T_TYPE, E_INVAL, E_OK,
    E_RANGE, t_dex, t_dexfield, t_dexlist, t_dexmethod, t_dexproto, t_dexstr, t_span,
};
use core::ffi::c_int;

pub(crate) fn string(d: &t_dex, idx: u32, out: &mut t_dexstr) -> c_int {
    if idx >= d.n[DEX_T_STRING as usize] {
        return E_RANGE;
    }
    let Some(at) = item_off(d, DEX_T_STRING, idx) else {
        return E_INVAL;
    };
    let Some(data_off) = at_u32(d, at) else {
        return E_INVAL;
    };
    let mut c = cur(d, u64::from(data_off));
    out.utf16 = uleb(&mut c);
    if c.err != 0 || c.pos > d.len {
        return E_INVAL;
    }
    let Some(p) = ptr_at(d.p, d.len, c.pos) else {
        return E_INVAL;
    };
    let mut size = 0u32;
    let mut units = 0u32;
    let span = t_span {
        p,
        len: d.len - c.pos,
    };
    if check(span, &mut size, &mut units) != E_OK || units != out.utf16 {
        return E_INVAL;
    }
    out.size = size;
    out.p = p.cast();
    E_OK
}

pub(crate) fn type_id(d: &t_dex, type_idx: u32, out: &mut t_dexstr) -> c_int {
    if type_idx >= d.n[DEX_T_TYPE as usize] {
        return E_RANGE;
    }
    let Some(at) = item_off(d, DEX_T_TYPE, type_idx) else {
        return E_INVAL;
    };
    let Some(idx) = at_u32(d, at) else {
        return E_INVAL;
    };
    string(d, idx, out)
}

pub(crate) fn type_list(d: &t_dex, off: u32, out: &mut t_dexlist) -> c_int {
    out.p = core::ptr::null();
    out.n = 0;
    if off == 0 {
        return E_OK;
    }
    if off & 3 != 0 || !fits(d.len, u64::from(off), 4, 1) {
        return E_INVAL;
    }
    let Some(count) = at_u32(d, off as usize) else {
        return E_INVAL;
    };
    let base = u64::from(off) + 4;
    if !fits(d.len, base, u64::from(count), 2) {
        return E_INVAL;
    }
    let mut i = 0u32;
    while i < count {
        let Some(at) = base.checked_add(2 * u64::from(i)) else {
            return E_INVAL;
        };
        let Some(at) = usize::try_from(at).ok() else {
            return E_INVAL;
        };
        let Some(ty) = at_u16(d, at) else {
            return E_INVAL;
        };
        if ty >= d.n[DEX_T_TYPE as usize] {
            return E_INVAL;
        }
        i += 1;
    }
    let Some(p) = usize::try_from(base)
        .ok()
        .and_then(|at| ptr_at(d.p, d.len, at))
    else {
        return E_INVAL;
    };
    out.p = p;
    out.n = i;
    E_OK
}

pub(crate) fn proto(d: &t_dex, proto_idx: u32, out: &mut t_dexproto) -> c_int {
    if proto_idx >= d.n[DEX_T_PROTO as usize] {
        return E_RANGE;
    }
    let Some(at) = item_off(d, DEX_T_PROTO, proto_idx) else {
        return E_INVAL;
    };
    let Some(shorty) = at_u32(d, at) else {
        return E_INVAL;
    };
    let Some(ret) = at_u32(d, at + 4) else {
        return E_INVAL;
    };
    let Some(list) = at_u32(d, at + 8) else {
        return E_INVAL;
    };
    out.shorty_idx = shorty;
    out.return_idx = ret;
    type_list(d, list, &mut out.params)
}

pub(crate) fn field(d: &t_dex, field_idx: u32, out: &mut t_dexfield) -> c_int {
    if field_idx >= d.n[DEX_T_FIELD as usize] {
        return E_RANGE;
    }
    let Some(at) = item_off(d, DEX_T_FIELD, field_idx) else {
        return E_INVAL;
    };
    let Some(class_idx) = at_u16(d, at) else {
        return E_INVAL;
    };
    let Some(type_idx) = at_u16(d, at + 2) else {
        return E_INVAL;
    };
    let Some(name_idx) = at_u32(d, at + 4) else {
        return E_INVAL;
    };
    out.class_idx = class_idx;
    out.type_idx = type_idx;
    out.name_idx = name_idx;
    E_OK
}

pub(crate) fn method(d: &t_dex, method_idx: u32, out: &mut t_dexmethod) -> c_int {
    if method_idx >= d.n[DEX_T_METHOD as usize] {
        return E_RANGE;
    }
    let Some(at) = item_off(d, DEX_T_METHOD, method_idx) else {
        return E_INVAL;
    };
    let Some(class_idx) = at_u16(d, at) else {
        return E_INVAL;
    };
    let Some(proto_idx) = at_u16(d, at + 2) else {
        return E_INVAL;
    };
    let Some(name_idx) = at_u32(d, at + 4) else {
        return E_INVAL;
    };
    out.class_idx = class_idx;
    out.proto_idx = proto_idx;
    out.name_idx = name_idx;
    E_OK
}

pub(crate) fn list_at(l: &t_dexlist, i: u32) -> u32 {
    if i >= l.n || l.p.is_null() {
        return DEX_NO_INDEX;
    }
    let Some(off) = usize::try_from(i).ok().and_then(|n| n.checked_mul(2)) else {
        return DEX_NO_INDEX;
    };
    if off > isize::MAX as usize {
        return DEX_NO_INDEX;
    }
    // SAFETY: i < n, p non nul, off = 2*i. Le contrat de dex_list_at exige 2*n octets.
    let p = unsafe { l.p.add(off) };
    match u16_ptr(p) {
        Some(v) => v,
        None => DEX_NO_INDEX,
    }
}
