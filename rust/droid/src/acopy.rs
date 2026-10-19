use core::ffi::{c_int, c_void};

use crate::abi::{
    DVM_NULL, arg, dvm_array_view, dvm_class_name, dvm_class_of, dvm_is_instance, memmove,
    t_darrview, t_dvm,
};
use crate::util::{AIOOBE, ASE, dr_class, dr_npe, throw_at};

fn bounds(a: &t_darrview, pos: u32, len: u32) -> bool {
    (pos as i32) >= 0 && (len as i32) >= 0 && pos <= a.len && len <= a.len - pos
}

unsafe fn refs(vm: *mut t_dvm, args: *const u32, v: &[t_darrview; 2]) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let src_at = unsafe { arg(args, 1) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let dst_ref = unsafe { arg(args, 2) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let dst_at = unsafe { arg(args, 3) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { arg(args, 4) };
    // SAFETY: vm n'est pas nul.
    let name = unsafe { dvm_class_name(dvm_class_of(vm, dst_ref)) };
    let desc = if name.is_null() {
        core::ptr::null()
    } else {
        // SAFETY: name est une chaîne C d'au moins un octet (le descripteur de tableau).
        unsafe { name.add(1) }
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let elem = unsafe { dr_class(vm, desc) };
    // SAFETY: les vues viennent de dvm_array_view et les bornes tiennent.
    let s = unsafe { v[0].data.cast::<u32>().add(src_at as usize) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let d = unsafe { v[1].data.cast::<u32>().add(dst_at as usize) };
    let mut i = 0u32;
    while i < n {
        // SAFETY: i < n et les bornes garantissent i + index dans le tableau.
        let sv = unsafe { s.add(i as usize).read() };
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        if sv != DVM_NULL && unsafe { dvm_is_instance(vm, sv, elem) } == 0 {
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            return unsafe { throw_at(vm, ASE, c"élément incompatible") };
        }
        // SAFETY: même indice, destination exclusive pour cet élément.
        unsafe { d.add(i as usize).write(sv) };
        i += 1;
    }
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins cinq mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_sys_arraycopy(
    vm: *mut t_dvm,
    args: *const u32,
    ret: *mut u64,
) -> c_int {
    let _ = ret;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let src = unsafe { arg(args, 0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let src_at = unsafe { arg(args, 1) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let dst = unsafe { arg(args, 2) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let dst_at = unsafe { arg(args, 3) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { arg(args, 4) };
    if src == DVM_NULL || dst == DVM_NULL {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    let mut v = [t_darrview {
        data: core::ptr::null_mut(),
        len: 0,
        width: 0,
        is_ref: 0,
    }; 2];
    // SAFETY: vm n'est pas nul, v est exclusif.
    let bad = unsafe { dvm_array_view(vm, src, &mut v[0]) } != 0
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        || unsafe { dvm_array_view(vm, dst, &mut v[1]) } != 0
        || v[0].is_ref != v[1].is_ref;
    if bad {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, ASE, c"tableaux incompatibles") };
    }
    if !bounds(&v[0], src_at, n) || !bounds(&v[1], dst_at, n) {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, AIOOBE, c"arraycopy hors bornes") };
    }
    // SAFETY: vm n'est pas nul.
    if unsafe { dvm_class_of(vm, src) } != unsafe { dvm_class_of(vm, dst) } {
        if v[0].is_ref == 0 {
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            return unsafe { throw_at(vm, ASE, c"types primitifs différents") };
        }
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { refs(vm, args, &v) };
    }
    let nbytes = (n as usize).wrapping_mul(v[0].width as usize);
    if nbytes == 0 {
        return 0;
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let dst_p = unsafe {
        v[1].data
            .cast::<u8>()
            .add((dst_at as usize).wrapping_mul(v[1].width as usize))
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let src_p = unsafe {
        v[0].data
            .cast::<u8>()
            .add((src_at as usize).wrapping_mul(v[0].width as usize))
    };
    // SAFETY: les deux plages font nbytes octets et tiennent dans les tableaux. memmove accepte le recouvrement.
    unsafe { memmove(dst_p.cast::<c_void>(), src_p.cast::<c_void>(), nbytes) };
    0
}
