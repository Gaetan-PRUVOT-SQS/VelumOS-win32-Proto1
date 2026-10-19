use core::ffi::c_int;

use crate::abi::{DROID_TEXT_MAX, arg, dvm_class_name, dvm_class_of, t_dvm};
use crate::text::dr_new8;
use crate::utf::dr_dotted;
use crate::util::dr_npe;

/// # Safety
/// Aucun pointeur n'est lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_nop(_vm: *mut t_dvm, _args: *const u32, _ret: *mut u64) -> c_int {
    0
}

fn hex(v: u32, out: &mut [u8]) -> usize {
    let mut n = 0usize;
    let mut sh = 28i32;
    while sh > 0 && ((v >> (sh as u32)) & 15) == 0 {
        sh -= 4;
    }
    while sh >= 0 {
        let dig = ((v >> (sh as u32)) & 15) as usize;
        out[n] = b"0123456789abcdef"[dig];
        n += 1;
        sh -= 4;
    }
    out[n] = 0;
    n
}

/// # Safety
/// `vm` n'est pas nul. `args` pointe au moins un mot. `ret` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_obj_tostring(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    let mut buf = [0u8; DROID_TEXT_MAX + 16];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let id = unsafe { arg(args, 0) };
    // SAFETY: vm n'est pas nul.
    let desc = unsafe { dvm_class_name(dvm_class_of(vm, id)) };
    if desc.is_null() {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        return unsafe { dr_npe(vm) };
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_dotted(desc, buf.as_mut_ptr().cast(), DROID_TEXT_MAX) };
    buf[n] = b'@';
    hex(id, &mut buf[n + 1..]);
    // SAFETY: buf est terminé par NUL, vm et ret ne sont pas nuls.
    unsafe { dr_new8(vm, buf.as_ptr().cast(), ret) }
}

/// # Safety
/// `args` pointe au moins deux mots. `ret` n'est pas nul. `vm` n'est pas lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_obj_equals(_vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let a = unsafe { arg(args, 0) };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let b = unsafe { arg(args, 1) };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(a == b)) };
    0
}

/// # Safety
/// `args` pointe au moins un mot. `ret` n'est pas nul. `vm` n'est pas lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_obj_hash(_vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let a = unsafe { arg(args, 0) };
    // SAFETY: ret n'est pas nul.
    unsafe { ret.write(u64::from(a)) };
    0
}
