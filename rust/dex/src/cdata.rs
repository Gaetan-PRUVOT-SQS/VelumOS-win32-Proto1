use crate::rd::{cur, uleb};
use crate::{
    DEX_M_DIRECT, DEX_T_FIELD, DEX_T_METHOD, E_INVAL, E_NOENT, E_OK, ref_in, t_dex, t_dexcdata,
    t_dexclass, t_dexmember,
};
use core::ffi::c_int;

pub(crate) fn open(dptr: *const t_dex, c: &t_dexclass, it: &mut t_dexcdata) -> c_int {
    let Ok(d) = ref_in(dptr) else {
        return E_INVAL;
    };
    it.d = dptr;
    it.kind = 4;
    it.last = 0;
    it.pos = 0;
    if c.class_data_off == 0 {
        return E_OK;
    }
    let mut cur = cur(d, u64::from(c.class_data_off));
    let mut i = 0usize;
    while i < 4 {
        it.left[i] = uleb(&mut cur);
        if cur.err != 0 || u64::from(it.left[i]) > d.len as u64 {
            return E_INVAL;
        }
        i += 1;
    }
    it.pos = cur.pos as u32;
    it.kind = 0;
    E_OK
}

fn member(it: &t_dexcdata, d: &t_dex, c: &mut crate::t_dexcur, out: &mut t_dexmember) -> c_int {
    let idx = u64::from(it.last).wrapping_add(u64::from(uleb(c)));
    out.access = uleb(c);
    out.code_off = 0;
    let mut limit = d.n[DEX_T_FIELD as usize];
    if it.kind >= DEX_M_DIRECT {
        limit = d.n[DEX_T_METHOD as usize];
        out.code_off = uleb(c);
    }
    if c.err != 0 || idx >= u64::from(limit) || u64::from(out.code_off) >= d.len as u64 {
        return E_INVAL;
    }
    let Ok(idx) = u32::try_from(idx) else {
        return E_INVAL;
    };
    out.kind = it.kind;
    out.idx = idx;
    E_OK
}

pub(crate) fn next(it: &mut t_dexcdata, out: &mut t_dexmember) -> c_int {
    let Ok(d) = ref_in(it.d) else {
        return E_INVAL;
    };
    while it.kind < 4 && it.left[it.kind as usize] == 0 {
        it.kind += 1;
        it.last = 0;
    }
    if it.kind >= 4 {
        return E_NOENT;
    }
    let mut c = cur(d, u64::from(it.pos));
    if member(it, d, &mut c, out) != E_OK {
        it.kind = 4;
        return E_INVAL;
    }
    let kind = it.kind as usize;
    it.left[kind] -= 1;
    it.last = out.idx;
    it.pos = c.pos as u32;
    E_OK
}
