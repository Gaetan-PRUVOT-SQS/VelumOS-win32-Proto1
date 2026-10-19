use core::ffi::{c_char, c_int, c_void};

use crate::abi::{
    ACC_CONSTRUCTOR, ACC_PUBLIC, DROID_VIEWS_MAX, DVM_NULL, E_INVAL, E_NOMEM, c_eq, c_strlen,
    dvm_builtins, dvm_call, dvm_class, dvm_class_of, dvm_method, dvm_natives, dvm_new, dvm_pin,
    dvm_throw, dvm_unpin, free, malloc, memset, t_dbuiltin, t_dname, t_dnative, t_droid,
    t_droidview, t_dvm,
};
use crate::fail::{dr_fail, dr_missing};
use crate::object::dr_nop;
use crate::tab::{dr_classes, dr_tab_android, dr_tab_lang};
use crate::thr::dr_thr_init_msg;
use crate::util::{ACT, CCE, dr_is};

fn is_exc(desc: *const c_char) -> bool {
    // SAFETY: desc est un descripteur C de la table des classes.
    let n = unsafe { c_strlen(desc) };
    if n > 10 {
        // SAFETY: n > 10, le suffixe reste dans la chaîne C.
        if unsafe { c_eq(desc.add(n - 10), c"Exception;".as_ptr()) } {
            return true;
        }
    }
    if n > 6 {
        // SAFETY: n > 6, le suffixe reste dans la chaîne C.
        return unsafe { c_eq(desc.add(n - 6), c"Error;".as_ptr()) };
    }
    false
}

unsafe fn fill_ctor(slot: *mut t_dnative, cls: *const c_char, with_msg: bool) {
    // SAFETY: slot pointe une entrée allouée, cls est un descripteur statique.
    unsafe {
        (*slot).cls = cls;
        (*slot).name = c"<init>".as_ptr();
        (*slot).sig = if with_msg {
            c"(Ljava/lang/String;)V".as_ptr()
        } else {
            c"()V".as_ptr()
        };
        (*slot).access = ACC_PUBLIC | ACC_CONSTRUCTOR;
        (*slot).r#fn = Some(if with_msg { dr_thr_init_msg } else { dr_nop });
    }
}

unsafe fn ctors(vm: *mut t_dvm, tab: *const t_dbuiltin, n: u32) -> c_int {
    let bytes = core::mem::size_of::<t_dnative>()
        .wrapping_mul(2)
        .wrapping_mul(n as usize);
    // SAFETY: malloc est l'allocateur C.
    let nat = unsafe { malloc(bytes) }.cast::<t_dnative>();
    if nat.is_null() {
        return E_NOMEM;
    }
    let mut i = 0u32;
    let mut k = 0usize;
    while i < n {
        // SAFETY: i < n, tab pointe n classes.
        let desc = unsafe { (*tab.add(i as usize)).desc };
        if is_exc(desc) {
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            unsafe { fill_ctor(nat.add(k), desc, false) };
            k += 1;
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            unsafe { fill_ctor(nat.add(k), desc, true) };
            k += 1;
        }
        i += 1;
    }
    // SAFETY: nat pointe k entrées initialisées, vm n'est pas nul.
    let rc = unsafe { dvm_natives(vm, nat, k as u32) };
    // SAFETY: nat vient de malloc.
    unsafe { free(nat.cast()) };
    rc
}

unsafe fn reset(d: *mut t_droid, vm: *mut t_dvm) {
    // SAFETY: d et vm ne sont pas nuls.
    let log = unsafe { (*d).log };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let clock = unsafe { (*d).clock };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let user = unsafe { (*d).user };
    // SAFETY: d pointe une structure t_droid exclusive.
    unsafe { memset(d.cast::<c_void>(), 0, core::mem::size_of::<t_droid>()) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe {
        (*d).log = log;
        (*d).clock = clock;
        (*d).user = user;
        (*d).vm = vm;
        (*vm).host = d.cast();
    }
}

/// # Safety
/// `d` et `vm` sont nuls, ou bien ils sont alignés et exclusifs. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn droid_install(d: *mut t_droid, vm: *mut t_dvm) -> c_int {
    if d.is_null() || vm.is_null() {
        return E_INVAL;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { reset(d, vm) };
    let mut cls = core::ptr::null();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_classes(&mut cls) };
    // SAFETY: vm n'est pas nul, cls pointe la table statique.
    let mut rc = unsafe { dvm_builtins(vm, cls, n) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { ctors(vm, cls, n) };
    }
    let mut nat = core::ptr::null();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let nlang = unsafe { dr_tab_lang(&mut nat) };
    if rc == 0 {
        // SAFETY: nat pointe la table statique du langage.
        rc = unsafe { dvm_natives(vm, nat, nlang) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let nand = unsafe { dr_tab_android(&mut nat) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { dvm_natives(vm, nat, nand) };
    }
    rc
}

unsafe fn invoke(
    d: *mut t_droid,
    recv: u32,
    name: &core::ffi::CStr,
    sig: &core::ffi::CStr,
    arg: u32,
) -> c_int {
    // SAFETY: d n'est pas nul et vm a été posé par droid_install.
    let vm = unsafe { (*d).vm };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let c = unsafe { dvm_class_of(vm, recv) };
    if c.is_null() {
        return E_INVAL;
    }
    let nm = t_dname {
        name: name.as_ptr(),
        sig: sig.as_ptr(),
    };
    let mut m = core::ptr::null();
    // SAFETY: c n'est pas nul, nm est exclusif.
    if unsafe { dvm_method(vm, c, &nm, &mut m) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_missing(d, c, &nm) };
    }
    let args = [recv, arg];
    let mut ret = 0u64;
    // SAFETY: m vient de dvm_method, args vit le temps de l'appel.
    let rc = unsafe { dvm_call(vm, m, args.as_ptr(), &mut ret) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_fail(d, rc) }
}

unsafe fn cycle_up(d: *mut t_droid) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let act = unsafe { (*d).activity };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let mut rc = unsafe { invoke(d, act, c"<init>", c"()V", DVM_NULL) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { invoke(d, act, c"onCreate", c"(Landroid/os/Bundle;)V", DVM_NULL) };
    }
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { invoke(d, act, c"onStart", c"()V", DVM_NULL) };
    }
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { invoke(d, act, c"onResume", c"()V", DVM_NULL) };
    }
    rc
}

