use crate::rd::{at_u16, at_u32, cur, fits, ptr_at, sleb, uleb};
use crate::{
    DEX_NO_INDEX, DEX_T_TYPE, E_INVAL, E_NOENT, E_OK, E_OVERFLOW, E_RANGE, ref_in, t_dex,
    t_dexcatch, t_dexcode, t_dexhit,
};
use core::ffi::c_int;

fn tries_area(d: &t_dex, out: &mut t_dexcode, end: u64) -> c_int {
    out.tries_off = 0;
    out.handlers_off = 0;
    if out.tries == 0 {
        return E_OK;
    }
    let Some(end) = end.checked_add(3) else {
        return E_INVAL;
    };
    let end = end & !3u64;
    if !fits(d.len, end, u64::from(out.tries), 8) {
        return E_INVAL;
    }
    let Some(handlers) = end.checked_add(8 * u64::from(out.tries)) else {
        return E_INVAL;
    };
    if !fits(d.len, handlers, 1, 1) {
        return E_INVAL;
    }
    let Ok(tries_off) = u32::try_from(end) else {
        return E_INVAL;
    };
    let Ok(handlers_off) = u32::try_from(handlers) else {
        return E_INVAL;
    };
    out.tries_off = tries_off;
    out.handlers_off = handlers_off;
    E_OK
}

pub(crate) fn code_item(d: &t_dex, code_off: u32, out: &mut t_dexcode) -> c_int {
    if code_off < 0x70 || code_off & 3 != 0 || !fits(d.len, u64::from(code_off), 16, 1) {
        return E_INVAL;
    }
    let Ok(base) = usize::try_from(code_off) else {
        return E_INVAL;
    };
    let Some(registers) = at_u16(d, base) else {
        return E_INVAL;
    };
    let Some(ins) = at_u16(d, base + 2) else {
        return E_INVAL;
    };
    let Some(outs) = at_u16(d, base + 4) else {
        return E_INVAL;
    };
    let Some(tries) = at_u16(d, base + 6) else {
        return E_INVAL;
    };
    let Some(insns_size) = at_u32(d, base + 12) else {
        return E_INVAL;
    };
    let Some(insns) = ptr_at(d.p, d.len, base + 16) else {
        return E_INVAL;
    };
    out.off = code_off;
    out.registers = registers;
    out.ins = ins;
    out.outs = outs;
    out.tries = tries;
    out.insns_size = insns_size;
    out.insns = insns;
    if ins > registers || !fits(d.len, u64::from(code_off) + 16, u64::from(insns_size), 2) {
        return E_INVAL;
    }
    let end = u64::from(code_off) + 16 + 2 * u64::from(insns_size);
    tries_area(d, out, end)
}

pub(crate) fn try_find(d: &t_dex, code: &t_dexcode, pc: u32) -> c_int {
    if code.tries > 0xffff || !fits(d.len, u64::from(code.tries_off), u64::from(code.tries), 8) {
        return E_INVAL;
    }
    let mut i = 0u32;
    while i < code.tries {
        let Some(base) = u64::from(code.tries_off).checked_add(8 * u64::from(i)) else {
            return E_INVAL;
        };
        let Some(base) = usize::try_from(base).ok() else {
            return E_INVAL;
        };
        let Some(start) = at_u32(d, base) else {
            return E_INVAL;
        };
        let Some(insn_at) = base.checked_add(4) else {
            return E_INVAL;
        };
        let Some(count) = at_u16(d, insn_at) else {
            return E_INVAL;
        };
        if pc >= start && pc - start < count {
            return c_int::try_from(i).unwrap_or(E_OVERFLOW);
        }
        i += 1;
    }
    E_NOENT
}

fn start(it: &mut t_dexhit, c: &crate::t_dexcur, n: i32) -> c_int {
    let count = n.unsigned_abs();
    if c.err != 0 || u64::from(count) > c.len as u64 {
        return E_INVAL;
    }
    let Ok(pos) = u32::try_from(c.pos) else {
        return E_INVAL;
    };
    it.catch_all = u32::from(n <= 0);
    it.left = count;
    it.pos = pos;
    E_OK
}

pub(crate) fn handler_open(
    dptr: *const t_dex,
    code: &t_dexcode,
    try_index: u32,
    it: &mut t_dexhit,
) -> c_int {
    let Ok(d) = ref_in(dptr) else {
        return E_INVAL;
    };
    it.d = dptr;
    it.left = 0;
    it.catch_all = 0;
    it.pos = 0;
    it.insns_size = code.insns_size;
    if try_index >= code.tries {
        return E_RANGE;
    }
    if code.tries > 0xffff || !fits(d.len, u64::from(code.tries_off), u64::from(code.tries), 8) {
        return E_INVAL;
    }
    let Some(at) = u64::from(code.tries_off)
        .checked_add(8 * u64::from(try_index))
        .and_then(|v| v.checked_add(6))
    else {
        return E_INVAL;
    };
    let Some(at) = usize::try_from(at).ok() else {
        return E_INVAL;
    };
    let Some(rel) = at_u16(d, at) else {
        return E_INVAL;
    };
    let Some(hoff) = u64::from(code.handlers_off).checked_add(u64::from(rel)) else {
        return E_INVAL;
    };
    let mut c = cur(d, hoff);
    let n = sleb(&mut c);
    start(it, &c, n)
}

pub(crate) fn handler_next(it: &mut t_dexhit, out: &mut t_dexcatch) -> c_int {
    let Ok(d) = ref_in(it.d) else {
        return E_INVAL;
    };
    if it.left == 0 && it.catch_all == 0 {
        return E_NOENT;
    }
    let mut c = cur(d, u64::from(it.pos));
    out.type_idx = DEX_NO_INDEX;
    if it.left > 0 {
        out.type_idx = uleb(&mut c);
    }
    out.addr = uleb(&mut c);
    if c.err != 0
        || out.addr >= it.insns_size
        || (it.left > 0 && out.type_idx >= d.n[DEX_T_TYPE as usize])
    {
        it.left = 0;
        it.catch_all = 0;
        return E_INVAL;
    }
    if it.left > 0 {
        it.left -= 1;
    } else {
        it.catch_all = 0;
    }
    it.pos = c.pos as u32;
    E_OK
}
