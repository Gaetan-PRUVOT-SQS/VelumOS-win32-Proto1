use core::ffi::c_int;

use crate::abi::{arg, t_dvm};
use crate::util::dr_host;

/// # Safety
/// `args` pointe au moins un mot. `ret` n'est pas nul. `vm` n'est pas lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_math_abs(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = vm;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let v = unsafe { arg(args, 0) };
    let mut out = u64::from(v);
    if (v as i32) < 0 {
        out = u64::from(v.wrapping_neg());
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(out) };
    0
}

/// # Safety
/// `args` pointe au moins deux mots. `ret` n'est pas nul. `vm` n'est pas lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_math_min(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = vm;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let a = unsafe { arg(args, 0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let b = unsafe { arg(args, 1) };
    let mut out = a;
    if (b as i32) < (a as i32) {
        out = b;
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(out)) };
    0
}

/// # Safety
/// `args` pointe au moins deux mots. `ret` n'est pas nul. `vm` n'est pas lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_math_max(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = vm;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let a = unsafe { arg(args, 0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let b = unsafe { arg(args, 1) };
    let mut out = a;
    if (b as i32) > (a as i32) {
        out = b;
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(out)) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `ret` n'est pas nul. `args` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sys_millis(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = args;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let d = unsafe { dr_host(vm) };
    let mut out = 0u64;
    if !d.is_null() {
        // SAFETY: d n'est pas nul.
        if let Some(clock) = unsafe { (*d).clock } {
            // SAFETY: clock est le rappel enregistré, user est celui qui l'accompagne.
            out = unsafe { clock((*d).user) };
        }
    }
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(out) };
    0
}