/// # Safety
/// `d` est nul, ou bien il est aligné, installé, et exclusif. `activity_desc` est nul ou une chaîne C.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn droid_start(d: *mut t_droid, activity_desc: *const c_char) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if d.is_null() || unsafe { (*d).vm.is_null() } || activity_desc.is_null() {
        return E_INVAL;
    }
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*d).activity } != DVM_NULL {
        return E_INVAL;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let vm = unsafe { (*d).vm };
    let mut c = core::ptr::null_mut();
    // SAFETY: vm n'est pas nul, activity_desc est une chaîne C.
    let mut rc = unsafe { dvm_class(vm, activity_desc, &mut c) };
    let mut act = DVM_NULL;
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { dvm_new(vm, c, &mut act) };
    }
    if rc != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_fail(d, rc) };
    }
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_is(vm, act, ACT.as_ptr()) } == 0 || unsafe { dvm_pin(vm, act) } != 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        let thrown = unsafe { dvm_throw(vm, CCE.as_ptr(), activity_desc) };
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_fail(d, thrown) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*d).activity = act };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { cycle_up(d) }
}

/// # Safety
/// `d` est nul, ou bien il est aligné et exclusif.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn droid_view(d: *const t_droid, view_id: u32) -> *const t_droidview {
    if d.is_null() {
        return core::ptr::null();
    }
    // SAFETY: d n'est pas nul.
    let nviews = unsafe { (*d).nviews };
    if view_id == 0 || view_id > nviews || view_id > DROID_VIEWS_MAX {
        return core::ptr::null();
    }
    // SAFETY: view_id est dans 1..=min(nviews, DROID_VIEWS_MAX).
    unsafe { &raw const (*d).views[(view_id - 1) as usize] }
}

/// # Safety
/// `d` est nul, ou bien il est aligné et exclusif.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn droid_click(d: *mut t_droid, view_id: u32) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let v = unsafe { droid_view(d, view_id) };
    if v.is_null() || d.is_null() {
        return E_INVAL;
    }
    // SAFETY: d n'est pas nul.
    if unsafe { (*d).vm.is_null() } || unsafe { (*d).activity } == DVM_NULL {
        return E_INVAL;
    }
    // SAFETY: v n'est pas nul.
    let ear = unsafe { (*v).listener };
    if ear == DVM_NULL {
        return 0;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let self_ref = unsafe { (*v).self_ref };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { invoke(d, ear, c"onClick", c"(Landroid/view/View;)V", self_ref) }
}

/// # Safety
/// `d` n'est pas nul. `d.vm` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_release(d: *mut t_droid) {
    // SAFETY: d et vm ne sont pas nuls.
    let vm = unsafe { (*d).vm };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { (*d).nviews };
    let mut i = 0u32;
    while i < n && i < DROID_VIEWS_MAX {
        // SAFETY: i < nviews et i < DROID_VIEWS_MAX.
        let view = unsafe { &(*d).views[i as usize] };
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dvm_unpin(vm, view.self_ref) };
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dvm_unpin(vm, view.listener) };
        i += 1;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let act = unsafe { (*d).activity };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dvm_unpin(vm, act) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe {
        (*d).activity = DVM_NULL;
        (*d).nviews = 0;
        (*d).root = 0;
        (*d).dirty = 1;
    }
}

/// # Safety
/// `d` est nul, ou bien il est aligné, installé, et exclusif.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn droid_stop(d: *mut t_droid) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if d.is_null() || unsafe { (*d).vm.is_null() } || unsafe { (*d).activity } == DVM_NULL {
        return E_INVAL;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let act = unsafe { (*d).activity };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let mut rc = unsafe { invoke(d, act, c"onPause", c"()V", DVM_NULL) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { invoke(d, act, c"onStop", c"()V", DVM_NULL) };
    }
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { invoke(d, act, c"onDestroy", c"()V", DVM_NULL) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_release(d) };
    rc
}
