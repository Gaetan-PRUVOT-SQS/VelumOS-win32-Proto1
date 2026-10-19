use core::ffi::c_int;

use crate::abi::{
    DVM_NULL, DVM_THROWN, E_NOMEM, arg, dvm_string_get, free, malloc, memcpy, t_dstr16, t_dvm,
};
use crate::text::{dr_new16, dr_str};
use crate::utf::dr_dec16;
use crate::util::{NFE, OOME, SIOOBE, throw_at};

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_str_length(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_str(vm, arg(args, 0), &mut s) };
    if rc == 0 {
        // SAFETY: ret n'est pas nul.
        unsafe { ret.write(u64::from(s.n)) };
    }
    rc
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_str_charat(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_str(vm, arg(args, 0), &mut s) };
    if rc != 0 {
        return rc;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let i = unsafe { arg(args, 1) };
    if i >= s.n {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, SIOOBE, c"indice hors de la chaîne") };
    }
    // SAFETY: i < s.n et s.p pointe s.n unités.
    let ch = unsafe { s.p.add(i as usize).read() };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(ch)) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_str_equals(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut a = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    let mut b = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_str(vm, arg(args, 0), &mut a) };
    if rc != 0 {
        return rc;
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let other = unsafe { arg(args, 1) };
    // SAFETY: vm n'est pas nul.
    if unsafe { dvm_string_get(vm, other, &mut b) } != 0 || a.n != b.n {
        return 0;
    }
    let mut i = 0u32;
    while i < a.n {
        // SAFETY: i < a.n == b.n, les deux tampons couvrent n unités.
        let (ca, cb) = unsafe { (a.p.add(i as usize).read(), b.p.add(i as usize).read()) };
        if ca != cb {
            break;
        }
        i += 1;
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(i == a.n)) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_str_hash(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_str(vm, arg(args, 0), &mut s) };
    if rc != 0 {
        return rc;
    }
    let mut h = 0u32;
    let mut i = 0u32;
    while i < s.n {
        // SAFETY: i < s.n, s.p couvre ces unités.
        let ch = unsafe { s.p.add(i as usize).read() };
        h = h.wrapping_mul(31).wrapping_add(u32::from(ch));
        i += 1;
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(h)) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_str_self(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let id = unsafe { arg(args, 0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_str(vm, id, &mut s) };
    if rc == 0 {
        // SAFETY: ret n'est pas nul.
        unsafe { ret.write(u64::from(id)) };
    }
    rc
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_int_tostring(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut buf = [0u16; 24];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let v = unsafe { arg(args, 0) } as i32 as i64;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_dec16(v, buf.as_mut_ptr()) };
    let s = t_dstr16 { p: buf.as_ptr(), n };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_new16(vm, s, ret) }
}

fn digits(s: t_dstr16, mut i: u32, v: &mut i64) -> c_int {
    if i >= s.n {
        return -1;
    }
    while i < s.n {
        // SAFETY: i < s.n.
        let ch = unsafe { s.p.add(i as usize).read() };
        if !(u16::from(b'0')..=u16::from(b'9')).contains(&ch) {
            return -1;
        }
        *v = *v * 10 + i64::from(ch - u16::from(b'0'));
        if *v > 2_147_483_648 {
            return -1;
        }
        i += 1;
    }
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_int_parse(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let id = unsafe { arg(args, 0) };
    if id == DVM_NULL {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, NFE, c"null") };
    }
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_str(vm, id, &mut s) } != 0 {
        return DVM_THROWN;
    }
    let mut v = 0i64;
    let first = if s.n == 0 {
        0
    } else {
        // SAFETY: s.n > 0, s.p pointe au moins une unité.
        unsafe { s.p.read() }
    };
    let neg = first == u16::from(b'-');
    let plus = first == u16::from(b'+');
    let start = u32::from(neg || plus);
    if digits(s, start, &mut v) != 0 || (!neg && v > 2_147_483_647) {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, NFE, c"entier invalide") };
    }
    if neg {
        v = -v;
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(v as u32)) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_str_concat(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut a = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    let mut b = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let mut rc = unsafe { dr_str(vm, arg(args, 0), &mut a) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { dr_str(vm, arg(args, 1), &mut b) };
    }
    if rc != 0 {
        return rc;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let heap = unsafe { (*vm).lim.heap_bytes };
    let units = u64::from(a.n) + u64::from(b.n);
    if units.wrapping_mul(2) > u64::from(heap) {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, OOME, c"chaîne trop longue") };
    }
    let bytes = ((a.n as usize) + (b.n as usize) + 1).wrapping_mul(2);
    // SAFETY: malloc est l'allocateur C. bytes est la taille demandée.
    let tmp = unsafe { malloc(bytes) }.cast::<u16>();
    if tmp.is_null() {
        return E_NOMEM;
    }
    if a.n > 0 {
        // SAFETY: tmp a la place, a.p couvre a.n unités.
        unsafe { memcpy(tmp.cast(), a.p.cast(), (a.n as usize) * 2) };
    }
    if b.n > 0 {
        // SAFETY: tmp + a.n a la place, b.p couvre b.n unités.
        unsafe { memcpy(tmp.add(a.n as usize).cast(), b.p.cast(), (b.n as usize) * 2) };
    }
    a.p = tmp;
    a.n += b.n;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    rc = unsafe { dr_new16(vm, a, ret) };
    // SAFETY: tmp vient de malloc, une seule fois.
    unsafe { free(tmp.cast()) };
    rc
}
