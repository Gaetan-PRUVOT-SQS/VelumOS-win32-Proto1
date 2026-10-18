use crate::check::check;
use crate::decode::{blank_insn, decode_at};
use crate::payload::{keys_ok, payload_at, switch_at};
use crate::read::{rd32_at, unit_at};
use crate::tab::{
    DO_BRANCH, DO_CONT, DO_FILL, DO_NOTSUP, DO_SWITCH, DP_ARRAY, DP_PACKED, OP_FILL_ARRAY_DATA,
    OP_GOTO_32, OP_NOP, OP_PACKED_SWITCH, op_info,
};
use crate::{E_INVAL, E_OK, t_dcodelimits};

fn bit(starts: &[u8], pc: u32) -> i32 {
    let Some(b) = starts.get(usize::try_from(pc >> 3).unwrap_or(usize::MAX)) else {
        return 0;
    };
    i32::from((b >> (pc & 7)) & 1)
}

fn bit_set(starts: &mut [u8], pc: u32) -> Result<(), i32> {
    let i = usize::try_from(pc >> 3).map_err(|_| E_INVAL)?;
    let b = starts.get_mut(i).ok_or(E_INVAL)?;
    *b |= 1u8 << (pc & 7);
    Ok(())
}

fn bit_clear(starts: &mut [u8], pc: u32) {
    let Ok(i) = usize::try_from(pc >> 3) else {
        return;
    };
    let Some(b) = starts.get_mut(i) else {
        return;
    };
    *b &= !(1u8 << (pc & 7));
}

pub(crate) fn is_payload(code: &[u8], pc: u32) -> bool {
    match unit_at(code, pc) {
        Some(u) => u & 0xff == 0 && u >> 8 != 0,
        None => false,
    }
}

pub(crate) fn item_len(code: &[u8], pc: u32) -> u32 {
    if is_payload(code, pc) {
        return match payload_at(code, pc) {
            Ok(pl) => u32::try_from(pl.units).unwrap_or(0),
            Err(_) => 0,
        };
    }
    let mut insn = blank_insn();
    if decode_at(code, pc, &mut insn) < 0 {
        return 0;
    }
    u32::from(insn.len)
}

fn target_ok(code: &[u8], starts: &[u8], n: u32, t: i64) -> bool {
    if t < 0 || t >= i64::from(n) {
        return false;
    }
    let Ok(tu) = u32::try_from(t) else {
        return false;
    };
    if bit(starts, tu) == 0 || is_payload(code, tu) {
        return false;
    }
    let Some(next) = tu.checked_add(1) else {
        return true;
    };
    if i64::from(next) < i64::from(n) && unit_at(code, tu) == Some(0) && is_payload(code, next) {
        return false;
    }
    true
}

fn data_ok(code: &[u8], starts: &[u8], n: u32, pc: u32, lit: i64, op: u8) -> i32 {
    let Some(t) = i64::from(pc).checked_add(lit) else {
        return E_INVAL;
    };
    if t < 0 || t >= i64::from(n) {
        return E_INVAL;
    }
    let Ok(tu) = u32::try_from(t) else {
        return E_INVAL;
    };
    if bit(starts, tu) == 0 {
        return E_INVAL;
    }
    let Ok(pl) = payload_at(code, tu) else {
        return E_INVAL;
    };
    let fill = op == OP_FILL_ARRAY_DATA;
    let packed = op == OP_PACKED_SWITCH;
    if fill != (pl.ident == DP_ARRAY) || packed != (pl.ident == DP_PACKED) {
        return E_INVAL;
    }
    if pl.ident == DP_ARRAY {
        return E_OK;
    }
    let Ok(sw) = switch_at(code, tu) else {
        return E_OK;
    };
    let mut i = 0u32;
    while i < sw.count {
        let Some(step) = usize::try_from(i).ok().and_then(|n| n.checked_mul(4)) else {
            return E_INVAL;
        };
        let Some(off) = sw.targets_off.checked_add(step) else {
            return E_INVAL;
        };
        let Some(rel) = rd32_at(code, off) else {
            return E_INVAL;
        };
        let Some(dest) = i64::from(pc).checked_add(i64::from(rel)) else {
            return E_INVAL;
        };
        if !target_ok(code, starts, n, dest) {
            return E_INVAL;
        }
        i += 1;
    }
    E_OK
}

