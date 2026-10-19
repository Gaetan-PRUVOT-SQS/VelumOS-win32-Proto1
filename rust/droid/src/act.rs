use core::ffi::c_int;

use crate::abi::{DROID_TEXT_MAX, DVM_NULL, arg, dvm_string_get, t_dstr16, t_dvm};
use crate::text::dr_text_of;
use crate::utf::dr_utf8;
use crate::util::{ACT, ISE, dr_host, dr_is, dr_npe, throw_at};

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_act_content(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_host(vm) }.is_null() || unsafe { dr_is(vm, arg(args, 0), ACT.as_ptr()) } == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    let mut v = core::ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { crate::view::dr_view_get(vm, arg(args, 1), &mut v) };
    if rc != 0 {
        return rc;
    }
    // SAFETY: v n'est pas nul.
    if unsafe { (*v).parent } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, ISE, c"vue déjà placée") };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let id = unsafe { (*v).id };
    // SAFETY: dr_host vient d'être vérifié non nul.
    let d = unsafe { dr_host(vm) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe {
        (*d).root = id;
        (*d).dirty = 1;
    }
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_act_title(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let d = unsafe { dr_host(vm) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if d.is_null() || unsafe { dr_is(vm, arg(args, 0), ACT.as_ptr()) } == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    let mut str_ref = DVM_NULL;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_text_of(vm, arg(args, 1), &mut str_ref) };
    if rc != 0 {
        return rc;
    }
    // SAFETY: d n'est pas nul, title a DROID_TEXT_MAX octets.
    unsafe { (*d).title[0] = 0 };
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if str_ref != DVM_NULL && unsafe { dvm_string_get(vm, str_ref, &mut s) } == 0 {
        // SAFETY: title a DROID_TEXT_MAX octets.
        unsafe { dr_utf8(s, (*d).title.as_mut_ptr().cast(), DROID_TEXT_MAX) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*d).dirty = 1 };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_act_finish(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_host(vm) }.is_null() || unsafe { dr_is(vm, arg(args, 0), ACT.as_ptr()) } == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: l'hôte vient d'être vérifié.
    unsafe { (*dr_host(vm)).finished = 1 };
    0
}
