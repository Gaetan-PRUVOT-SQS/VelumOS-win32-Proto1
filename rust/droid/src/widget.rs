use core::ffi::c_int;

use crate::abi::{
    DROID_KIDS_MAX, DROID_TEXT_MAX, DROID_VERTICAL, DROID_VIEWS_MAX, DV_BUTTON, DV_LINEAR, DV_TEXT,
    DVM_NULL, arg, dvm_string_get, t_dstr16, t_dvm,
};
use crate::text::{dr_new8, dr_text_of};
use crate::utf::dr_utf8;
use crate::util::{CCE, IAE, ISE, VG, dr_host, dr_is, throw_at};
use crate::view::dr_view_get;

fn ancestor(d: *const crate::abi::t_droid, id: u32, mut of: u32) -> bool {
    let mut n = 0u32;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    while of != 0 && unsafe { of <= (*d).nviews } && n <= DROID_VIEWS_MAX {
        n += 1;
        if of == id {
            return true;
        }
        // SAFETY: of est dans 1..=nviews, d n'est pas nul.
        of = unsafe { (*d).views[(of - 1) as usize].parent };
    }
    false
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_tv_settext(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    let mut v = core::ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let mut rc = unsafe { dr_view_get(vm, arg(args, 0), &mut v) };
    if rc == 0 {
        // SAFETY: v n'est pas nul.
        let kind = unsafe { (*v).kind };
        if kind != DV_TEXT && kind != DV_BUTTON {
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            rc = unsafe { throw_at(vm, CCE, c"TextView attendu") };
        }
    }
    let mut str_ref = DVM_NULL;
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { dr_text_of(vm, arg(args, 1), &mut str_ref) };
    }
    if rc != 0 {
        return rc;
    }
    // SAFETY: v n'est pas nul, text a DROID_TEXT_MAX octets.
    unsafe { (*v).text[0] = 0 };
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    if str_ref != DVM_NULL && unsafe { dvm_string_get(vm, str_ref, &mut s) } == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_utf8(s, (*v).text.as_mut_ptr().cast(), DROID_TEXT_MAX) };
    }
    // SAFETY: une vue construite a un hôte.
    unsafe { (*dr_host(vm)).dirty = 1 };
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_tv_gettext(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut v = core::ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let mut rc = unsafe { dr_view_get(vm, arg(args, 0), &mut v) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        let kind = unsafe { (*v).kind };
        if kind != DV_TEXT && kind != DV_BUTTON {
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            rc = unsafe { throw_at(vm, CCE, c"TextView attendu") };
        }
    }
    if rc != 0 {
        return rc;
    }
    // SAFETY: v n'est pas nul, text est une chaîne C, ret n'est pas nul.
    unsafe { dr_new8(vm, (*v).text.as_ptr().cast(), ret) }
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_vg_addview(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let d = unsafe { dr_host(vm) };
    let mut g = core::ptr::null_mut();
    let mut k = core::ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let mut rc = unsafe { dr_view_get(vm, arg(args, 0), &mut g) };
    if rc == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        rc = unsafe { dr_view_get(vm, arg(args, 1), &mut k) };
    }
    if rc != 0 {
        return rc;
    }
    if
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_is(vm, arg(args, 0), VG.as_ptr()) } == 0 {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"ViewGroup attendu") };
    }
    // SAFETY: g et k ne sont pas nuls, d non plus puisqu'une vue existe.
    let (k_parent, k_id, g_id, g_kids, root) =
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { ((*k).parent, (*k).id, (*g).id, (*g).nkids, (*d).root) };
    if k_parent != 0 || k_id == root || ancestor(d, k_id, g_id) {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, ISE, c"vue déjà placée ou cycle") };
    }
    if g_kids >= DROID_KIDS_MAX {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, ISE, c"trop d'enfants") };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe {
        (*g).kids[g_kids as usize] = k_id;
        (*g).nkids = g_kids + 1;
        (*k).parent = g_id;
        (*d).dirty = 1;
    }
    0
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins deux mots. `ret` peut être nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_ll_orient(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let _ = ret;
    let mut v = core::ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_view_get(vm, arg(args, 0), &mut v) };
    if rc != 0 {
        return rc;
    }
    // SAFETY: v n'est pas nul.
    if unsafe { (*v).kind } != DV_LINEAR {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, CCE, c"LinearLayout attendu") };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let orient = unsafe { arg(args, 1) };
    if orient > DROID_VERTICAL {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { throw_at(vm, IAE, c"orientation inconnue") };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*v).orientation = orient };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*dr_host(vm)).dirty = 1 };
    0
}
