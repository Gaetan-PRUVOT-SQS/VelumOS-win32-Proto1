use core::ffi::{c_char, c_int};

use crate::abi::{
    DROID_TEXT_MAX, DVM_NULL, DVM_THROWN, E_NOMEM, E_TIMEOUT, dvm_class_name, dvm_class_of,
    dvm_string_get, dvm_throw, dvm_throwable_message, t_dclass, t_dname, t_droid, t_dstr16,
};
use crate::utf::{dr_dotted, dr_utf8};
use crate::util::NSME;

fn cat(buf: &mut [u8], mut n: usize, s: *const c_char) -> usize {
    let mut i = 0usize;
    while !s.is_null() && n + 1 < DROID_TEXT_MAX {
        // SAFETY: s est une chaîne C, ou le tour précédent a lu un octet non nul.
        let ch = unsafe { s.add(i).read() };
        if ch == 0 {
            break;
        }
        buf[n] = ch as u8;
        n += 1;
        i += 1;
    }
    if n < buf.len() {
        buf[n] = 0;
    }
    n
}

fn describe(d: *mut t_droid, exc: u32) {
    // SAFETY: d n'est pas nul, error a DROID_TEXT_MAX octets.
    let d = unsafe { &mut *d };
    let vm = d.vm;
    // SAFETY: vm n'est pas nul quand describe est appelé depuis dr_fail.
    let name = unsafe { dvm_class_name(dvm_class_of(vm, exc)) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_dotted(name, d.error.as_mut_ptr().cast(), 128) };
    let mut msg = DVM_NULL;
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: vm n'est pas nul, msg est exclusif.
    if unsafe { dvm_throwable_message(vm, exc, &mut msg) } != 0
        || msg == DVM_NULL
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        || unsafe { dvm_string_get(vm, msg, &mut s) } != 0
    {
        return;
    }
    let n = cat(&mut d.error, n, c" : ".as_ptr());
    // SAFETY: error + n a DROID_TEXT_MAX - n octets, s est une chaîne UTF-16.
    unsafe { dr_utf8(s, d.error.as_mut_ptr().add(n).cast(), DROID_TEXT_MAX - n) };
}

/// # Safety
/// `d` n'est pas nul. `d.vm` n'est pas nul si `rc` vaut `DVM_THROWN`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_fail(d: *mut t_droid, rc: c_int) -> c_int {
    if rc == 0 {
        return 0;
    }
    // SAFETY: d n'est pas nul, error est un tableau de DROID_TEXT_MAX octets.
    unsafe { (*d).error[0] = 0 };
    if rc == DVM_THROWN {
        // SAFETY: d et d.vm ne sont pas nuls, pending est la référence levée.
        let exc = unsafe { (*(*d).vm).pending };
        describe(d, exc);
        // SAFETY: d.vm n'est pas nul.
        unsafe { (*(*d).vm).pending = DVM_NULL };
        return rc;
    }
    // SAFETY: d n'est pas nul.
    let err = unsafe { &mut (*d).error };
    cat(err, 0, c"arrêt de la machine virtuelle".as_ptr());
    if rc == E_TIMEOUT {
        cat(err, 0, c"budget d'instructions épuisé".as_ptr());
    }
    if rc == E_NOMEM {
        cat(err, 0, c"mémoire épuisée".as_ptr());
    }
    rc
}

/// # Safety
/// `d` et `d.vm` ne sont pas nuls. `nm` n'est pas nul. `c` peut être nul : le nom alors lu est nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_missing(
    d: *mut t_droid,
    c: *const t_dclass,
    nm: *const t_dname,
) -> c_int {
    let mut buf = [0u8; DROID_TEXT_MAX];
    // SAFETY: c est celui reçu, dvm_class_name accepte nul.
    let cname = unsafe { dvm_class_name(c) };
    let n = cat(&mut buf, 0, cname);
    let n = cat(&mut buf, n, c"->".as_ptr());
    // SAFETY: nm n'est pas nul, name et sig sont des chaînes C.
    let (name, sig) = unsafe { ((*nm).name, (*nm).sig) };
    let n = cat(&mut buf, n, name);
    cat(&mut buf, n, sig);
    // SAFETY: d et d.vm ne sont pas nuls. buf est terminé par NUL.
    let vm = unsafe { (*d).vm };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dvm_throw(vm, NSME.as_ptr(), buf.as_ptr().cast()) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_fail(d, rc) }
}
