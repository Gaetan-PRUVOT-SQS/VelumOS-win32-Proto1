use crate::ids::{string, type_id, type_list};
use crate::rd::{at_u16, at_u32, item_off};
use crate::{
    DEX_NO_INDEX, DEX_T_CLASS, DEX_T_FIELD, DEX_T_METHOD, DEX_T_PROTO, DEX_T_STRING, DEX_T_TYPE,
    E_INVAL, E_NOENT, E_OK, E_OVERFLOW, E_RANGE, t_dex, t_dexclass, t_dexlist, t_dexstr,
};
use core::ffi::c_int;

fn idx_ok(idx: u32, n: u32) -> bool {
    idx == DEX_NO_INDEX || idx < n
}

fn below_len(off: u32, len: usize) -> bool {
    u64::from(off) < len as u64
}

pub(crate) fn class_at(d: &t_dex, def_idx: u32, out: &mut t_dexclass) -> c_int {
    if def_idx >= d.n[DEX_T_CLASS as usize] {
        return E_RANGE;
    }
    let Some(at) = item_off(d, DEX_T_CLASS, def_idx) else {
        return E_INVAL;
    };
    let Some(class_idx) = at_u32(d, at) else {
        return E_INVAL;
    };
    let Some(access) = at_u32(d, at + 4) else {
        return E_INVAL;
    };
    let Some(superclass) = at_u32(d, at + 8) else {
        return E_INVAL;
    };
    let Some(source) = at_u32(d, at + 16) else {
        return E_INVAL;
    };
    let Some(data_off) = at_u32(d, at + 24) else {
        return E_INVAL;
    };
    let Some(values_off) = at_u32(d, at + 28) else {
        return E_INVAL;
    };
    out.class_idx = class_idx;
    out.access = access;
    out.superclass_idx = superclass;
    out.source_file_idx = source;
    out.class_data_off = data_off;
    out.static_values_off = values_off;
    if class_idx >= d.n[DEX_T_TYPE as usize]
        || !idx_ok(superclass, d.n[DEX_T_TYPE as usize])
        || !idx_ok(source, d.n[DEX_T_STRING as usize])
    {
        return E_INVAL;
    }
    let Some(notes) = at_u32(d, at + 20) else {
        return E_INVAL;
    };
    if !below_len(notes, d.len) || !below_len(data_off, d.len) || !below_len(values_off, d.len) {
        return E_INVAL;
    }
    let Some(ifaces) = at_u32(d, at + 12) else {
        return E_INVAL;
    };
    type_list(d, ifaces, &mut out.interfaces)
}

fn same(p: *const u8, size: u32, desc: &[u8]) -> bool {
    let Ok(n) = usize::try_from(size) else {
        return false;
    };
    let Some(end) = n.checked_add(1) else {
        return false;
    };
    if p.is_null() || end > isize::MAX as usize {
        return false;
    }
    // SAFETY: dex_string a placé un NUL à l'indice size. end = size + 1.
    let bytes = unsafe { core::slice::from_raw_parts(p, end) };
    bytes == desc
}

pub(crate) fn find(d: &t_dex, desc: &[u8]) -> c_int {
    let mut i = 0u32;
    let mut s = t_dexstr {
        p: core::ptr::null(),
        size: 0,
        utf16: 0,
    };
    while i < d.n[DEX_T_CLASS as usize] {
        let Some(at) = item_off(d, DEX_T_CLASS, i) else {
            i += 1;
            continue;
        };
        let Some(ty) = at_u32(d, at) else {
            i += 1;
            continue;
        };
        if type_id(d, ty, &mut s) == E_OK && same(s.p.cast(), s.size, desc) {
            return c_int::try_from(i).unwrap_or(E_OVERFLOW);
        }
        i += 1;
    }
    E_NOENT
}

fn verify_strings(d: &t_dex) -> c_int {
    let mut i = 0u32;
    let mut s = t_dexstr {
        p: core::ptr::null(),
        size: 0,
        utf16: 0,
    };
    while i < d.n[DEX_T_STRING as usize] {
        if string(d, i, &mut s) != E_OK {
            return E_INVAL;
        }
        i += 1;
    }
    E_OK
}

fn verify_types(d: &t_dex) -> c_int {
    let mut i = 0u32;
    while i < d.n[DEX_T_TYPE as usize] {
        let Some(at) = item_off(d, DEX_T_TYPE, i) else {
            return E_INVAL;
        };
        let Some(idx) = at_u32(d, at) else {
            return E_INVAL;
        };
        if idx >= d.n[DEX_T_STRING as usize] {
            return E_INVAL;
        }
        i += 1;
    }
    E_OK
}

fn verify_protos(d: &t_dex) -> c_int {
    let mut i = 0u32;
    let mut list = t_dexlist {
        p: core::ptr::null(),
        n: 0,
    };
    while i < d.n[DEX_T_PROTO as usize] {
        let Some(at) = item_off(d, DEX_T_PROTO, i) else {
            return E_INVAL;
        };
        let Some(shorty) = at_u32(d, at) else {
            return E_INVAL;
        };
        let Some(ret) = at_u32(d, at + 4) else {
            return E_INVAL;
        };
        let Some(off) = at_u32(d, at + 8) else {
            return E_INVAL;
        };
        if shorty >= d.n[DEX_T_STRING as usize]
            || ret >= d.n[DEX_T_TYPE as usize]
            || type_list(d, off, &mut list) != E_OK
        {
            return E_INVAL;
        }
        i += 1;
    }
    E_OK
}

fn verify_members(d: &t_dex, table: u32, mid_n: u32) -> c_int {
    let Some(n) = d.n.get(table as usize).copied() else {
        return E_INVAL;
    };
    let mut i = 0u32;
    while i < n {
        let Some(at) = item_off(d, table, i) else {
            return E_INVAL;
        };
        let Some(class_idx) = at_u16(d, at) else {
            return E_INVAL;
        };
        let Some(mid) = at_u16(d, at + 2) else {
            return E_INVAL;
        };
        let Some(name) = at_u32(d, at + 4) else {
            return E_INVAL;
        };
        if class_idx >= d.n[DEX_T_TYPE as usize]
            || mid >= mid_n
            || name >= d.n[DEX_T_STRING as usize]
        {
            return E_INVAL;
        }
        i += 1;
    }
    E_OK
}

pub(crate) fn verify_ids(d: &t_dex) -> Result<(), c_int> {
    if verify_strings(d) != E_OK || verify_types(d) != E_OK || verify_protos(d) != E_OK {
        return Err(E_INVAL);
    }
    if verify_members(d, DEX_T_FIELD, d.n[DEX_T_TYPE as usize]) != E_OK {
        return Err(E_INVAL);
    }
    if verify_members(d, DEX_T_METHOD, d.n[DEX_T_PROTO as usize]) != E_OK {
        return Err(E_INVAL);
    }
    Ok(())
}

pub(crate) fn verify_classes(d: &t_dex) -> Result<(), c_int> {
    let mut i = 0u32;
    let mut c = t_dexclass {
        class_idx: 0,
        access: 0,
        superclass_idx: 0,
        interfaces: t_dexlist {
            p: core::ptr::null(),
            n: 0,
        },
        source_file_idx: 0,
        class_data_off: 0,
        static_values_off: 0,
    };
    while i < d.n[DEX_T_CLASS as usize] {
        if class_at(d, i, &mut c) != E_OK {
            return Err(E_INVAL);
        }
        i += 1;
    }
    Ok(())
}
