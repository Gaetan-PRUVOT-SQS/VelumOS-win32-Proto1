#![allow(clippy::undocumented_unsafe_blocks)]

use core::ffi::{c_char, c_int};

use crate::export::{
    dvm_array_new, dvm_builtins, dvm_call, dvm_class, dvm_class_of, dvm_create, dvm_destroy,
    dvm_gc, dvm_heap_stats, dvm_is_instance, dvm_local, dvm_local_pop, dvm_new, dvm_pin,
    dvm_string_get, dvm_string_utf8_new, dvm_throw, dvm_unpin, rt_obj, rt_sig_words, rt_utf8_to16,
    rt_utf16_to8,
};
use crate::ins::{
    in_d2i, in_d2l, in_enter, in_fmod, in_get, in_native, in_op_cmp, in_op_dbin, in_op_fbin,
    in_op_ibin, in_op_ilit, in_op_lbin, in_op_un_double, in_op_un_float, in_op_un_int,
    in_op_un_long, in_push,
};
use crate::mem::{arm, disarm, live};
use crate::{
    ACC_PUBLIC, DOK_STRING, DVM_LOCALS_MAX, DVM_NULL, DVM_THROWN, E_EXIST, E_INVAL, E_NOMEM,
    E_OVERFLOW, E_RANGE, E_TIMEOUT, t_dbuiltin, t_dheapstats, t_dinsn, t_dlimits, t_dmethod,
    t_dstr16, t_dvm, t_in, t_span, t_text,
};

