use core::ffi::{c_char, c_int};

use crate::abi::{
    DR_SB_CAP, DVM_NULL, arg, dvm_payload, dvm_string_get, memmove, t_drsb, t_dstr16, t_dvm,
};
use crate::text::{dr_new16, dr_text_of};
use crate::utf::dr_dec16;
use crate::util::{CCE, OOME, SB, dr_class, dr_npe, throw_at};

fn sb_of(vm: *mut t_dvm, r: u32) -> *mut t_drsb {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let c = unsafe { dr_class(vm, SB.as_ptr()) };
    // SAFETY: vm n'est pas nul. c est nul ou la classe StringBuilder.
    unsafe { dvm_payload(vm, r, c).cast() }
}

/// # Safety
/// `vm` n'est pas nul. `s.p` est nul seulement si `s.n` vaut 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_put(vm: *mut t_dvm, r: u32, s: t_dstr16) -> c_int {
    if r == DVM_NULL {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    let sb = sb_of(vm, r);
    if sb.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"StringBuilder attendu") };
    }
    // SAFETY: sb vient de dvm_payload et n'est pas nul.
    let len = unsafe { (*sb).len };
    if len > DR_SB_CAP || s.n > DR_SB_CAP - len {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, OOME, c"StringBuilder plein") };
    }
    if s.n > 0 {
        // SAFETY: len + s.n <= DR_SB_CAP, les deux plages tiennent. memmove accepte le recouvrement.
        unsafe {
            memmove(
                (*sb).buf.as_mut_ptr().add(len as usize).cast(),
                s.p.cast(),
                (s.n as usize) * 2,
            );
        }
        // SAFETY: sb n'est pas nul.
        unsafe { (*sb).len = len + s.n };
    }
    0
}

/// # Safety
/// `vm` n'est pas nul. `txt` est une chaîne C.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_ascii(vm: *mut t_dvm, r: u32, txt: *const c_char) -> c_int {
    let mut buf = [0u16; 24];
    let mut s = t_dstr16 {
        p: buf.as_ptr(),
        n: 0,
    };
    while s.n < 24 {
        // SAFETY: txt est une chaîne C, s.n < 24 et on s'arrête au NUL.
        let ch = unsafe { txt.add(s.n as usize).read() };
        if ch == 0 {
            break;
        }
        buf[s.n as usize] = ch as u8 as u16;
        s.n += 1;
    }
    s.p = buf.as_ptr();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_sb_put(vm, r, s) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_length(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let sb = sb_of(
        vm,
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { arg(args, 0) },
    );
    if sb.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: sb n'est pas nul, ret n'est pas nul.
    unsafe { ret.write(u64::from((*sb).len)) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_tostring(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let sb = sb_of(
        vm,
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { arg(args, 0) },
    );
    if sb.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: sb n'est pas nul.
    let mut n = unsafe { (*sb).len };
    if n > DR_SB_CAP {
        n = DR_SB_CAP;
    }
    let s = t_dstr16 {
        // SAFETY: sb n'est pas nul, buf vit le temps de dr_new16 qui copie.
        p: unsafe { (*sb).buf.as_ptr() },
        n,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_new16(vm, s, ret) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_append_obj(
    vm: *mut t_dvm,
    args: *const u32,
    ret: *mut u64,
) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let recv = unsafe { arg(args, 0) };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(recv)) };
    let mut str_ref = DVM_NULL;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_text_of(vm, arg(args, 1), &mut str_ref) };
    if rc != 0 {
        return rc;
    }
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: vm n'est pas nul.
    if str_ref == DVM_NULL || unsafe { dvm_string_get(vm, str_ref, &mut s) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_sb_ascii(vm, recv, c"null".as_ptr()) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_sb_put(vm, recv, s) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_append_i(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let recv = unsafe { arg(args, 0) };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(recv)) };
    let mut buf = [0u16; 24];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_dec16(arg(args, 1) as i32 as i64, buf.as_mut_ptr()) };
    let s = t_dstr16 { p: buf.as_ptr(), n };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_sb_put(vm, recv, s) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins trois mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_append_j(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let recv = unsafe { arg(args, 0) };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(recv)) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let lo = unsafe { arg(args, 1) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let hi = unsafe { arg(args, 2) };
    let v = (u64::from(lo) | (u64::from(hi) << 32)) as i64;
    let mut buf = [0u16; 24];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_dec16(v, buf.as_mut_ptr()) };
    let s = t_dstr16 { p: buf.as_ptr(), n };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_sb_put(vm, recv, s) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_append_c(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let recv = unsafe { arg(args, 0) };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(recv)) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let c = unsafe { arg(args, 1) } as u16;
    let s = t_dstr16 { p: &c, n: 1 };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_sb_put(vm, recv, s) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sb_append_z(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let recv = unsafe { arg(args, 0) };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(recv)) };
    let word = if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { arg(args, 1) } != 0 {
        c"true".as_ptr()
    } else {
        c"false".as_ptr()
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_sb_ascii(vm, recv, word) }
}
