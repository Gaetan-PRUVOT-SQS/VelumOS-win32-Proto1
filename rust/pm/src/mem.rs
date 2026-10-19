use core::ffi::c_void;

#[cfg(test)]
use core::sync::atomic::{AtomicI32, AtomicU32, Ordering};

#[cfg(not(velum_host))]
unsafe extern "C" {
    fn malloc(size: usize) -> *mut c_void;
    fn free(ptr: *mut c_void);
}

#[cfg(velum_host)]
unsafe extern "C" {
    pub fn pm_alloc(size: usize) -> *mut c_void;
    pub fn pm_free(ptr: *mut c_void);
}

#[cfg(test)]
pub(crate) static ALLOC_CALLS: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static ALLOC_FAIL: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static ALLOC_LIVE: AtomicI32 = AtomicI32::new(0);

#[cfg(test)]
pub(crate) fn reset_alloc(fail_at: u32) {
    ALLOC_CALLS.store(0, Ordering::SeqCst);
    ALLOC_FAIL.store(fail_at, Ordering::SeqCst);
}

#[cfg(not(velum_host))]
fn allow_alloc() -> bool {
    #[cfg(test)]
    {
        let n = ALLOC_CALLS.fetch_add(1, Ordering::SeqCst).wrapping_add(1);
        let fail = ALLOC_FAIL.load(Ordering::SeqCst);
        if fail > 0 && n == fail {
            return false;
        }
    }
    true
}

#[cfg(not(velum_host))]
fn note_live(p: *mut c_void) {
    #[cfg(test)]
    if !p.is_null() {
        ALLOC_LIVE.fetch_add(1, Ordering::SeqCst);
    }
    #[cfg(not(test))]
    {
        let _ = p;
    }
}

#[cfg(not(velum_host))]
fn note_free() {
    #[cfg(test)]
    {
        ALLOC_LIVE.fetch_sub(1, Ordering::SeqCst);
    }
}

/// # Safety
/// `size` est la taille demandée à l'allocateur. Un échec rend un pointeur nul.
#[cfg(not(velum_host))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_alloc(size: usize) -> *mut c_void {
    if !allow_alloc() {
        return core::ptr::null_mut();
    }
    // SAFETY: malloc est l'allocateur C. size est transmis tel quel.
    let p = unsafe { malloc(size) };
    note_live(p);
    p
}

/// # Safety
/// `ptr` est nul, ou bien il vient de `pm_alloc` et n'a pas déjà été libéré.
#[cfg(not(velum_host))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_free(ptr: *mut c_void) {
    if ptr.is_null() {
        return;
    }
    note_free();
    // SAFETY: ptr non nul, issu de pm_alloc, une seule fois.
    unsafe { free(ptr) };
}
