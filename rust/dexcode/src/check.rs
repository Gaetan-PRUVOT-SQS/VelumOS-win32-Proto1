use crate::tab::{
    DK_FIELD, DK_METHOD, DK_NONE, DK_PROTO, DK_STRING, DK_TYPE, DO_INVOKE, DO_WA, DO_WB, DO_WC,
    DO_WDST, DR_A, DR_B, DR_C, DR_LIST, DR_RANGE, fmt_info, op_info,
};
use crate::{E_OK, E_RANGE, t_dcodelimits, t_dinsn};

fn reg_ok(r: u32, wide: bool, nreg: u32) -> bool {
    u64::from(r) + u64::from(wide) < u64::from(nreg)
}

fn regs_ok(insn: &t_dinsn, flags: u16, nreg: u32) -> bool {
    let mask = fmt_info(insn.fmt).regs;
    if mask & DR_A != 0 && !reg_ok(insn.a, flags & (DO_WDST | DO_WA) != 0, nreg) {
        return false;
    }
    if mask & DR_B != 0 && !reg_ok(insn.b, flags & DO_WB != 0, nreg) {
        return false;
    }
    if mask & DR_C != 0 && !reg_ok(insn.c, flags & DO_WC != 0, nreg) {
        return false;
    }
    if mask & DR_RANGE != 0 {
        if insn.argc == 0 {
            return true;
        }
        return u64::from(insn.c) + u64::from(insn.argc) <= u64::from(nreg);
    }
    let mut i = 0u8;
    while mask & DR_LIST != 0 && i < insn.argc {
        let Some(arg) = insn.args.get(usize::from(i)) else {
            return false;
        };
        if u32::from(*arg) >= nreg {
            return false;
        }
        i += 1;
    }
    true
}

fn index_ok(kind: u8, idx: u32, lim: &t_dcodelimits) -> bool {
    match kind {
        DK_STRING => idx < lim.strings,
        DK_TYPE => idx < lim.types,
        DK_FIELD => idx < lim.fields,
        DK_METHOD => idx < lim.methods,
        DK_PROTO => idx < lim.protos,
        DK_NONE => true,
        _ => false,
    }
}

pub(crate) fn check(insn: &t_dinsn, lim: &t_dcodelimits) -> i32 {
    let op = op_info(insn.op);
    if !regs_ok(insn, op.flags, lim.registers) {
        return E_RANGE;
    }
    if !index_ok(op.kind, insn.idx, lim) {
        return E_RANGE;
    }
    if op.flags & DO_INVOKE != 0 && u32::from(insn.argc) > lim.outs {
        return E_RANGE;
    }
    E_OK
}
