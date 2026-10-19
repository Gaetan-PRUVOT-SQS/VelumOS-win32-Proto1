use core::cell::UnsafeCell;
use core::ffi::c_char;

use crate::abi::{CTL_TEXT_MAX, E_INVAL, E_RANGE, t_clip};
use crate::raw::scopy_distinct;
use crate::utf8::ctl_u8_next;

struct ClipCell(UnsafeCell<t_clip>);

// SAFETY: UnsafeCell partagé. L'hôte et les tests touchent ce tampon sur un seul fil, comme le static C.
unsafe impl Sync for ClipCell {}

static CLIP: ClipCell = ClipCell(UnsafeCell::new(t_clip {
    data: [0; CTL_TEXT_MAX],
    len: 0,
}));

fn clip() -> *mut t_clip {
    CLIP.0.get()
}

/// # Safety
/// `utf8` est nul seulement si `len` vaut 0. Sinon il couvre `len` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_clip_set(utf8: *const c_char, len: u32) -> i32 {
    if utf8.is_null() && len != 0 {
        return E_INVAL;
    }
    if len > CTL_TEXT_MAX as u32 - 1 {
        return E_RANGE;
    }
    // SAFETY: CLIP est le tampon global, aligné, vivant tout le programme.
    let g = unsafe { &mut *clip() };
    if len != 0 {
        scopy_distinct(g.data.as_mut_ptr().cast(), utf8.cast(), len as usize);
    }
    g.data[len as usize] = 0;
    g.len = len;
    0
}

/// # Safety
/// `out` est non nul et couvre `cap` octets si `cap` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_clip_get(out: *mut c_char, cap: u32) -> i32 {
    if out.is_null() || cap == 0 {
        return E_INVAL;
    }
    // SAFETY: CLIP est vivant.
    let g = unsafe { &*clip() };
    let mut n = 0u32;
    // SAFETY: data couvre g.len octets.
    let mut next = unsafe { ctl_u8_next(g.data.as_ptr().cast(), g.len, 0) };
    while n < g.len && next < cap {
        n = next;
        // SAFETY: n < g.len, data couvre g.len octets.
        next = unsafe { ctl_u8_next(g.data.as_ptr().cast(), g.len, n) };
    }
    if n != 0 {
        scopy_distinct(out.cast(), g.data.as_ptr().cast(), n as usize);
    }
    // SAFETY: n < cap, out couvre cap octets.
    unsafe { out.add(n as usize).write(0) };
    n as i32
}

/// # Safety
/// Aucun pointeur d'appelant. Écrit le presse-papiers global.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_clip_clear() {
    // SAFETY: CLIP est vivant.
    let g = unsafe { &mut *clip() };
    g.data[0] = 0;
    g.len = 0;
}
