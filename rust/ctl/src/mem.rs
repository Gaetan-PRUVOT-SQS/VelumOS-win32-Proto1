use core::ffi::c_void;

#[cfg(not(all(not(test), velum_host)))]
use crate::ffi::{malloc, memset};

#[cfg(all(not(test), velum_host))]
unsafe extern "C" {
    pub fn ctl_alloc(size: usize) -> *mut c_void;
    pub fn ctl_free(ptr: *mut c_void);
}

/// # Safety
/// `size` est le nombre d'octets demandés. Le bloc rendu, s'il n'est pas nul, a `size` octets à zéro.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_alloc(size: usize) -> *mut c_void {
    // SAFETY: malloc est l'allocateur hôte. L'appel reste externe, sans mise en ligne.
    let p = unsafe { malloc(size) };
    if !p.is_null() {
        // SAFETY: malloc a rendu p, de taille size, ou size vaut 0.
        unsafe {
            memset(p, 0, size);
        }
    }
    p
}

/// # Safety
/// `ptr` est nul, ou a été rendu par `ctl_alloc` et n'est plus utilisé.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_free(ptr: *mut c_void) {
    // SAFETY: ptr est nul ou vient de l'allocateur hôte. free reste un appel externe.
    unsafe { crate::ffi::free(ptr) }
}
