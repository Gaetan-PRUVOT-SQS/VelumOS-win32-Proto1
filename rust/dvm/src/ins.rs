use core::ffi::{c_char, c_int};

use crate::bridge::{
    go_array, go_array_length, go_array_view, go_catches, go_const_class, go_const_string,
    go_field, go_fill_array, go_instance_of, go_new_array, go_new_instance, go_resolve, go_throw,
};
use crate::{
    ACC_ABSTRACT, ACC_NATIVE, DAK_BOOLEAN, DAK_BYTE, DAK_CHAR, DAK_OBJECT, DAK_SHORT, DAK_WIDE,
    DEX_NO_INDEX, DF_3RC, DIK_STATIC, DVM_NULL, DVM_THROWN, E_INVAL, E_TIMEOUT, IN_ABSTRACT,
    IN_ARITH, IN_CAST, IN_HDR, IN_INVOKE_LEN, IN_NPE, IN_SOE, IN_VERIFY, OP_ADD_DOUBLE,
    OP_ADD_DOUBLE_2ADDR, OP_ADD_FLOAT, OP_ADD_FLOAT_2ADDR, OP_ADD_INT, OP_ADD_INT_2ADDR,
    OP_ADD_INT_LIT8, OP_ADD_INT_LIT16, OP_ADD_LONG, OP_ADD_LONG_2ADDR, OP_AGET, OP_APUT,
    OP_CMP_LONG, OP_CMPG_DOUBLE, OP_CMPG_FLOAT, OP_CMPL_DOUBLE, OP_CONST_CLASS, OP_CONST_WIDE_16,
    OP_DOUBLE_TO_INT, OP_DOUBLE_TO_LONG, OP_FLOAT_TO_INT, OP_FLOAT_TO_LONG, OP_IF_EQ, OP_IF_EQZ,
    OP_IF_LE, OP_IGET, OP_INT_TO_BYTE, OP_INT_TO_CHAR, OP_INT_TO_DOUBLE, OP_INT_TO_FLOAT,
    OP_INT_TO_LONG, OP_INVOKE_VIRTUAL, OP_LONG_TO_FLOAT, OP_LONG_TO_INT, OP_MOVE_RESULT_WIDE,
    OP_MOVE_WIDE, OP_MOVE_WIDE_16, OP_NEG_DOUBLE, OP_NEG_FLOAT, OP_NEG_INT, OP_NEG_LONG,
    OP_NEW_INSTANCE, OP_NOT_INT, OP_NOT_LONG, OP_RETURN_VOID, OP_RETURN_WIDE, OP_THROW, lit_desc,
    t_daacc, t_darray, t_darrview, t_dexcatch, t_dexcode, t_dexhit, t_dinsn, t_dinvoke, t_dmethod,
    t_dnewarr, t_dref, t_dswitch, t_dvm, t_in, t_inop,
};

macro_rules! dex {
    ($name:ident($($a:expr),* $(,)?)) => {{
        #[allow(unused_unsafe)]
        // SAFETY: symbole C dex ou dexcode, même signature que le contrat, non défini ici.
        unsafe {
            #[cfg(not(test))]
            {
                crate::ext::$name($($a),*)
            }
            #[cfg(test)]
            {
                crate::dexstub::$name($($a),*)
            }
        }
    }};
}

macro_rules! u {
    ($($t:tt)*) => {{
        // SAFETY: préconditions du contrat C tenues par l'appelant de cette opération.
        unsafe { $($t)* }
    }};
}

fn desc(s: &[u8]) -> *const c_char {
    lit_desc(s)
}

