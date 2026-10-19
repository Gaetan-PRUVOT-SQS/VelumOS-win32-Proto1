use core::ffi::c_char;

use crate::abi::{dvm_class, dvm_is_instance, dvm_throw, t_dclass, t_dvm};

pub(crate) const NPE: &core::ffi::CStr = c"Ljava/lang/NullPointerException;";
pub(crate) const CCE: &core::ffi::CStr = c"Ljava/lang/ClassCastException;";
pub(crate) const IAE: &core::ffi::CStr = c"Ljava/lang/IllegalArgumentException;";
pub(crate) const ISE: &core::ffi::CStr = c"Ljava/lang/IllegalStateException;";
pub(crate) const AIOOBE: &core::ffi::CStr = c"Ljava/lang/ArrayIndexOutOfBoundsException;";
pub(crate) const SIOOBE: &core::ffi::CStr = c"Ljava/lang/StringIndexOutOfBoundsException;";
pub(crate) const ASE: &core::ffi::CStr = c"Ljava/lang/ArrayStoreException;";
pub(crate) const NFE: &core::ffi::CStr = c"Ljava/lang/NumberFormatException;";
pub(crate) const OOME: &core::ffi::CStr = c"Ljava/lang/OutOfMemoryError;";
pub(crate) const NSME: &core::ffi::CStr = c"Ljava/lang/NoSuchMethodError;";
pub(crate) const OBJ: &core::ffi::CStr = c"Ljava/lang/Object;";
pub(crate) const STR: &core::ffi::CStr = c"Ljava/lang/String;";
pub(crate) const CSQ: &core::ffi::CStr = c"Ljava/lang/CharSequence;";
pub(crate) const SB: &core::ffi::CStr = c"Ljava/lang/StringBuilder;";
pub(crate) const INT: &core::ffi::CStr = c"Ljava/lang/Integer;";
pub(crate) const MATH: &core::ffi::CStr = c"Ljava/lang/Math;";
pub(crate) const SYS: &core::ffi::CStr = c"Ljava/lang/System;";
pub(crate) const THR: &core::ffi::CStr = c"Ljava/lang/Throwable;";
pub(crate) const EXC: &core::ffi::CStr = c"Ljava/lang/Exception;";
pub(crate) const RTE: &core::ffi::CStr = c"Ljava/lang/RuntimeException;";
pub(crate) const ERR: &core::ffi::CStr = c"Ljava/lang/Error;";
pub(crate) const IOOBE: &core::ffi::CStr = c"Ljava/lang/IndexOutOfBoundsException;";
pub(crate) const CTX: &core::ffi::CStr = c"Landroid/content/Context;";
pub(crate) const BUNDLE: &core::ffi::CStr = c"Landroid/os/Bundle;";
pub(crate) const ACT: &core::ffi::CStr = c"Landroid/app/Activity;";
pub(crate) const LOG: &core::ffi::CStr = c"Landroid/util/Log;";
pub(crate) const VIEW: &core::ffi::CStr = c"Landroid/view/View;";
pub(crate) const OCL: &core::ffi::CStr = c"Landroid/view/View$OnClickListener;";
pub(crate) const VG: &core::ffi::CStr = c"Landroid/view/ViewGroup;";
pub(crate) const TV: &core::ffi::CStr = c"Landroid/widget/TextView;";
pub(crate) const BTN: &core::ffi::CStr = c"Landroid/widget/Button;";
pub(crate) const LL: &core::ffi::CStr = c"Landroid/widget/LinearLayout;";
pub(crate) const LOGSIG: &core::ffi::CStr = c"(Ljava/lang/String;Ljava/lang/String;)I";
pub(crate) const CTXSIG: &core::ffi::CStr = c"(Landroid/content/Context;)V";

/// # Safety
/// `vm` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_host(vm: *mut t_dvm) -> *mut crate::abi::t_droid {
    // SAFETY: vm n'est pas nul et host a la taille d'un pointeur.
    unsafe { (*vm).host.cast() }
}

/// # Safety
/// `vm` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_npe(vm: *mut t_dvm) -> core::ffi::c_int {
    // SAFETY: vm n'est pas nul. Le descripteur est un littéral C.
    unsafe { dvm_throw(vm, NPE.as_ptr(), core::ptr::null()) }
}

/// # Safety
/// `vm` n'est pas nul. `desc` est nul ou une chaîne C.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_class(vm: *mut t_dvm, desc: *const c_char) -> *mut t_dclass {
    let mut c = core::ptr::null_mut();
    if desc.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: vm n'est pas nul, desc est une chaîne C, c est exclusif.
    if unsafe { dvm_class(vm, desc, &mut c) } != 0 {
        return core::ptr::null_mut();
    }
    c
}

/// # Safety
/// `vm` n'est pas nul. `desc` est une chaîne C.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_is(vm: *mut t_dvm, r: u32, desc: *const c_char) -> core::ffi::c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let c = unsafe { dr_class(vm, desc) };
    // SAFETY: vm n'est pas nul. c est nul ou une classe rendue par dvm_class.
    unsafe { dvm_is_instance(vm, r, c) }
}

pub(crate) unsafe fn throw_at(
    vm: *mut t_dvm,
    desc: &core::ffi::CStr,
    msg: &core::ffi::CStr,
) -> core::ffi::c_int {
    // SAFETY: vm n'est pas nul. desc et msg sont des littéraux C.
    unsafe { dvm_throw(vm, desc.as_ptr(), msg.as_ptr()) }
}
