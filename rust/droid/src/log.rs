use core::ffi::c_int;

use crate::abi::{
    DROID_LOG_DEBUG, DROID_LOG_ERROR, DROID_LOG_INFO, DROID_LOG_WARN, DROID_TEXT_MAX, arg, strlen,
    t_dvm,
};
use crate::text::dr_cstr;
use crate::util::dr_host;

unsafe fn log_at(vm: *mut t_dvm, args: *const u32, ret: *mut u64, lv: u32) -> c_int {
    let mut tag = [0u8; DROID_TEXT_MAX];
    let mut msg = [0u8; DROID_TEXT_MAX];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let d = unsafe { dr_host(vm) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let mut rc = unsafe { dr_cstr(vm, arg(args, 0), tag.as_mut_ptr().cast()) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { dr_cstr(vm, arg(args, 1), msg.as_mut_ptr().cast()) };
    }
    if rc != 0 {
        return rc;
    }
    if !d.is_null() {
        // SAFETY: d n'est pas nul.
        if let Some(log) = unsafe { (*d).log } {
            // SAFETY: log est le rappel enregistré. tag et msg sont des chaînes C.
            unsafe { log((*d).user, lv, tag.as_ptr().cast(), msg.as_ptr().cast()) };
        }
    }
    // SAFETY: msg est terminé par NUL, ret n'est pas nul.
    let n = unsafe { strlen(msg.as_ptr().cast()) } as u32;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { ret.write(u64::from(n)) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_log_d(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { log_at(vm, args, ret, DROID_LOG_DEBUG) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_log_i(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { log_at(vm, args, ret, DROID_LOG_INFO) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_log_w(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { log_at(vm, args, ret, DROID_LOG_WARN) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_log_e(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { log_at(vm, args, ret, DROID_LOG_ERROR) }
}