fn zero_insn() -> t_dinsn {
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

fn f32b(v: f32) -> u32 {
    v.to_bits()
}

fn f64b(v: f64) -> u64 {
    v.to_bits()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_get(inn: *mut t_in, n: u32) -> u32 {
    u! {
        if n >= (*inn).nreg {
            (*inn).bad = 1;
            return 0;
        }
        *(*inn).r.add(n as usize)
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_set(inn: *mut t_in, n: u32, v: u32) {
    u! {
        if n >= (*inn).nreg {
            (*inn).bad = 1;
        } else {
            *(*inn).r.add(n as usize) = v;
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_getw(inn: *mut t_in, n: u32) -> u64 {
    u! {
        if n >= (*inn).nreg || (*inn).nreg - n < 2 {
            (*inn).bad = 1;
            return 0;
        }
        let lo = u64::from(*(*inn).r.add(n as usize));
        let hi = u64::from(*(*inn).r.add(n as usize + 1));
        lo | (hi << 32)
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_setw(inn: *mut t_in, n: u32, v: u64) {
    u! {
        if n >= (*inn).nreg || (*inn).nreg - n < 2 {
            (*inn).bad = 1;
            return;
        }
        *(*inn).r.add(n as usize) = v as u32;
        *(*inn).r.add(n as usize + 1) = (v >> 32) as u32;
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_throw(inn: *mut t_in, msg: *const c_char) -> c_int {
    let rc = u!(go_throw((*inn).vm, msg, core::ptr::null()));
    if rc == 0 { DVM_THROWN } else { rc }
}

fn in_load(inn: *mut t_in, fp: u32) {
    u! {
        let h = (*(*inn).vm).stack.add(fp as usize);
        (*inn).m = core::ptr::read_unaligned(h.cast::<*const t_dmethod>());
        (*inn).fp = fp;
        (*inn).r = h.add(IN_HDR as usize);
        (*inn).nreg = u32::from((*(*inn).m).registers);
    }
}

fn in_fill(h: *mut u32, m: *const t_dmethod, args: *const u32) {
    u! {
        core::ptr::write_unaligned(h.cast::<*const t_dmethod>(), m);
        *h.add(2) = 0;
        let first = u32::from((*m).registers) - u32::from((*m).ins);
        let mut k = 0u32;
        while k < u32::from((*m).registers) {
            let mut v = 0u32;
            if k >= first {
                v = *args.add((k - first) as usize);
            }
            *h.add(IN_HDR as usize + k as usize) = v;
            k += 1;
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_push(inn: *mut t_in, m: *const t_dmethod, args: *const u32) -> c_int {
    let (vm, need) = u! {
        let vm = (*inn).vm;
        let need = IN_HDR + u32::from((*m).registers);
        if (*m).ins > (*m).registers || ((*m).ins != 0 && args.is_null()) {
            return in_throw(inn, desc(IN_VERIFY));
        }
        if (*vm).depth >= (*vm).lim.depth_max
            || (*vm).sp > (*vm).stack_words
            || need > (*vm).stack_words - (*vm).sp
        {
            return in_throw(inn, desc(IN_SOE));
        }
        (vm, need)
    };
    let fp = u!((*vm).sp);
    u! {
        if !(*inn).m.is_null() {
            *(*vm).stack.add((*inn).fp as usize + 2) = (*inn).pc;
        }
        in_fill((*vm).stack.add(fp as usize), m, args);
        *(*vm).stack.add(fp as usize + 3) = (*inn).fp;
        (*vm).sp = (*vm).sp.wrapping_add(need);
        (*vm).depth = (*vm).depth.wrapping_add(1);
    }
    in_load(inn, fp);
    u! {
        (*inn).pc = 0;
        (*inn).next = 0;
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_pop(inn: *mut t_in) {
    let prev = u!(*(*(*inn).vm).stack.add((*inn).fp as usize + 3));
    u! {
        (*(*inn).vm).sp = (*inn).fp;
        (*(*inn).vm).depth = (*(*inn).vm).depth.wrapping_sub(1);
        if (*inn).fp == (*inn).fp0 {
            (*inn).done = 1;
            return;
        }
    }
    in_load(inn, prev);
    u! {
        (*inn).pc = *(*(*inn).vm).stack.add(prev as usize + 2);
        (*inn).next = (*inn).pc.wrapping_add(IN_INVOKE_LEN);
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_native(inn: *mut t_in, m: *const t_dmethod, args: *const u32) -> c_int {
    u! {
        if (*(*inn).vm).depth >= (*(*inn).vm).lim.depth_max {
            return in_throw(inn, desc(IN_SOE));
        }
        let Some(f) = (*m).native else {
            return E_INVAL;
        };
        let mut ret = 0u64;
        (*(*inn).vm).depth = (*(*inn).vm).depth.wrapping_add(1);
        let rc = f((*inn).vm, args, &mut ret);
        (*(*inn).vm).depth = (*(*inn).vm).depth.wrapping_sub(1);
        if rc == 0 {
            (*(*inn).vm).result = ret;
        }
        rc
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_enter(inn: *mut t_in, m: *const t_dmethod, args: *const u32) -> c_int {
    u! {
        if (*m).native.is_some() {
            return in_native(inn, m, args);
        }
        if (*m).insns.p.is_null()
            || (*m).insns.len < 2
            || ((*m).access & (ACC_ABSTRACT | ACC_NATIVE)) != 0
        {
            return in_throw(inn, desc(IN_ABSTRACT));
        }
    }
    u!(in_push(inn, m, args))
}

fn in_match(inn: *mut t_in, c: *const t_dexcatch) -> c_int {
    u! {
        if (*c).type_idx == DEX_NO_INDEX {
            return 1;
        }
        let before = (*(*inn).vm).pending;
        let rc = go_catches((*inn).vm, (*inn).m, (*c).type_idx, before);
        if rc > 0 && (*(*inn).vm).pending != before {
            return 0;
        }
        rc
    }
}

fn in_handler(inn: *mut t_in, addr: *mut u32) -> c_int {
    u! {
        if (*(*inn).m).tries == 0 || (*(*inn).m).dex.is_null() {
            return 0;
        }
        let mut code = t_dexcode {
            insns: core::ptr::null(),
            registers: 0,
            ins: 0,
            outs: 0,
            tries: 0,
            insns_size: 0,
            off: 0,
            tries_off: 0,
            handlers_off: 0,
        };
        if dex!(dex_code((*(*inn).m).dex, (*(*inn).m).code_off, &mut code)) < 0 {
            return 0;
        }
        let rc = dex!(dex_try_find((*(*inn).m).dex, &code, (*inn).pc));
        let mut it = t_dexhit {
            d: core::ptr::null(),
            pos: 0,
            left: 0,
            catch_all: 0,
            insns_size: 0,
        };
        if rc < 0 || dex!(dex_handler_open((*(*inn).m).dex, &code, rc as u32, &mut it)) < 0 {
            return 0;
        }
        let mut out = 0;
        let mut ca = t_dexcatch { type_idx: 0, addr: 0 };
        while out == 0 && dex!(dex_handler_next(&mut it, &mut ca)) == 0 {
            out = in_match(inn, &ca);
            *addr = ca.addr;
        }
        out
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_unwind(inn: *mut t_in) -> c_int {
    let mut addr = 0u32;
    let mut rc = in_handler(inn, &mut addr);
    while rc == 0 && u!((*inn).done) == 0 {
        u!(in_pop(inn));
        if u!((*inn).done) == 0 {
            rc = in_handler(inn, &mut addr);
        }
    }
    if rc < 0 {
        return rc;
    }
    if u!((*inn).done) != 0 {
        return DVM_THROWN;
    }
    u! { (*inn).pc = addr; }
    0
}

const fn op_slot(op: u8) -> t_inop {
    match op {
        0x00 => in_op_nop,
        0x01..=0x09 => in_op_move,
        0x0a..=0x0c => in_op_move_result,
        0x0d => in_op_move_exception,
        0x0e..=0x11 => in_op_return,
        0x12..=0x19 => in_op_const,
        0x1a..=0x1c | 0x22 => in_op_const_ref,
        0x1d | 0x1e | 0x27 => in_op_monitor,
        0x1f => in_op_check_cast,
        0x20 => in_op_instance_of,
        0x21 => in_op_array_length,
        0x23 => in_op_new_array,
        0x24 | 0x25 => in_op_filled,
        0x26 => in_op_fill_data,
        0x28..=0x2a => in_op_goto,
        0x2b | 0x2c => in_op_switch,
        0x2d..=0x31 => in_op_cmp,
        0x32..=0x3d => in_op_if,
        0x44..=0x4a => in_op_aget,
        0x4b..=0x51 => in_op_aput,
        0x52..=0x6d => in_op_field,
        0x6e..=0x72 | 0x74..=0x78 => in_op_invoke,
        0x7b | 0x7c | 0x81..=0x83 | 0x8d..=0x8f => in_op_un_int,
        0x7d | 0x7e | 0x84..=0x86 => in_op_un_long,
        0x7f | 0x87..=0x89 => in_op_un_float,
        0x80 | 0x8a..=0x8c => in_op_un_double,
        0x90..=0x9a | 0xb0..=0xba => in_op_ibin,
        0x9b..=0xa5 | 0xbb..=0xc5 => in_op_lbin,
        0xa6..=0xaa | 0xc6..=0xca => in_op_fbin,
        0xab..=0xaf | 0xcb..=0xcf => in_op_dbin,
        0xd0..=0xe2 => in_op_ilit,
        _ => in_op_bad,
    }
}

const fn op_table() -> [t_inop; 256] {
    let mut t = [in_op_bad as t_inop; 256];
    let mut i = 0u16;
    while i < 256 {
        t[i as usize] = op_slot(i as u8);
        i += 1;
    }
    t
}

static G_IN_OPS: [t_inop; 256] = op_table();

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op(op: u8) -> t_inop {
    G_IN_OPS[op as usize]
}

fn in_step(inn: *mut t_in) -> c_int {
    u! {
        let vm = (*inn).vm;
        if (*vm).lim.budget != 0 && (*vm).spent >= (*vm).lim.budget {
            return E_TIMEOUT;
        }
        (*vm).spent = (*vm).spent.wrapping_add(1);
        if dex!(dexcode_decode((*(*inn).m).insns, (*inn).pc, &mut (*inn).i)) < 0 {
            return E_INVAL;
        }
        (*inn).next = (*inn).pc.wrapping_add(u32::from((*inn).i.len));
        let rc0 = in_op((*inn).i.op)(inn);
        let mut rc = rc0;
        if rc == 0 && (*inn).bad != 0 {
            rc = in_throw(inn, desc(IN_VERIFY));
        }
        (*inn).bad = 0;
        if rc == 0 {
            (*inn).pc = (*inn).next;
        }
        rc
    }
}

fn in_run(inn: *mut t_in) -> c_int {
    let mut rc = 0;
    while rc == 0 && u!((*inn).done) == 0 {
        rc = in_step(inn);
        if rc > 0 {
            rc = u!(in_unwind(inn));
        }
    }
    rc
}

pub(crate) unsafe fn dvm_call_inner(
    vm: *mut t_dvm,
    m: *const t_dmethod,
    args: *const u32,
    ret: *mut u64,
) -> c_int {
    if vm.is_null() || m.is_null() || u!((*vm).stack).is_null() {
        return E_INVAL;
    }
    let mut inn = t_in {
        vm,
        m: core::ptr::null(),
        r: core::ptr::null_mut(),
        nreg: 0,
        pc: 0,
        next: 0,
        fp: 0,
        fp0: 0,
        done: 0,
        bad: 0,
        i: zero_insn(),
        argv: [0; 5],
    };
    let (mark_sp, mark_depth) = u!(((*vm).sp, (*vm).depth));
    let mut rc = u!(in_enter(&mut inn, m, args));
    inn.fp0 = inn.fp;
    if rc == 0 && !inn.m.is_null() {
        rc = in_run(&mut inn);
    }
    u! {
        (*vm).sp = mark_sp;
        (*vm).depth = mark_depth;
        if rc == 0 && !ret.is_null() {
            *ret = (*vm).result;
        }
    }
    rc
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_gather(inn: *mut t_in, n: *mut u32) -> *const u32 {
    u! {
        *n = u32::from((*inn).i.argc);
        if (*inn).i.fmt == DF_3RC {
            if (*inn).i.c > (*inn).nreg || *n > (*inn).nreg - (*inn).i.c {
                (*inn).bad = 1;
            }
            if (*inn).bad != 0 {
                return (*inn).argv.as_ptr();
            }
            return (*inn).r.add((*inn).i.c as usize);
        }
        let mut k = 0u32;
        while k < *n && k < 5 {
            (*inn).argv[k as usize] = in_get(inn, u32::from((*inn).i.args[k as usize]));
            k += 1;
        }
        (*inn).argv.as_ptr()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_narrow(kind: u32, v: u64) -> u64 {
    if kind == DAK_WIDE {
        return v;
    }
    if kind == DAK_BOOLEAN {
        return u64::from(v as u8);
    }
    if kind == DAK_BYTE {
        return u64::from(v as u8 as i8 as i32 as u32);
    }
    if kind == DAK_CHAR {
        return u64::from(v as u16);
    }
    if kind == DAK_SHORT {
        return u64::from(v as u16 as i16 as i32 as u32);
    }
    u64::from(v as u32)
}

fn in_norm(bits: u64, e: &mut i32) -> u64 {
    *e = ((bits >> 52) & 0x7ff) as i32;
    let mut m = bits & 0x0_000f_ffff_ffff_ffff;
    if *e != 0 {
        return m | (1u64 << 52);
    }
    *e = 1;
    while m != 0 && m < (1u64 << 52) {
        m <<= 1;
        *e -= 1;
    }
    m
}

fn in_reduce(mut mx: u64, my: u64, mut n: i32) -> u64 {
    while n > 0 {
        if mx >= my {
            mx -= my;
        }
        mx <<= 1;
        n -= 1;
    }
    if mx >= my {
        mx -= my;
    }
    mx
}

fn in_pack(mut m: u64, mut e: i32, sign: u64) -> u64 {
    if m == 0 {
        return sign;
    }
    while m < (1u64 << 52) {
        m <<= 1;
        e -= 1;
    }
    if e > 0 {
        return sign | ((e as u64) << 52) | (m - (1u64 << 52));
    }
    if e < -62 {
        return sign;
    }
    sign | (m >> (1 - e) as u32)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_fmod(x: f64, y: f64) -> f64 {
    let b0 = f64b(x);
    let b1 = f64b(y);
    if b1.wrapping_shl(1) == 0
        || b0.wrapping_shl(1) >= 0x7ffu64 << 53
        || b1.wrapping_shl(1) > 0x7ffu64 << 53
    {
        return f64::from_bits(0x7ff8_0000_0000_0000);
    }
    if b0.wrapping_shl(1) < b1.wrapping_shl(1) {
        return x;
    }
    let mut e0 = 0i32;
    let mut e1 = 0i32;
    let m0 = in_norm(b0, &mut e0);
    let m1 = in_norm(b1, &mut e1);
    let m0 = in_reduce(m0, m1, e0 - e1);
    f64::from_bits(in_pack(m0, e1, b0 & (1u64 << 63)))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_d2i(d: f64) -> i32 {
    if d.is_nan() {
        return 0;
    }
    if d >= 2_147_483_647.0 {
        return i32::MAX;
    }
    if d <= -2_147_483_648.0 {
        return i32::MIN;
    }
    d as i32
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_d2l(d: f64) -> i64 {
    if d.is_nan() {
        return 0;
    }
    if d >= 9_223_372_036_854_775_808.0 {
        return i64::MAX;
    }
    if d <= -9_223_372_036_854_775_808.0 {
        return i64::MIN;
    }
    d as i64
}

fn in_isimple(k: u32, x: u32, y: u32) -> u32 {
    if k == 0 {
        return x.wrapping_add(y);
    }
    if k == 1 {
        return x.wrapping_sub(y);
    }
    if k == 2 {
        return x.wrapping_mul(y);
    }
    if k == 5 {
        return x & y;
    }
    if k == 6 {
        return x | y;
    }
    if k == 7 {
        return x ^ y;
    }
    if k == 8 {
        return x.wrapping_shl(y & 31);
    }
    if k == 9 {
        return (x as i32).wrapping_shr(y & 31) as u32;
    }
    x.wrapping_shr(y & 31)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_iop(inn: *mut t_in, k: u32, x: *mut u32, y: u32) -> c_int {
    u! {
        let a = *x as i32;
        if k != 3 && k != 4 {
            *x = in_isimple(k, *x, y);
            return 0;
        }
        if y == 0 {
            return in_throw(inn, desc(IN_ARITH));
        }
        if y == u32::MAX && k == 3 {
            *x = 0u32.wrapping_sub(*x);
        } else if y == u32::MAX {
            *x = 0;
        } else if k == 3 {
            *x = (a / (y as i32)) as u32;
        } else {
            *x = (a % (y as i32)) as u32;
        }
        0
    }
}

fn in_lsimple(k: u32, x: u64, y: u64) -> u64 {
    if k == 0 {
        return x.wrapping_add(y);
    }
    if k == 1 {
        return x.wrapping_sub(y);
    }
    if k == 2 {
        return x.wrapping_mul(y);
    }
    if k == 5 {
        return x & y;
    }
    if k == 6 {
        return x | y;
    }
    if k == 7 {
        return x ^ y;
    }
    if k == 8 {
        return x.wrapping_shl((y & 63) as u32);
    }
    if k == 9 {
        return (x as i64).wrapping_shr((y & 63) as u32) as u64;
    }
    x.wrapping_shr((y & 63) as u32)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_lop(inn: *mut t_in, k: u32, x: *mut u64, y: u64) -> c_int {
    u! {
        let a = *x as i64;
        if k != 3 && k != 4 {
            *x = in_lsimple(k, *x, y);
            return 0;
        }
        if y == 0 {
            return in_throw(inn, desc(IN_ARITH));
        }
        if y == u64::MAX && k == 3 {
            *x = 0u64.wrapping_sub(*x);
        } else if y == u64::MAX {
            *x = 0;
        } else if k == 3 {
            *x = (a / (y as i64)) as u64;
        } else {
            *x = (a % (y as i64)) as u64;
        }
        0
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_nop(_inn: *mut t_in) -> c_int {
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_bad(inn: *mut t_in) -> c_int {
    u!(in_throw(inn, desc(IN_VERIFY)))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_move(inn: *mut t_in) -> c_int {
    u! {
        if (*inn).i.op >= OP_MOVE_WIDE && (*inn).i.op <= OP_MOVE_WIDE_16 {
            let v = in_getw(inn, (*inn).i.b);
            in_setw(inn, (*inn).i.a, v);
        } else {
            let v = in_get(inn, (*inn).i.b);
            in_set(inn, (*inn).i.a, v);
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_move_result(inn: *mut t_in) -> c_int {
    u! {
        if (*inn).i.op == OP_MOVE_RESULT_WIDE {
            in_setw(inn, (*inn).i.a, (*(*inn).vm).result);
        } else {
            in_set(inn, (*inn).i.a, (*(*inn).vm).result as u32);
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_move_exception(inn: *mut t_in) -> c_int {
    u! {
        in_set(inn, (*inn).i.a, (*(*inn).vm).pending);
        (*(*inn).vm).pending = DVM_NULL;
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_return(inn: *mut t_in) -> c_int {
    u! {
        let mut v = 0u64;
        if (*inn).i.op == OP_RETURN_WIDE {
            v = in_getw(inn, (*inn).i.a);
        } else if (*inn).i.op != OP_RETURN_VOID {
            v = u64::from(in_get(inn, (*inn).i.a));
        }
        if (*inn).bad != 0 {
            return 0;
        }
        (*(*inn).vm).result = v;
    }
    u!(in_pop(inn));
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_const(inn: *mut t_in) -> c_int {
    u! {
        if (*inn).i.op >= OP_CONST_WIDE_16 {
            in_setw(inn, (*inn).i.a, (*inn).i.lit as u64);
        } else {
            in_set(inn, (*inn).i.a, (*inn).i.lit as u32);
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_const_ref(inn: *mut t_in) -> c_int {
    let mut rref = DVM_NULL;
    let rc = u! {
        if (*inn).i.op == OP_CONST_CLASS {
            go_const_class((*inn).vm, (*inn).m, (*inn).i.idx, &mut rref)
        } else if (*inn).i.op == OP_NEW_INSTANCE {
            go_new_instance((*inn).vm, (*inn).m, (*inn).i.idx, &mut rref)
        } else {
            go_const_string((*inn).vm, (*inn).m, (*inn).i.idx, &mut rref)
        }
    };
    if rc == 0 {
        u!(in_set(inn, (*inn).i.a, rref));
    }
    rc
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_monitor(inn: *mut t_in) -> c_int {
    let rref = u!(in_get(inn, (*inn).i.a));
    if u!((*inn).bad) != 0 {
        return 0;
    }
    if rref == DVM_NULL {
        return u!(in_throw(inn, desc(IN_NPE)));
    }
    if u!((*inn).i.op) != OP_THROW {
        return 0;
    }
    u! { (*(*inn).vm).pending = rref; }
    DVM_THROWN
}

fn in_isinst(inn: *mut t_in, rref: t_dref, yes: *mut u32) -> c_int {
    u! {
        let before = (*(*inn).vm).pending;
        let rc = go_instance_of((*inn).vm, (*inn).m, (*inn).i.idx, rref);
        *yes = u32::from(rc > 0);
        if rc > 0 && (*(*inn).vm).pending != before && (*(*inn).vm).pending != DVM_NULL {
            return DVM_THROWN;
        }
        if rc > 0 {
            return 0;
        }
        rc
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_check_cast(inn: *mut t_in) -> c_int {
    let rref = u!(in_get(inn, (*inn).i.a));
    if rref == DVM_NULL {
        return 0;
    }
    let mut yes = 0u32;
    let mut rc = in_isinst(inn, rref, &mut yes);
    if rc == 0 && yes == 0 {
        rc = u!(in_throw(inn, desc(IN_CAST)));
    }
    rc
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_instance_of(inn: *mut t_in) -> c_int {
    let rref = u!(in_get(inn, (*inn).i.b));
    let mut yes = 0u32;
    let mut rc = 0;
    if rref != DVM_NULL {
        rc = in_isinst(inn, rref, &mut yes);
    }
    if rc == 0 {
        u!(in_set(inn, (*inn).i.a, yes));
    }
    rc
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_array_length(inn: *mut t_in) -> c_int {
    let arr = u!(in_get(inn, (*inn).i.b));
    if arr == DVM_NULL {
        return u!(in_throw(inn, desc(IN_NPE)));
    }
    let mut len = 0u32;
    let rc = u!(go_array_length((*inn).vm, arr, &mut len));
    if rc == 0 {
        u!(in_set(inn, (*inn).i.a, len));
    }
    rc
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_new_array(inn: *mut t_in) -> c_int {
    let mut rq = u! {
        t_dnewarr {
            type_idx: (*inn).i.idx,
            len: in_get(inn, (*inn).i.b) as i32,
            out: DVM_NULL,
        }
    };
    if u!((*inn).bad) != 0 {
        return 0;
    }
    let rc = u!(go_new_array((*inn).vm, (*inn).m, &mut rq));
    if rc == 0 {
        u!(in_set(inn, (*inn).i.a, rq.out));
    }
    rc
}

fn in_put_all(inn: *mut t_in, arr: t_dref, args: *const u32, n: u32) -> c_int {
    let mut v = t_darrview {
        data: core::ptr::null_mut(),
        len: 0,
        width: 0,
        is_ref: 0,
    };
    let mut rc = u!(go_array_view((*inn).vm, arr, &mut v));
    let mut acc = t_daacc {
        arr,
        index: 0,
        val: 0,
        kind: if rc == 0 && v.is_ref != 0 {
            DAK_OBJECT as u8
        } else {
            0
        },
        put: 1,
    };
    while rc == 0 && (acc.index as u32) < n {
        acc.val = u!(u64::from(*args.add(acc.index as usize)));
        rc = u!(go_array((*inn).vm, &mut acc));
        acc.index += 1;
    }
    rc
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_filled(inn: *mut t_in) -> c_int {
    let mut n = 0u32;
    let args = u!(in_gather(inn, &mut n));
    if u!((*inn).bad) != 0 {
        return 0;
    }
    let mut rq = u! {
        t_dnewarr {
            type_idx: (*inn).i.idx,
            len: n as i32,
            out: DVM_NULL,
        }
    };
    let mut rc = u!(go_new_array((*inn).vm, (*inn).m, &mut rq));
    if rc == 0 {
        rc = in_put_all(inn, rq.out, args, n);
    }
    if rc == 0 {
        u! { (*(*inn).vm).result = u64::from(rq.out); }
    }
    rc
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_fill_data(inn: *mut t_in) -> c_int {
    let arr = u!(in_get(inn, (*inn).i.a));
    let mut data = t_darray {
        width: 0,
        count: 0,
        data: core::ptr::null(),
    };
    let off = u!((*inn).pc.wrapping_add((*inn).i.lit as u32));
    if dex!(dexcode_array_data((*(*inn).m).insns, off, &mut data)) < 0 {
        return E_INVAL;
    }
    if arr == DVM_NULL {
        return u!(in_throw(inn, desc(IN_NPE)));
    }
    u!(go_fill_array((*inn).vm, arr, &data))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_goto(inn: *mut t_in) -> c_int {
    u! {
        (*inn).next = (*inn).pc.wrapping_add((*inn).i.lit as u32);
    }
    0
}

fn in_sparse(sw: *const t_dswitch, key: i32) -> u32 {
    let mut k = 0u32;
    u! {
        while k < (*sw).count && dex!(dexcode_rd32((*sw).keys.add(4 * k as usize))) != key {
            k += 1;
        }
    }
    k
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_switch(inn: *mut t_in) -> c_int {
    let mut sw = t_dswitch {
        sparse: 0,
        count: 0,
        first_key: 0,
        keys: core::ptr::null(),
        targets: core::ptr::null(),
    };
    let at = u!((*inn).pc.wrapping_add((*inn).i.lit as u32));
    if dex!(dexcode_switch((*(*inn).m).insns, at, &mut sw)) < 0 {
        return E_INVAL;
    }
    let key = u!(in_get(inn, (*inn).i.a) as i32);
    let mut k = (key as u32).wrapping_sub(sw.first_key as u32);
    if sw.sparse != 0 {
        k = in_sparse(&sw, key);
    }
    if k < sw.count {
        let delta = dex!(dexcode_rd32(sw.targets.add(4 * k as usize)));
        u! { (*inn).next = (*inn).pc.wrapping_add(delta as u32); }
    }
    0
}

fn in_cond(k: u32, x: i32, y: i32) -> bool {
    if k == 0 {
        return x == y;
    }
    if k == 1 {
        return x != y;
    }
    if k == 2 {
        return x < y;
    }
    if k == 3 {
        return x >= y;
    }
    if k == 4 {
        return x > y;
    }
    x <= y
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_if(inn: *mut t_in) -> c_int {
    u! {
        let mut y = 0i32;
        let mut k = u32::from((*inn).i.op.wrapping_sub(OP_IF_EQZ));
        if (*inn).i.op <= OP_IF_LE {
            k = u32::from((*inn).i.op.wrapping_sub(OP_IF_EQ));
            y = in_get(inn, (*inn).i.b) as i32;
        }
        if in_cond(k, in_get(inn, (*inn).i.a) as i32, y) {
            (*inn).next = (*inn).pc.wrapping_add((*inn).i.lit as u32);
        }
    }
    0
}

fn cmp_fp(x: f64, y: f64, bias: i32) -> i32 {
    if x.is_nan() || y.is_nan() {
        return bias;
    }
    i32::from(x > y) - i32::from(x < y)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_cmp(inn: *mut t_in) -> c_int {
    u! {
        let op = (*inn).i.op;
        let mut bias = -1i32;
        if op == OP_CMPG_FLOAT || op == OP_CMPG_DOUBLE {
            bias = 1;
        }
        let r = if op == OP_CMP_LONG {
            let x = in_getw(inn, (*inn).i.b) as i64;
            let y = in_getw(inn, (*inn).i.c) as i64;
            i32::from(x > y) - i32::from(x < y)
        } else if op >= OP_CMPL_DOUBLE {
            cmp_fp(f64::from_bits(in_getw(inn, (*inn).i.b)), f64::from_bits(in_getw(inn, (*inn).i.c)), bias)
        } else {
            cmp_fp(f64::from(f32::from_bits(in_get(inn, (*inn).i.b))), f64::from(f32::from_bits(in_get(inn, (*inn).i.c))), bias)
        };
        in_set(inn, (*inn).i.a, r as u32);
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_aget(inn: *mut t_in) -> c_int {
    let mut acc = u! {
        t_daacc {
            arr: in_get(inn, (*inn).i.b),
            index: in_get(inn, (*inn).i.c) as i32,
            val: 0,
            kind: (*inn).i.op.wrapping_sub(OP_AGET),
            put: 0,
        }
    };
    if u!((*inn).bad) != 0 {
        return 0;
    }
    if acc.arr == DVM_NULL {
        return u!(in_throw(inn, desc(IN_NPE)));
    }
    let rc = u!(go_array((*inn).vm, &mut acc));
    if rc != 0 {
        return rc;
    }
    u! {
        if u32::from(acc.kind) == DAK_WIDE {
            in_setw(inn, (*inn).i.a, acc.val);
        } else {
            in_set(inn, (*inn).i.a, in_narrow(u32::from(acc.kind), acc.val) as u32);
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_aput(inn: *mut t_in) -> c_int {
    let mut acc = u! {
        t_daacc {
            arr: in_get(inn, (*inn).i.b),
            index: in_get(inn, (*inn).i.c) as i32,
            val: 0,
            kind: (*inn).i.op.wrapping_sub(OP_APUT),
            put: 1,
        }
    };
    u! {
        if u32::from(acc.kind) == DAK_WIDE {
            acc.val = in_getw(inn, (*inn).i.a);
        } else {
            acc.val = in_narrow(u32::from(acc.kind), u64::from(in_get(inn, (*inn).i.a)));
        }
        if (*inn).bad != 0 {
            return 0;
        }
    }
    if acc.arr == DVM_NULL {
        return u!(in_throw(inn, desc(IN_NPE)));
    }
    u!(go_array((*inn).vm, &mut acc))
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_field(inn: *mut t_in) -> c_int {
    let mut f = u! {
        let k = u32::from((*inn).i.op.wrapping_sub(OP_IGET));
        let mut f = crate::t_dfacc {
            field_idx: (*inn).i.idx,
            obj: DVM_NULL,
            val: 0,
            wide: u8::from(k % 7 == 1),
            is_ref: u8::from(k % 7 == 2),
            put: u8::from((k % 14) >= 7),
            is_static: u8::from(k >= 14),
        };
        if f.is_static == 0 {
            f.obj = in_get(inn, (*inn).i.b);
        }
        if f.put != 0 && f.wide != 0 {
            f.val = in_getw(inn, (*inn).i.a);
        } else if f.put != 0 {
            f.val = in_narrow(k % 7, u64::from(in_get(inn, (*inn).i.a)));
        }
        if (*inn).bad != 0 {
            return 0;
        }
        f
    };
    if f.is_static == 0 && f.obj == DVM_NULL {
        return u!(in_throw(inn, desc(IN_NPE)));
    }
    let rc = u!(go_field((*inn).vm, (*inn).m, &mut f));
    if rc != 0 || f.put != 0 {
        return rc;
    }
    let k = u!(u32::from((*inn).i.op.wrapping_sub(OP_IGET)));
    u! {
        if f.wide != 0 {
            in_setw(inn, (*inn).i.a, f.val);
        } else {
            in_set(inn, (*inn).i.a, in_narrow(k % 7, f.val) as u32);
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_invoke(inn: *mut t_in) -> c_int {
    let mut n = 0u32;
    let args = u!(in_gather(inn, &mut n));
    let mut inv = u! {
        t_dinvoke {
            method_idx: (*inn).i.idx,
            self_: DVM_NULL,
            target: core::ptr::null(),
            kind: (*inn).i.op.wrapping_sub(OP_INVOKE_VIRTUAL) % 6,
        }
    };
    if u!((*inn).bad) != 0 || (inv.kind != DIK_STATIC && n == 0) {
        return u!(in_throw(inn, desc(IN_VERIFY)));
    }
    if inv.kind != DIK_STATIC {
        inv.self_ = u!(*args);
    }
    let rc = u!(go_resolve((*inn).vm, (*inn).m, &mut inv));
    if rc != 0 {
        return rc;
    }
    if inv.target.is_null() || u!(u32::from((*inv.target).ins) != n) {
        return u!(in_throw(inn, desc(IN_VERIFY)));
    }
    u!(in_enter(inn, inv.target, args))
}

fn in_un32(op: u8, x: u32) -> u32 {
    if op == OP_NEG_INT {
        return 0u32.wrapping_sub(x);
    }
    if op == OP_NOT_INT {
        return !x;
    }
    if op == OP_INT_TO_BYTE {
        return x as u8 as i8 as i32 as u32;
    }
    if op == OP_INT_TO_CHAR {
        return u32::from(x as u16);
    }
    x as u16 as i16 as i32 as u32
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_un_int(inn: *mut t_in) -> c_int {
    u! {
        let x = in_get(inn, (*inn).i.b);
        let op = (*inn).i.op;
        if op == OP_INT_TO_LONG {
            in_setw(inn, (*inn).i.a, (x as i32 as i64) as u64);
        } else if op == OP_INT_TO_DOUBLE {
            in_setw(inn, (*inn).i.a, f64b(f64::from(x as i32)));
        } else if op == OP_INT_TO_FLOAT {
            in_set(inn, (*inn).i.a, f32b((x as i32) as f32));
        } else {
            in_set(inn, (*inn).i.a, in_un32(op, x));
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_un_long(inn: *mut t_in) -> c_int {
    u! {
        let x = in_getw(inn, (*inn).i.b);
        let op = (*inn).i.op;
        if op == OP_NEG_LONG {
            in_setw(inn, (*inn).i.a, 0u64.wrapping_sub(x));
        } else if op == OP_NOT_LONG {
            in_setw(inn, (*inn).i.a, !x);
        } else if op == OP_LONG_TO_INT {
            in_set(inn, (*inn).i.a, x as u32);
        } else if op == OP_LONG_TO_FLOAT {
            in_set(inn, (*inn).i.a, f32b((x as i64) as f32));
        } else {
            in_setw(inn, (*inn).i.a, f64b((x as i64) as f64));
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_un_float(inn: *mut t_in) -> c_int {
    u! {
        let x = in_get(inn, (*inn).i.b);
        let d = f64::from(f32::from_bits(x));
        let op = (*inn).i.op;
        if op == OP_NEG_FLOAT {
            in_set(inn, (*inn).i.a, x ^ 0x8000_0000);
        } else if op == OP_FLOAT_TO_INT {
            in_set(inn, (*inn).i.a, in_d2i(d) as u32);
        } else if op == OP_FLOAT_TO_LONG {
            in_setw(inn, (*inn).i.a, in_d2l(d) as u64);
        } else {
            in_setw(inn, (*inn).i.a, f64b(d));
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_un_double(inn: *mut t_in) -> c_int {
    u! {
        let x = in_getw(inn, (*inn).i.b);
        let d = f64::from_bits(x);
        let op = (*inn).i.op;
        if op == OP_NEG_DOUBLE {
            in_setw(inn, (*inn).i.a, x ^ (1u64 << 63));
        } else if op == OP_DOUBLE_TO_INT {
            in_set(inn, (*inn).i.a, in_d2i(d) as u32);
        } else if op == OP_DOUBLE_TO_LONG {
            in_setw(inn, (*inn).i.a, in_d2l(d) as u64);
        } else {
            in_set(inn, (*inn).i.a, f32b(d as f32));
        }
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_ibin(inn: *mut t_in) -> c_int {
    u! {
        let mut src0 = (*inn).i.b;
        let mut src1 = (*inn).i.c;
        if (*inn).i.op >= OP_ADD_INT_2ADDR {
            src0 = (*inn).i.a;
            src1 = (*inn).i.b;
        }
        let mut x = in_get(inn, src0);
        let y = in_get(inn, src1);
        let rc = in_iop(inn, u32::from((*inn).i.op.wrapping_sub(OP_ADD_INT)) % 32, &mut x, y);
        if rc == 0 {
            in_set(inn, (*inn).i.a, x);
        }
        rc
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_ilit(inn: *mut t_in) -> c_int {
    u! {
        let mut k = u32::from((*inn).i.op.wrapping_sub(OP_ADD_INT_LIT16)) % 8;
        if (*inn).i.op >= OP_ADD_INT_LIT8 {
            k = u32::from((*inn).i.op.wrapping_sub(OP_ADD_INT_LIT8));
        }
        let mut x = in_get(inn, (*inn).i.b);
        let mut y = (*inn).i.lit as u32;
        if k == 1 {
            core::mem::swap(&mut x, &mut y);
        }
        let rc = in_iop(inn, k, &mut x, y);
        if rc == 0 {
            in_set(inn, (*inn).i.a, x);
        }
        rc
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_lbin(inn: *mut t_in) -> c_int {
    u! {
        let mut src0 = (*inn).i.b;
        let mut src1 = (*inn).i.c;
        if (*inn).i.op >= OP_ADD_LONG_2ADDR {
            src0 = (*inn).i.a;
            src1 = (*inn).i.b;
        }
        let k = u32::from((*inn).i.op.wrapping_sub(OP_ADD_LONG)) % 32;
        let mut v0 = in_getw(inn, src0);
        let v1 = if k >= 8 {
            u64::from(in_get(inn, src1))
        } else {
            in_getw(inn, src1)
        };
        let rc = in_lop(inn, k, &mut v0, v1);
        if rc == 0 {
            in_setw(inn, (*inn).i.a, v0);
        }
        rc
    }
}

fn in_dop(k: u32, x: f64, y: f64) -> f64 {
    if k == 0 {
        return x + y;
    }
    if k == 1 {
        return x - y;
    }
    if k == 2 {
        return x * y;
    }
    if k == 3 {
        return x / y;
    }
    u!(in_fmod(x, y))
}

fn in_fop(k: u32, x: f32, y: f32) -> f32 {
    if k == 0 {
        return x + y;
    }
    if k == 1 {
        return x - y;
    }
    if k == 2 {
        return x * y;
    }
    if k == 3 {
        return x / y;
    }
    u!(in_fmod(f64::from(x), f64::from(y)) as f32)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_fbin(inn: *mut t_in) -> c_int {
    u! {
        let mut src0 = (*inn).i.b;
        let mut src1 = (*inn).i.c;
        if (*inn).i.op >= OP_ADD_FLOAT_2ADDR {
            src0 = (*inn).i.a;
            src1 = (*inn).i.b;
        }
        let k = u32::from((*inn).i.op.wrapping_sub(OP_ADD_FLOAT)) % 32;
        let v = in_fop(k, f32::from_bits(in_get(inn, src0)), f32::from_bits(in_get(inn, src1)));
        in_set(inn, (*inn).i.a, f32b(v));
    }
    0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn in_op_dbin(inn: *mut t_in) -> c_int {
    u! {
        let mut src0 = (*inn).i.b;
        let mut src1 = (*inn).i.c;
        if (*inn).i.op >= OP_ADD_DOUBLE_2ADDR {
            src0 = (*inn).i.a;
            src1 = (*inn).i.b;
        }
        let k = u32::from((*inn).i.op.wrapping_sub(OP_ADD_DOUBLE)) % 32;
        let v = in_dop(k, f64::from_bits(in_getw(inn, src0)), f64::from_bits(in_getw(inn, src1)));
        in_setw(inn, (*inn).i.a, f64b(v));
    }
    0
}
