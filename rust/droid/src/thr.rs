use core::ffi::c_int;

use crate::abi::{
    DVM_NULL, arg, dvm_string_get, dvm_throwable_message, dvm_throwable_set_message, t_dstr16,
    t_dvm,
};
use crate::util::{CCE, THR, dr_is, dr_npe, throw_at};

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_thr_init_msg(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let recv = unsafe { arg(args, 0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let msg = unsafe { arg(args, 1) };
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_is(vm, recv, THR.as_ptr()) } == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if msg != DVM_NULL && unsafe { dvm_string_get(vm, msg, &mut s) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"message") };
    }
    // SAFETY: vm n'est pas nul.
    if unsafe { dvm_throwable_set_message(vm, recv, msg) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_thr_message(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut msg = DVM_NULL;
    // SAFETY: vm n'est pas nul, msg est exclusif.
    if unsafe { dvm_throwable_message(vm, arg(args, 0), &mut msg) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(msg)) };
    0
}
