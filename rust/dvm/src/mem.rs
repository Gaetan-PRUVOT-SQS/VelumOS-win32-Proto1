use core::ffi::{c_char, c_void};

unsafe extern "C" {
    fn calloc(nmemb: usize, size: usize) -> *mut c_void;
    fn free(ptr: *mut c_void);
}

#[cfg(test)]
use core::cell::Cell;

#[cfg(test)]
std::thread_local! {
    static LIVE: Cell<i32> = const { Cell::new(0) };
    static ARMED: Cell<bool> = const { Cell::new(false) };
    static LEFT: Cell<u32> = const { Cell::new(0) };
}

#[cfg(test)]
pub(crate) fn live() -> i32 {
    LIVE.get()
}

#[cfg(test)]
pub(crate) fn arm(n: u32) {
    LEFT.set(n);
    ARMED.set(true);
}

#[cfg(test)]
pub(crate) fn disarm() {
    ARMED.set(false);
}

pub(crate) fn c_calloc(nmemb: usize, size: usize) -> *mut c_void {
    #[cfg(test)]
    if ARMED.get() {
        let left = LEFT.get();
        if left == 0 {
            return core::ptr::null_mut();
        }
        LEFT.set(left - 1);
    }
    // SAFETY: calloc est le symbole C. nmemb et size sont ceux du C.
    let p = unsafe { calloc(nmemb, size) };
    #[cfg(test)]
    if !p.is_null() {
        LIVE.set(LIVE.get() + 1);
    }
    p
}

pub(crate) fn c_free(ptr: *mut c_void) {
    if ptr.is_null() {
        return;
    }
    #[cfg(test)]
    LIVE.set(LIVE.get() - 1);
    // SAFETY: ptr vient de calloc, une seule fois, ou le C libère un pointeur qu'il a alloué.
    unsafe { free(ptr) };
}

pub(crate) unsafe fn c_strlen(s: *const c_char) -> usize {
    let mut n = 0usize;
    // SAFETY: s est une chaîne C terminée, comme strlen.
    while unsafe { *s.add(n) } != 0 {
        n += 1;
    }
    n
}

pub(crate) unsafe fn c_strcmp(a: *const c_char, b: *const c_char) -> i32 {
    let mut i = 0usize;
    loop {
        // SAFETY: a et b sont des chaînes C. On s'arrête au premier NUL.
        let x = unsafe { *a.add(i) } as u8;
        // SAFETY: b est une chaîne C, même borne que a.
        let y = unsafe { *b.add(i) } as u8;
        if x != y || x == 0 {
            return i32::from(x) - i32::from(y);
        }
        i += 1;
    }
}
