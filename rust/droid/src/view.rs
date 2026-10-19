use core::ffi::{c_int, c_void};

use crate::abi::{
    DROID_VIEWS_MAX, DV_BUTTON, DV_LINEAR, DV_TEXT, DV_VIEW, DVM_NULL, E_INVAL, arg, dvm_payload,
    dvm_pin, dvm_unpin, memset, t_droidview, t_dvm,
};
use crate::util::{
    BTN, CCE, ISE, LL, OCL, OOME, TV, VIEW, dr_class, dr_host, dr_is, dr_npe, throw_at,
};

unsafe fn payload_id(vm: *mut t_dvm, r: u32) -> *mut u32 {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let c = unsafe { dr_class(vm, VIEW.as_ptr()) };
    // SAFETY: vm n'est pas nul. c est nul ou la classe View.
    unsafe { dvm_payload(vm, r, c).cast() }
}

/// # Safety
/// `vm` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_view_of(vm: *mut t_dvm, r: u32) -> *mut t_droidview {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let d = unsafe { dr_host(vm) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let idp = unsafe { payload_id(vm, r) };
    if d.is_null() || idp.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: idp n'est pas nul.
    let id = unsafe { idp.read() };
    // SAFETY: d n'est pas nul.
    let nviews = unsafe { (*d).nviews };
    if id == 0 || id > nviews || id > DROID_VIEWS_MAX {
        return core::ptr::null_mut();
    }
    // SAFETY: id est dans 1..=min(nviews, DROID_VIEWS_MAX).
    if unsafe { (*d).views[(id - 1) as usize].self_ref } != r {
        return core::ptr::null_mut();
    }
    // SAFETY: id est dans 1..=nviews et nviews ne dépasse pas le tableau quand l'init a réussi.
    unsafe { &raw mut (*d).views[(id - 1) as usize] }
}

/// # Safety
/// `vm` n'est pas nul. `out` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_view_get(vm: *mut t_dvm, r: u32, out: *mut *mut t_droidview) -> c_int {
    if r == DVM_NULL {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let v = unsafe { dr_view_of(vm, r) };
    // SAFETY: out n'est pas nul.
    unsafe { out.write(v) };
    if v.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"vue construite attendue") };
    }
    0
}

unsafe fn kind_of(vm: *mut t_dvm, r: u32) -> u32 {
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_is(vm, r, BTN.as_ptr()) } != 0 {
        return DV_BUTTON;
    }
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_is(vm, r, TV.as_ptr()) } != 0 {
        return DV_TEXT;
    }
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_is(vm, r, LL.as_ptr()) } != 0 {
        return DV_LINEAR;
    }
    DV_VIEW
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_view_init(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let r = unsafe { arg(args, 0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let d = unsafe { dr_host(vm) };
    if r == DVM_NULL || d.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let idp = unsafe { payload_id(vm, r) };
    if idp.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"vue attendue") };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if !unsafe { dr_view_of(vm, r) }.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, ISE, c"vue déjà construite") };
    }
    // SAFETY: d n'est pas nul.
    if unsafe { (*d).nviews } >= DROID_VIEWS_MAX {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, OOME, c"trop de vues") };
    }
    // SAFETY: vm n'est pas nul.
    if unsafe { dvm_pin(vm, r) } != 0 {
        return E_INVAL;
    }
    // SAFETY: d n'est pas nul et nviews < DROID_VIEWS_MAX.
    let slot = unsafe { (*d).nviews };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let v = unsafe { &raw mut (*d).views[slot as usize] };
    // SAFETY: v pointe une fiche du tableau, de la taille d'une vue.
    unsafe { memset(v.cast::<c_void>(), 0, core::mem::size_of::<t_droidview>()) };
    // SAFETY: d n'est pas nul. L'incrément reste <= DROID_VIEWS_MAX.
    unsafe { (*d).nviews = slot + 1 };
    // SAFETY: v vient d'être mis à zéro et reste exclusif.
    unsafe {
        (*v).kind = kind_of(vm, r);
        (*v).id = (*d).nviews;
        (*v).self_ref = r;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let id = unsafe { (*v).id };
    // SAFETY: idp n'est pas nul.
    unsafe { idp.write(id) };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_view_listen(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    let mut v = core::ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_view_get(vm, arg(args, 0), &mut v) };
    if rc != 0 {
        return rc;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let ear = unsafe { arg(args, 1) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if ear != DVM_NULL && unsafe { dr_is(vm, ear, OCL.as_ptr()) } == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"écouteur attendu") };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if ear != DVM_NULL && unsafe { dvm_pin(vm, ear) } != 0 {
        return E_INVAL;
    }
    // SAFETY: v vient de dr_view_get et n'est pas nul.
    let old = unsafe { (*v).listener };
    // SAFETY: vm n'est pas nul.
    unsafe { dvm_unpin(vm, old) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*v).listener = ear };
    0
}