fn insn_targets(code: &[u8], starts: &[u8], n: u32, pc: u32) -> i32 {
    let mut insn = blank_insn();
    if decode_at(code, pc, &mut insn) < 0 {
        return E_INVAL;
    }
    let fl = op_info(insn.op).flags;
    if fl & DO_BRANCH != 0 && insn.lit == 0 && insn.op != OP_GOTO_32 {
        return E_INVAL;
    }
    if fl & DO_BRANCH != 0 {
        let Some(t) = i64::from(pc).checked_add(insn.lit) else {
            return E_INVAL;
        };
        if !target_ok(code, starts, n, t) {
            return E_INVAL;
        }
    }
    if fl & (DO_SWITCH | DO_FILL) != 0 {
        return data_ok(code, starts, n, pc, insn.lit, insn.op);
    }
    E_OK
}

fn targets(code: &[u8], starts: &[u8], n: u32) -> i32 {
    let mut pc = 0u32;
    while pc < n {
        let len = item_len(code, pc);
        if len == 0 {
            return E_INVAL;
        }
        if !is_payload(code, pc) && insn_targets(code, starts, n, pc) < 0 {
            return E_INVAL;
        }
        let Some(next) = pc.checked_add(len) else {
            return E_INVAL;
        };
        pc = next;
    }
    E_OK
}

fn strip(code: &[u8], starts: &mut [u8], n: u32) {
    let mut pc = 0u32;
    let mut len = 1u32;
    while pc < n && len != 0 {
        len = item_len(code, pc);
        if is_payload(code, pc) {
            bit_clear(starts, pc);
            if pc > 0 {
                let prev = pc - 1;
                if unit_at(code, prev) == Some(0) && bit(starts, prev) != 0 {
                    bit_clear(starts, prev);
                }
            }
        }
        let Some(next) = pc.checked_add(len) else {
            break;
        };
        pc = next;
    }
}

fn scan(code: &[u8], lim: &t_dcodelimits, starts: &mut [u8], n: u32) -> i32 {
    let mut pc = 0u32;
    let mut live = 1i32;
    while pc < n {
        if bit_set(starts, pc).is_err() {
            return E_INVAL;
        }
        let r = if is_payload(code, pc) {
            scan_payload(code, &mut pc, &mut live)
        } else {
            scan_insn(code, lim, &mut pc, &mut live)
        };
        if r < 0 {
            return r;
        }
    }
    if live != 0 {
        return E_INVAL;
    }
    E_OK
}

fn scan_payload(code: &[u8], pc: &mut u32, live: &mut i32) -> i32 {
    if *live == 1 {
        return E_INVAL;
    }
    let pl = match payload_at(code, *pc) {
        Ok(pl) => pl,
        Err(e) => return e,
    };
    if !keys_ok(code, *pc, &pl) {
        return E_INVAL;
    }
    let Ok(add) = u32::try_from(pl.units) else {
        return E_INVAL;
    };
    if add == 0 {
        return E_INVAL;
    }
    let Some(next) = pc.checked_add(add) else {
        return E_INVAL;
    };
    *pc = next;
    *live = 0;
    E_OK
}

fn scan_insn(code: &[u8], lim: &t_dcodelimits, pc: &mut u32, live: &mut i32) -> i32 {
    let mut insn = blank_insn();
    let r = decode_at(code, *pc, &mut insn);
    if r < 0 {
        return r;
    }
    let fl = op_info(insn.op).flags;
    if fl & DO_NOTSUP != 0 {
        return crate::E_NOTSUP;
    }
    let r = check(&insn, lim);
    if r < 0 {
        return r;
    }
    if insn.op == OP_NOP && *live == 0 {
        *live = 2;
    } else {
        *live = i32::from(fl & DO_CONT != 0);
    }
    let Some(next) = pc.checked_add(u32::from(insn.len)) else {
        return E_INVAL;
    };
    *pc = next;
    E_OK
}

pub(crate) fn verify_code(code: &[u8], lim: &t_dcodelimits, starts: &mut [u8]) -> i32 {
    starts.fill(0);
    let Ok(n) = u32::try_from(code.len() / 2) else {
        return E_INVAL;
    };
    let r = scan(code, lim, starts, n);
    if r != E_OK {
        return r;
    }
    let r = targets(code, starts, n);
    if r != E_OK {
        return r;
    }
    strip(code, starts, n);
    E_OK
}
