use core::ffi::{c_char, c_int};

use crate::abi::{
    DROID_TEXT_MAX, DVM_NULL, dvm_call, dvm_class_of, dvm_method, dvm_string_get,
    dvm_string_utf8_new, dvm_string_utf16_new, t_dname, t_dstr16, t_dvm,
};
use crate::utf::dr_utf8;
use crate::util::{CCE, dr_npe, throw_at};

/// # Safety
/// `vm` n'est pas nul. `s.p` est nul seulement si `s.n` vaut 0. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_new16(vm: *mut t_dvm, s: t_dstr16, ret: *mut u64) -> c_int {
    let mut r = DVM_NULL;
    // SAFETY: vm n'est pas nul, ret de la VM est exclusif.
    let rc = unsafe { dvm_string_utf16_new(vm, s, &mut r) };
    if rc == 0 {
        // SAFETY: ret n'est pas nul et il est exclusif.
        unsafe { ret.write(u64::from(r)) };
    }
    rc
}

/// # Safety
/// `vm` n'est pas nul. `s` est une chaîne C. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_new8(vm: *mut t_dvm, s: *const c_char, ret: *mut u64) -> c_int {
    let mut r = DVM_NULL;
    // SAFETY: vm n'est pas nul, s est une chaîne C.
    let rc = unsafe { dvm_string_utf8_new(vm, s, &mut r) };
    if rc == 0 {
        // SAFETY: ret n'est pas nul.
        unsafe { ret.write(u64::from(r)) };
    }
    rc
}

/// # Safety
/// `vm` n'est pas nul. `out` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_str(vm: *mut t_dvm, r: u32, out: *mut t_dstr16) -> c_int {
    if r == DVM_NULL {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: vm et out ne sont pas nuls.
    if unsafe { dvm_string_get(vm, r, out) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"chaîne attendue") };
    }
    0
}

/// # Safety
/// `vm` n'est pas nul. `out` pointe `DROID_TEXT_MAX` octets exclusifs.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_cstr(vm: *mut t_dvm, r: u32, out: *mut c_char) -> c_int {
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_str(vm, r, &mut s) };
    if rc == 0 {
        // SAFETY: out a DROID_TEXT_MAX octets, s est une chaîne UTF-16 valide.
        unsafe { dr_utf8(s, out, DROID_TEXT_MAX) };
    }
    rc
}

/// # Safety
/// `vm` n'est pas nul. `str_out` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_text_of(vm: *mut t_dvm, r: u32, str_out: *mut u32) -> c_int {
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: str_out n'est pas nul.
    unsafe { str_out.write(r) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if r == DVM_NULL || unsafe { dvm_string_get(vm, r, &mut s) } == 0 {
        return 0;
    }
    let nm = t_dname {
        name: c"toString".as_ptr(),
        sig: c"()Ljava/lang/String;".as_ptr(),
    };
    // SAFETY: str_out n'est pas nul.
    unsafe { str_out.write(DVM_NULL) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let cls = unsafe { dvm_class_of(vm, r) };
    let mut m = core::ptr::null();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if cls.is_null() || unsafe { dvm_method(vm, cls, &nm, &mut m) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"toString absent") };
    }
    let mut ret = 0u64;
    // SAFETY: m vient de dvm_method, r est l'unique argument, ret est exclusif.
    let rc = unsafe { dvm_call(vm, m, &r, &mut ret) };
    if rc != 0 {
        return rc;
    }
    let got = ret as u32;
    // SAFETY: str_out n'est pas nul.
    unsafe { str_out.write(got) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if got != DVM_NULL && unsafe { dvm_string_get(vm, got, &mut s) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"toString ne rend pas une chaîne") };
    }
    0
}
