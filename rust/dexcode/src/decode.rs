use crate::read::{field, literal, low_u8, low_u16, unit, unit_at};
use crate::tab::{DF_21H, DR_LIST, DS_A4, OP_CONST_WIDE_HIGH16, fmt_info, op_info};
use crate::{E_INVAL, E_OK, t_dinsn};

pub(crate) fn blank_insn() -> t_dinsn {
    t_dinsn {
        op: 0,
        fmt: 0,
        len: 0,
        argc: 0,
        a: 0,
        b: 0,
        c: 0,
        lit: 0,
        idx: 0,
        idx2: 0,
        args: [0; 5],
    }
}

fn shl_lit(v: i64, shift: u32) -> i64 {
    let bits = u64::from_ne_bytes(v.to_ne_bytes());
    i64::from_ne_bytes((bits << shift).to_ne_bytes())
}

fn insn_slice(code: &[u8], pc: u32, len: u8) -> Option<&[u8]> {
    let off = usize::try_from(u64::from(pc).checked_mul(2)?).ok()?;
    let n = usize::from(len).checked_mul(2)?;
    code.get(off..off.checked_add(n)?)
}

fn fill_args(code: &[u8], out: &mut t_dinsn) -> Option<()> {
    let w2 = unit(code, 2)?;
    out.args[0] = low_u16(w2 & 15);
    out.args[1] = low_u16((w2 >> 4) & 15);
    out.args[2] = low_u16((w2 >> 8) & 15);
    out.args[3] = low_u16(w2 >> 12);
    out.args[4] = low_u16(field(code, DS_A4)?);
    Some(())
}

fn fill(code: &[u8], f_len: u8, out: &mut t_dinsn) -> Result<(), ()> {
    let info = fmt_info(out.fmt);
    out.len = f_len;
    out.a = field(code, info.a).ok_or(())?;
    out.b = field(code, info.b).ok_or(())?;
    out.c = field(code, info.c).ok_or(())?;
    out.lit = literal(code, info.lit).ok_or(())?;
    out.idx = field(code, info.idx).ok_or(())?;
    out.argc = low_u8(field(code, info.argc).ok_or(())?);
    out.idx2 = 0;
    if info.len == 4 {
        out.idx2 = unit(code, 3).ok_or(())?;
    }
    if info.regs & DR_LIST != 0 {
        fill_args(code, out).ok_or(())?;
    }
    if out.fmt == DF_21H && out.op == OP_CONST_WIDE_HIGH16 {
        out.lit = shl_lit(out.lit, 48);
    } else if out.fmt == DF_21H {
        out.lit = shl_lit(out.lit, 16);
    }
    Ok(())
}

pub(crate) fn decode_at(code: &[u8], pc: u32, out: &mut t_dinsn) -> i32 {
    let units = code.len() / 2;
    if u64::from(pc) >= units as u64 {
        return E_INVAL;
    }
    let Some(w0) = unit_at(code, pc) else {
        return E_INVAL;
    };
    let op = low_u8(w0);
    out.op = op;
    out.fmt = op_info(op).fmt;
    let info = fmt_info(out.fmt);
    let end = u64::from(pc) + u64::from(info.len);
    if info.len == 0 || end > units as u64 {
        return E_INVAL;
    }
    out.args = [0; 5];
    let Some(slice) = insn_slice(code, pc, info.len) else {
        return E_INVAL;
    };
    if fill(slice, info.len, out).is_err() {
        return E_INVAL;
    }
    if info.regs & DR_LIST != 0 && out.argc > 5 {
        return E_INVAL;
    }
    E_OK
}