fn zinsn() -> t_dinsn {
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

fn zmethod() -> t_dmethod {
    t_dmethod {
        cls: core::ptr::null_mut(),
        name: core::ptr::null(),
        sig: core::ptr::null(),
        shorty: core::ptr::null(),
        dex: core::ptr::null(),
        native: None,
        insns: t_span {
            p: core::ptr::null(),
            len: 0,
        },
        code_off: 0,
        access: 0,
        slot: 0,
        registers: 0,
        ins: 0,
        outs: 0,
        tries: 0,
    }
}

fn frame(vm: *mut t_dvm, regs: &mut [u32; 8], op: u8) -> t_in {
    let mut i = zinsn();
    i.op = op;
    t_in {
        vm,
        m: core::ptr::null(),
        r: regs.as_mut_ptr(),
        nreg: 8,
        pc: 0,
        next: 0,
        fp: 0,
        fp0: 0,
        done: 0,
        bad: 0,
        i,
        argv: [0; 5],
    }
}

fn cs(s: &[u8]) -> *const c_char {
    s.as_ptr().cast()
}

fn open() -> *mut t_dvm {
    let mut vm = core::ptr::null_mut();
    // SAFETY: out est une locale. lim nul demande les défauts.
    let rc = unsafe { dvm_create(&mut vm, core::ptr::null()) };
    assert_eq!(rc, 0);
    assert!(!vm.is_null());
    vm
}

fn shut(vm: *mut t_dvm, base: i32) {
    // SAFETY: vm vient de dvm_create dans ce test.
    unsafe { dvm_destroy(vm) };
    assert_eq!(live(), base);
}

fn put64(regs: &mut [u32; 8], at: usize, v: u64) {
    regs[at] = v as u32;
    regs[at + 1] = (v >> 32) as u32;
}

fn get64(regs: &[u32; 8], at: usize) -> u64 {
    u64::from(regs[at]) | (u64::from(regs[at + 1]) << 32)
}

#[test]
fn sig_words_large_et_tableaux() {
    // SAFETY: littéraux C terminés.
    unsafe {
        assert_eq!(rt_sig_words(cs(b"(I)V\0")), 1);
        assert_eq!(rt_sig_words(cs(b"(J)V\0")), 2);
        assert_eq!(rt_sig_words(cs(b"([D)V\0")), 1);
        assert_eq!(rt_sig_words(cs(b"(JD)V\0")), 4);
        assert_eq!(rt_sig_words(cs(b"()V\0")), 0);
        assert_eq!(rt_sig_words(cs(b"(Ljava/lang/String;)V\0")), 1);
    }
}

#[test]
fn utf_limites() {
    let mut u = [0u16; 4];
    // SAFETY: chaîne C et tampon de 4 unités.
    let n = unsafe { rt_utf8_to16(cs(b"\xF0\x9F\x98\x80\0"), u.as_mut_ptr()) };
    assert_eq!(n, 2);
    assert_eq!(u[0], 0xD83D);
    assert_eq!(u[1], 0xDE00);
    let bad = unsafe { rt_utf8_to16(cs(b"\xC0\x80\0"), u.as_mut_ptr()) };
    assert_eq!(bad, E_INVAL);
    let lone = [0xD800u16];
    let mut buf = [0u8; 8];
    let s = t_dstr16 {
        p: lone.as_ptr(),
        n: 1,
    };
    let out = t_text {
        p: buf.as_mut_ptr().cast(),
        cap: 8,
    };
    // SAFETY: une unité UTF-16 et 8 octets de sortie.
    let k = unsafe { rt_utf16_to8(s, out) };
    assert_eq!(k, 3);
    assert_eq!(&buf[..3], &[0xef, 0xbf, 0xbd]);
    let tight = t_text {
        p: buf.as_mut_ptr().cast(),
        cap: 3,
    };
    assert_eq!(unsafe { rt_utf16_to8(s, tight) }, E_OVERFLOW);
    let empty = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    let nowhere = t_text {
        p: core::ptr::null_mut(),
        cap: 1,
    };
    assert_eq!(unsafe { rt_utf16_to8(empty, nowhere) }, E_OVERFLOW);
}

#[test]
fn creation_invalide_et_plafonds() {
    let base = live();
    let mut vm = core::ptr::null_mut();
    assert_eq!(
        unsafe { dvm_create(core::ptr::null_mut(), core::ptr::null()) },
        E_INVAL
    );
    unsafe { dvm_destroy(core::ptr::null_mut()) };
    let mut lim = t_dlimits {
        heap_bytes: 0,
        stack_words: 0,
        depth_max: 0,
        classes_max: 4,
        budget: 0,
    };
    assert_eq!(unsafe { dvm_create(&mut vm, &lim) }, E_INVAL);
    assert!(vm.is_null());
    lim.classes_max = 32;
    lim.stack_words = 16_777_217;
    assert_eq!(unsafe { dvm_create(&mut vm, &lim) }, E_INVAL);
    lim.stack_words = 16;
    lim.heap_bytes = 8;
    assert_eq!(unsafe { dvm_create(&mut vm, &lim) }, E_NOMEM);
    assert!(vm.is_null());
    assert_eq!(live(), base);
}

#[test]
fn creation_chaque_calloc_en_echec() {
    let base = live();
    let mut k = 0u32;
    let mut r = E_NOMEM;
    let mut vm = core::ptr::null_mut();
    while r != 0 && k < 64 {
        arm(k);
        r = unsafe { dvm_create(&mut vm, core::ptr::null()) };
        disarm();
        if r != 0 {
            assert_eq!(r, E_NOMEM);
            assert!(vm.is_null());
            assert_eq!(live(), base);
        }
        k += 1;
    }
    assert_eq!(r, 0);
    assert!(k > 8);
    shut(vm, base);
}

#[test]
fn operations_chaque_calloc_en_echec() {
    let base = live();
    let vm = open();
    let mut k = 0u32;
    let mut r = E_NOMEM;
    while r != 0 && k < 64 {
        arm(k);
        r = scenario(vm);
        disarm();
        if r != 0 {
            assert_eq!(r, E_NOMEM);
        }
        unsafe { dvm_gc(vm) };
        k += 1;
    }
    assert_eq!(r, 0);
    assert!(k > 4);
    assert_eq!(scenario(vm), 0);
    shut(vm, base);
}

fn scenario(vm: *mut t_dvm) -> c_int {
    let b = t_dbuiltin {
        desc: cs(b"Lv/Echec;\0"),
        super_desc: cs(b"Ljava/lang/Object;\0"),
        iface_desc: core::ptr::null(),
        access: ACC_PUBLIC,
        payload_bytes: 16,
    };
    // SAFETY: vm vivant, descripteurs terminés.
    let mut r = unsafe { dvm_builtins(vm, &b, 1) };
    let mut o = DVM_NULL;
    if r == 0 || r == E_EXIST {
        r = unsafe { dvm_string_utf8_new(vm, cs(b"abc\0"), &mut o) };
    }
    if r == 0 {
        r = unsafe { dvm_array_new(vm, cs(b"[[Ljava/lang/String;\0"), 4, &mut o) };
    }
    if r == 0 {
        r = unsafe { dvm_throw(vm, cs(b"Lx/Absente;\0"), cs(b"message\0")) };
    }
    if r == DVM_THROWN { 0 } else { r }
}

#[test]
fn classe_absente_et_tableau_negatif() {
    let base = live();
    let vm = open();
    let mut c = core::ptr::null_mut();
    let rc = unsafe { dvm_class(vm, cs(b"Nope\0"), &mut c) };
    assert_eq!(rc, DVM_THROWN);
    assert!(c.is_null());
    let mut pending = unsafe { (*vm).pending };
    let cls = unsafe { dvm_class_of(vm, pending) };
    assert!(!cls.is_null());
    assert_eq!(
        unsafe { dvm_array_new(vm, cs(b"[I\0"), -1, &mut pending) },
        DVM_THROWN
    );
    shut(vm, base);
}

#[test]
fn chaine_emoji_et_gc() {
    let base = live();
    let vm = open();
    let mut avant = t_dheapstats {
        bytes: 0,
        objects: 0,
        collections: 0,
        peak_bytes: 0,
    };
    unsafe { dvm_heap_stats(vm, &mut avant) };
    let mut s = DVM_NULL;
    assert_eq!(
        unsafe { dvm_string_utf8_new(vm, cs(b"\xF0\x9F\x98\x80\0"), &mut s) },
        0
    );
    let mut view = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    assert_eq!(unsafe { dvm_string_get(vm, s, &mut view) }, 0);
    assert_eq!(view.n, 2);
    assert_eq!(unsafe { *view.p }, 0xD83D);
    assert_eq!(unsafe { *view.p.add(1) }, 0xDE00);
    unsafe { dvm_gc(vm) };
    assert!(unsafe { dvm_class_of(vm, s) }.is_null());
    let mut kept = DVM_NULL;
    assert_eq!(
        unsafe { dvm_string_utf8_new(vm, cs(b"garde\0"), &mut kept) },
        0
    );
    assert_eq!(unsafe { dvm_pin(vm, kept) }, 0);
    unsafe { dvm_gc(vm) };
    assert!(!unsafe { dvm_class_of(vm, kept) }.is_null());
    unsafe { dvm_unpin(vm, kept) };
    unsafe { dvm_gc(vm) };
    assert!(unsafe { dvm_class_of(vm, kept) }.is_null());
    let mut lost = DVM_NULL;
    assert_eq!(
        unsafe { dvm_string_utf8_new(vm, cs(b"perdue\0"), &mut lost) },
        0
    );
    unsafe { dvm_gc(vm) };
    let mut apres = avant;
    unsafe { dvm_heap_stats(vm, &mut apres) };
    assert_eq!(apres.objects, avant.objects);
    assert_eq!(apres.bytes, avant.bytes);
    assert_eq!(apres.collections, avant.collections + 4);
    let mut again = 0u32;
    assert_eq!(
        unsafe { dvm_string_utf8_new(vm, cs(b"autre\0"), &mut again) },
        0
    );
    assert_eq!(again, lost);
    let o = unsafe { rt_obj(vm, again, DOK_STRING) };
    assert!(!o.is_null());
    unsafe { (*o).pins = u16::MAX };
    assert_eq!(unsafe { dvm_pin(vm, again) }, E_RANGE);
    shut(vm, base);
}

#[test]
fn locaux_soixante_quatre() {
    let base = live();
    let vm = open();
    for _ in 0..DVM_LOCALS_MAX {
        assert_eq!(unsafe { dvm_local(vm, DVM_NULL) }, 0);
    }
    assert_eq!(unsafe { dvm_local(vm, DVM_NULL) }, E_RANGE);
    unsafe { dvm_local_pop(vm, DVM_LOCALS_MAX) };
    assert_eq!(unsafe { (*vm).nlocals }, 0);
    assert_eq!(unsafe { dvm_local(vm, 77777) }, E_INVAL);
    shut(vm, base);
}

#[test]
fn objet_vers_object_et_second_builtin() {
    let base = live();
    let vm = open();
    let b = t_dbuiltin {
        desc: cs(b"Lv/Deja;\0"),
        super_desc: cs(b"Ljava/lang/Object;\0"),
        iface_desc: core::ptr::null(),
        access: ACC_PUBLIC,
        payload_bytes: 0,
    };
    assert_eq!(unsafe { dvm_builtins(vm, &b, 1) }, 0);
    assert_eq!(unsafe { dvm_builtins(vm, &b, 1) }, E_EXIST);
    let mut obj = core::ptr::null_mut();
    assert_eq!(
        unsafe { dvm_class(vm, cs(b"Ljava/lang/Object;\0"), &mut obj) },
        0
    );
    let mut r = DVM_NULL;
    assert_eq!(unsafe { dvm_new(vm, obj, &mut r) }, 0);
    assert_eq!(unsafe { dvm_is_instance(vm, r, obj) }, 1);
    shut(vm, base);
}

#[test]
fn pile_debordement_budget_et_natif() {
    let base = live();
    let mut vm = core::ptr::null_mut();
    let lim = t_dlimits {
        heap_bytes: 1 << 20,
        stack_words: 16,
        depth_max: 4,
        classes_max: 32,
        budget: 0,
    };
    assert_eq!(unsafe { dvm_create(&mut vm, &lim) }, 0);
    let mut regs = [0u32; 8];
    let mut inn = frame(vm, &mut regs, 0);
    let mut m = zmethod();
    m.registers = 20;
    assert_eq!(
        unsafe { in_push(&mut inn, &m, core::ptr::null()) },
        DVM_THROWN
    );
    m.registers = 1;
    m.access = crate::ACC_ABSTRACT;
    assert_eq!(
        unsafe { in_enter(&mut inn, &m, core::ptr::null()) },
        DVM_THROWN
    );
    m.access = 0;
    m.native = None;
    let depth = unsafe { (*vm).depth };
    assert_eq!(
        unsafe { in_native(&mut inn, &m, core::ptr::null()) },
        E_INVAL
    );
    assert_eq!(unsafe { (*vm).depth }, depth);
    unsafe extern "C" fn nat(_vm: *mut t_dvm, _a: *const u32, ret: *mut u64) -> c_int {
        // SAFETY: ret est le tampon fourni par in_native.
        unsafe { *ret = 42 };
        0
    }
    m.native = Some(nat);
    assert_eq!(unsafe { in_native(&mut inn, &m, core::ptr::null()) }, 0);
    assert_eq!(unsafe { (*vm).result }, 42);
    let code = [0u8, 0];
    m.native = None;
    m.insns = t_span {
        p: code.as_ptr(),
        len: 2,
    };
    m.registers = 1;
    m.ins = 0;
    unsafe {
        (*vm).lim.budget = 5;
        (*vm).spent = 5;
    }
    let mut ret = 0u64;
    assert_eq!(
        unsafe { dvm_call(vm, &m, core::ptr::null(), &mut ret) },
        E_TIMEOUT
    );
    assert_eq!(unsafe { (*vm).spent }, 5);
    unsafe {
        (*vm).lim.budget = 0;
        (*vm).spent = 0;
    }
    assert_eq!(
        unsafe { dvm_call(vm, &m, core::ptr::null(), &mut ret) },
        E_INVAL
    );
    assert_eq!(unsafe { (*vm).spent }, 1);
    assert_eq!(
        unsafe { dvm_call(core::ptr::null_mut(), &m, core::ptr::null(), &mut ret) },
        E_INVAL
    );
    shut(vm, base);
}

fn run_ibin(vm: *mut t_dvm, op: u8, x: u32, y: u32, two: bool) -> (u32, c_int) {
    let mut regs = [0u32; 8];
    let mut inn = frame(vm, &mut regs, op);
    if two {
        regs[1] = x;
        regs[2] = y;
        inn.i.a = 1;
        inn.i.b = 2;
    } else {
        regs[1] = x;
        regs[2] = y;
        inn.i.a = 0;
        inn.i.b = 1;
        inn.i.c = 2;
    }
    let rc = unsafe { in_op_ibin(&mut inn) };
    let got = if two { regs[1] } else { regs[0] };
    (got, rc)
}

fn run_lbin(vm: *mut t_dvm, op: u8, x: u64, y: u64, two: bool) -> (u64, c_int) {
    let mut regs = [0u32; 8];
    let mut inn = frame(vm, &mut regs, op);
    let k = u32::from(op.wrapping_sub(0x9b)) % 32;
    if two {
        put64(&mut regs, 0, x);
        if k >= 8 {
            regs[2] = y as u32;
        } else {
            put64(&mut regs, 2, y);
        }
        inn.i.a = 0;
        inn.i.b = 2;
    } else {
        put64(&mut regs, 2, x);
        if k >= 8 {
            regs[4] = y as u32;
        } else {
            put64(&mut regs, 4, y);
        }
        inn.i.a = 0;
        inn.i.b = 2;
        inn.i.c = 4;
    }
    let rc = unsafe { in_op_lbin(&mut inn) };
    (get64(&regs, 0), rc)
}

fn run_lit(vm: *mut t_dvm, op: u8, x: u32, y: u32) -> (u32, c_int) {
    let mut regs = [0u32; 8];
    regs[1] = x;
    let mut inn = frame(vm, &mut regs, op);
    inn.i.a = 0;
    inn.i.b = 1;
    inn.i.lit = if op >= 0xd8 {
        i64::from(y as u8 as i8)
    } else {
        i64::from(y as u16 as i16)
    };
    let rc = unsafe { in_op_ilit(&mut inn) };
    (regs[0], rc)
}

fn run_un(vm: *mut t_dvm, op: u8, x: u64) -> u64 {
    let mut regs = [0u32; 8];
    let mut inn = frame(vm, &mut regs, op);
    let wide_src = matches!(op, 0x7d | 0x7e | 0x84..=0x86 | 0x8a..=0x8c);
    if wide_src {
        put64(&mut regs, 2, x);
        inn.i.b = 2;
    } else {
        regs[1] = x as u32;
        inn.i.b = 1;
    }
    inn.i.a = 0;
    let rc = unsafe {
        if matches!(op, 0x7b | 0x7c | 0x81..=0x83 | 0x8d..=0x8f) {
            in_op_un_int(&mut inn)
        } else if matches!(op, 0x7d | 0x7e | 0x84..=0x86) {
            in_op_un_long(&mut inn)
        } else if matches!(op, 0x7f | 0x87..=0x89) {
            in_op_un_float(&mut inn)
        } else {
            in_op_un_double(&mut inn)
        }
    };
    assert_eq!(rc, 0);
    if matches!(
        op,
        0x7d | 0x7e | 0x80 | 0x81 | 0x83 | 0x86 | 0x88 | 0x89 | 0x8b
    ) {
        get64(&regs, 0)
    } else {
        u64::from(regs[0])
    }
}

fn run_cmp(vm: *mut t_dvm, op: u8, x: u64, y: u64) -> u32 {
    let mut regs = [0u32; 8];
    let mut inn = frame(vm, &mut regs, op);
    let wide = op >= 0x2f;
    if wide {
        put64(&mut regs, 2, x);
        put64(&mut regs, 4, y);
        inn.i.b = 2;
        inn.i.c = 4;
    } else {
        regs[1] = x as u32;
        regs[2] = y as u32;
        inn.i.b = 1;
        inn.i.c = 2;
    }
    inn.i.a = 0;
    assert_eq!(unsafe { in_op_cmp(&mut inn) }, 0);
    regs[0]
}

#[test]
fn arith_entiers_oracle_d09() {
    let base = live();
    let vm = open();
    let ints: &[(u8, u32, u32, u32, c_int)] = &[
        (0x90, 0x7fffffff, 1, 0x80000000, 0),
        (0x91, 0x80000000, 1, 0x7fffffff, 0),
        (0x92, 100000, 100000, 1410065408, 0),
        (0x93, 7, 2, 3, 0),
        (0x93, 0xfffffff9, 2, 0xfffffffd, 0),
        (0x93, 0x80000000, 0xffffffff, 0x80000000, 0),
        (0x93, 1, 0, 0, DVM_THROWN),
        (0x94, 0xfffffff9, 2, 0xffffffff, 0),
        (0x94, 0x80000000, 0xffffffff, 0, 0),
        (0x94, 1, 0, 0, DVM_THROWN),
        (0x95, 0xf0f0, 0xff00, 0xf000, 0),
        (0x96, 0xf0f0, 0xff00, 0xfff0, 0),
        (0x97, 0xf0f0, 0xff00, 0x0ff0, 0),
        (0x98, 1, 31, 0x80000000, 0),
        (0x98, 1, 33, 2, 0),
        (0x99, 0x80000000, 31, 0xffffffff, 0),
        (0x99, 0x80000000, 32, 0x80000000, 0),
        (0x9a, 0x80000000, 31, 1, 0),
        (0x9a, 0xffffffff, 32, 0xffffffff, 0),
    ];
    for (op, x, y, want, rc) in ints {
        let (got, r) = run_ibin(vm, *op, *x, *y, false);
        assert_eq!(r, *rc);
        if *rc == 0 {
            assert_eq!(got, *want);
        }
    }
    let lits: &[(u8, u32, u32, u32, c_int)] = &[
        (0xd8, 0x7fffffff, 1, 0x80000000, 0),
        (0xd9, 3, 10, 7, 0),
        (0xda, 0xfffffffd, 0xfe, 6, 0),
        (0xdb, 0xfffffff9, 2, 0xfffffffd, 0),
        (0xdb, 0x80000000, 0xff, 0x80000000, 0),
        (0xdb, 1, 0, 0, DVM_THROWN),
        (0xdc, 0xfffffff9, 2, 0xffffffff, 0),
        (0xdc, 1, 0, 0, DVM_THROWN),
        (0xdd, 0xff, 0x0f, 0x0f, 0),
        (0xde, 1, 0x80, 0xffffff81, 0),
        (0xdf, 0, 0xff, 0xffffffff, 0),
        (0xe0, 1, 33, 2, 0),
        (0xe1, 0xfffffff8, 1, 0xfffffffc, 0),
        (0xe2, 0xfffffff8, 28, 0xf, 0),
        (0xd0, 1, 0x8000, 0xffff8001, 0),
        (0xd1, 1, 0x7fff, 0x7ffe, 0),
        (0xd2, 3, 1000, 3000, 0),
        (0xd3, 100, 7, 14, 0),
        (0xd3, 100, 0, 0, DVM_THROWN),
        (0xd4, 100, 7, 2, 0),
        (0xd5, 0x12345, 0x0ff0, 0x0340, 0),
        (0xd6, 0x10000, 0x00ff, 0x100ff, 0),
        (0xd7, 0, 0xffff, 0xffffffff, 0),
    ];
    for (op, x, y, want, rc) in lits {
        let (got, r) = run_lit(vm, *op, *x, *y);
        assert_eq!(r, *rc);
        if *rc == 0 {
            assert_eq!(got, *want);
        }
    }
    let addr: &[(u8, u32, u32, u32, c_int)] = &[
        (0xb0, 0x7fffffff, 1, 0x80000000, 0),
        (0xb3, 5, 0, 0, DVM_THROWN),
        (0xb4, 17, 5, 2, 0),
        (0xb8, 1, 33, 2, 0),
    ];
    for (op, x, y, want, rc) in addr {
        let (got, r) = run_ibin(vm, *op, *x, *y, true);
        assert_eq!(r, *rc);
        if *rc == 0 {
            assert_eq!(got, *want);
        }
    }
    let longs: &[(u8, u64, u64, u64, c_int)] = &[
        (0x9b, 0x7fffffffffffffff, 1, 0x8000000000000000, 0),
        (0x9c, 0x8000000000000000, 1, 0x7fffffffffffffff, 0),
        (0x9d, 0x100000000, 0x100000000, 0, 0),
        (0x9e, 0xfffffffffffffff9, 2, 0xfffffffffffffffd, 0),
        (
            0x9e,
            0x8000000000000000,
            0xffffffffffffffff,
            0x8000000000000000,
            0,
        ),
        (0x9e, 1, 0, 0, DVM_THROWN),
        (0x9f, 0xfffffffffffffff9, 2, 0xffffffffffffffff, 0),
        (0x9f, 0x8000000000000000, 0xffffffffffffffff, 0, 0),
        (0x9f, 1, 0, 0, DVM_THROWN),
        (
            0xa0,
            0xf0f0f0f0f0f0f0f0,
            0xff00ff00ff00ff00,
            0xf000f000f000f000,
            0,
        ),
        (0xa1, 0xf0, 0x100000000, 0x1000000f0, 0),
        (0xa2, 0xffffffffffffffff, 0xffff, 0xffffffffffff0000, 0),
        (0xa3, 1, 63, 0x8000000000000000, 0),
        (0xa3, 1, 65, 2, 0),
        (0xa4, 0x8000000000000000, 63, 0xffffffffffffffff, 0),
        (0xa4, 0x8000000000000000, 64, 0x8000000000000000, 0),
        (0xa5, 0x8000000000000000, 63, 1, 0),
    ];
    for (op, x, y, want, rc) in longs {
        let (got, r) = run_lbin(vm, *op, *x, *y, false);
        assert_eq!(r, *rc);
        if *rc == 0 {
            assert_eq!(got, *want);
        }
    }
    let (got, r) = run_lbin(vm, 0xbb, 0xffffffff, 1, true);
    assert_eq!(r, 0);
    assert_eq!(got, 0x100000000);
    let (got, r) = run_lbin(vm, 0xbe, 0x8000000000000000, 0xffffffffffffffff, true);
    assert_eq!(r, 0);
    assert_eq!(got, 0x8000000000000000);
    let (got, r) = run_lbin(vm, 0xc3, 1, 65, true);
    assert_eq!(r, 0);
    assert_eq!(got, 2);
    shut(vm, base);
}

#[test]
fn conversions_et_comparaisons() {
    let base = live();
    let vm = open();
    let rows: &[(u8, u64, u64)] = &[
        (0x7b, 0x80000000, 0x80000000),
        (0x7c, 0, 0xffffffff),
        (0x7d, 0x8000000000000000, 0x8000000000000000),
        (0x7e, 0, 0xffffffffffffffff),
        (0x7f, 0, 0x80000000),
        (0x80, 0, 0x8000000000000000),
        (0x81, 0xffffffff, 0xffffffffffffffff),
        (0x82, 0x7fffffff, 0x4f000000),
        (0x83, 0xffffffff, 0xbff0000000000000),
        (0x84, 0x1ffffffff, 0xffffffff),
        (0x85, 1, 0x3f800000),
        (0x86, 0xffffffffffffffff, 0xbff0000000000000),
        (0x87, 0x7fc00000, 0),
        (0x87, 0x7f800000, 0x7fffffff),
        (0x87, 0xff800000, 0x80000000),
        (0x87, 0x4f000000, 0x7fffffff),
        (0x87, 0xcf000000, 0x80000000),
        (0x87, 0xbfc00000, 0xffffffff),
        (0x88, 0x7fc00000, 0),
        (0x88, 0x7f800000, 0x7fffffffffffffff),
        (0x88, 0x5f000000, 0x7fffffffffffffff),
        (0x88, 0xdf000000, 0x8000000000000000),
        (0x88, 0xbfc00000, 0xffffffffffffffff),
        (0x89, 0x3fc00000, 0x3ff8000000000000),
        (0x8a, 0x7ff8000000000000, 0),
        (0x8a, 0x7ff0000000000000, 0x7fffffff),
        (0x8a, 0xfff0000000000000, 0x80000000),
        (0x8a, 0x41dfffffffe00000, 0x7fffffff),
        (0x8a, 0xc1e0000000100000, 0x80000000),
        (0x8a, 0xbfeccccccccccccd, 0),
        (0x8b, 0x7ff8000000000000, 0),
        (0x8b, 0x7ff0000000000000, 0x7fffffffffffffff),
        (0x8b, 0xfff0000000000000, 0x8000000000000000),
        (0x8b, 0x43e0000000000000, 0x7fffffffffffffff),
        (0x8b, 0xc3e0000000000000, 0x8000000000000000),
        (0x8b, 0xbff8000000000000, 0xffffffffffffffff),
        (0x8c, 0x3ff8000000000000, 0x3fc00000),
        (0x8c, 0x7e37e43c8800759c, 0x7f800000),
        (0x8d, 0x180, 0xffffff80),
        (0x8d, 0x7f, 0x7f),
        (0x8e, 0xffffffff, 0xffff),
        (0x8f, 0x18000, 0xffff8000),
    ];
    for (op, x, want) in rows {
        assert_eq!(run_un(vm, *op, *x), *want);
    }
    let cmps: &[(u8, u64, u64, u32)] = &[
        (0x2d, 0x7fc00000, 0x3f800000, 0xffffffff),
        (0x2e, 0x7fc00000, 0x3f800000, 1),
        (0x2d, 0x3f800000, 0x40000000, 0xffffffff),
        (0x2e, 0x40000000, 0x3f800000, 1),
        (0x2d, 0x3f800000, 0x3f800000, 0),
        (0x2f, 0x3ff0000000000000, 0x7ff8000000000000, 0xffffffff),
        (0x30, 0x3ff0000000000000, 0x7ff8000000000000, 1),
        (0x30, 0x8000000000000000, 0, 0),
        (0x31, 0x8000000000000000, 0x7fffffffffffffff, 0xffffffff),
        (0x31, 5, 5, 0),
        (0x31, 1, 0, 1),
    ];
    for (op, x, y, want) in cmps {
        assert_eq!(run_cmp(vm, *op, *x, *y), *want);
    }
    assert_eq!(unsafe { in_d2i(f64::NAN) }, 0);
    assert_eq!(unsafe { in_d2l(f64::NAN) }, 0);
    assert_eq!(unsafe { in_d2i(f64::from(i32::MAX) + 1.0) }, i32::MAX);
    assert_eq!(unsafe { in_d2l(-9_223_372_036_854_775_808.0) }, i64::MIN);
    let mut regs = [0u32; 8];
    let mut inn = frame(vm, &mut regs, 0);
    inn.nreg = 1;
    assert_eq!(unsafe { in_get(&mut inn, 3) }, 0);
    assert_eq!(inn.bad, 1);
    shut(vm, base);
}

#[test]
fn flottants_oracle_gcc() {
    let xb: [u64; 21] = [
        0x4016000000000000,
        0xc016000000000000,
        0x4016000000000000,
        0x7e37e43c8800759c,
        0x0000000000000001,
        0x3ff0000000000000,
        0x7fe1ccf385ebc8a0,
        0x0000000000000000,
        0x8000000000000000,
        0x3ff0000000000000,
        0x7ff0000000000000,
        0x3ff0000000000000,
        0x7ff8000000000000,
        0x4008000000000000,
        0xc008000000000000,
        0x0010000000000000,
        0x4376345785d8a000,
        0x3fb999999999999a,
        0x7fefffffffffffff,
        0x419d6f3454800000,
        0x4020000000000000,
    ];
    let yb: [u64; 21] = [
        0x4000000000000000,
        0x4000000000000000,
        0xc000000000000000,
        0x4008000000000000,
        0x4008000000000000,
        0x0000000000000001,
        0x000730d67819e8d2,
        0x3ff0000000000000,
        0x3ff0000000000000,
        0x0000000000000000,
        0x3ff0000000000000,
        0x7ff0000000000000,
        0x3ff0000000000000,
        0x4008000000000000,
        0x4008000000000000,
        0x0000000000000003,
        0x401c000000000000,
        0x3f9eb851eb851eb8,
        0x0000000000000001,
        0x3f50624dd2f1a9fc,
        0x000012688b70e62b,
    ];
    let dmod: [u64; 21] = [
        0x3ff8000000000000,
        0xbff8000000000000,
        0x3ff8000000000000,
        0,
        0x0000000000000001,
        0,
        0x00028401cf53d610,
        0,
        0x8000000000000000,
        0x7ff8000000000000,
        0x7ff8000000000000,
        0x3ff0000000000000,
        0x7ff8000000000000,
        0,
        0x8000000000000000,
        0x0000000000000001,
        0x4014000000000000,
        0x3f847ae147ae1480,
        0,
        0x3f50624b1084c210,
        0x00000ed60dc315aa,
    ];
    let fmodb: [u32; 21] = [
        0x3fc00000, 0xbfc00000, 0x3fc00000, 0x7fc00000, 0, 0x7fc00000, 0x7fc00000, 0, 0x80000000,
        0x7fc00000, 0x7fc00000, 0x3f800000, 0x7fc00000, 0, 0x80000000, 0x7fc00000, 0, 0x3c23d70e,
        0x7fc00000, 0x38f60580, 0x7fc00000,
    ];
    for i in 0..21 {
        let x = f64::from_bits(xb[i]);
        let y = f64::from_bits(yb[i]);
        assert_eq!(unsafe { in_fmod(x, y) }.to_bits(), dmod[i]);
        let fr = unsafe { in_fmod(f64::from(x as f32), f64::from(y as f32)) } as f32;
        assert_eq!(fr.to_bits(), fmodb[i]);
    }
    let base = live();
    let vm = open();
    let mut regs = [0u32; 8];
    put64(&mut regs, 2, 0x4016000000000000);
    put64(&mut regs, 4, 0x4000000000000000);
    let mut inn = frame(vm, &mut regs, 0xab);
    inn.i.a = 0;
    inn.i.b = 2;
    inn.i.c = 4;
    assert_eq!(unsafe { in_op_dbin(&mut inn) }, 0);
    assert_eq!(get64(&regs, 0), 0x401e000000000000);
    inn.i.op = 0xcf;
    put64(&mut regs, 0, 0x4016000000000000);
    put64(&mut regs, 2, 0x4000000000000000);
    inn.i.a = 0;
    inn.i.b = 2;
    assert_eq!(unsafe { in_op_dbin(&mut inn) }, 0);
    assert_eq!(get64(&regs, 0), 0x3ff8000000000000);
    let mut regs = [0u32; 8];
    regs[1] = 0x40000000;
    regs[2] = 0x40400000;
    let mut inn = frame(vm, &mut regs, 0xc8);
    inn.i.a = 1;
    inn.i.b = 2;
    assert_eq!(unsafe { in_op_fbin(&mut inn) }, 0);
    assert_eq!(regs[1], 0x40c00000);
    regs[0] = 0x40b00000;
    regs[1] = 0x40000000;
    inn.i.op = 0xca;
    inn.i.a = 0;
    inn.i.b = 1;
    assert_eq!(unsafe { in_op_fbin(&mut inn) }, 0);
    assert_eq!(regs[0], 0x3fc00000);
    shut(vm, base);
}
