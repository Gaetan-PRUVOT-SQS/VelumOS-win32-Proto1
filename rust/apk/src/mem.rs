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
    fn apk_host_alloc(size: usize) -> *mut c_void;
    fn apk_host_free(ptr: *mut c_void);
}

#[cfg(test)]
pub(crate) static ALLOC_LEFT: AtomicU32 = AtomicU32::new(u32::MAX);
#[cfg(test)]
pub(crate) static ALLOC_LIVE: AtomicI32 = AtomicI32::new(0);

#[cfg(test)]
pub(crate) fn reset_alloc(left: u32) {
    ALLOC_LEFT.store(left, Ordering::SeqCst);
}

#[cfg(test)]
pub(crate) fn alloc_live() -> i32 {
    ALLOC_LIVE.load(Ordering::SeqCst)
}

#[cfg(not(velum_host))]
fn host_alloc(size: usize) -> *mut c_void {
    // SAFETY: malloc est l'allocateur C. size est transmis tel quel.
    unsafe { malloc(size) }
}

#[cfg(velum_host)]
fn host_alloc(size: usize) -> *mut c_void {
    // SAFETY: apk_host_alloc est fourni par l'hôte. size est transmis tel quel.
    unsafe { apk_host_alloc(size) }
}

#[cfg(not(velum_host))]
fn host_free(ptr: *mut c_void) {
    // SAFETY: ptr est non nul et vient de malloc, une seule fois.
    unsafe { free(ptr) };
}

#[cfg(velum_host)]
fn host_free(ptr: *mut c_void) {
    // SAFETY: ptr est non nul et vient de apk_host_alloc, une seule fois.
    unsafe { apk_host_free(ptr) };
}

pub(crate) fn heap_alloc(size: usize) -> *mut c_void {
    #[cfg(test)]
    {
        let left = ALLOC_LEFT.load(Ordering::SeqCst);
        if left == 0 {
            return core::ptr::null_mut();
        }
        if left != u32::MAX {
            ALLOC_LEFT.store(left - 1, Ordering::SeqCst);
        }
    }
    let p = host_alloc(size);
    #[cfg(test)]
    if !p.is_null() {
        ALLOC_LIVE.fetch_add(1, Ordering::SeqCst);
    }
    p
}

pub(crate) fn heap_free(ptr: *mut c_void) {
    if ptr.is_null() {
        return;
    }
    #[cfg(test)]
    {
        ALLOC_LIVE.fetch_sub(1, Ordering::SeqCst);
    }
    host_free(ptr);
}
