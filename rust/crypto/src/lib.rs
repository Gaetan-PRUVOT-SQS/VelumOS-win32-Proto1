#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod chacha;
mod hmac;
mod pbkdf2;
mod sha256;

use core::ffi::{c_int, c_void};

pub use chacha::{chacha20_block, chacha20_init};
pub use hmac::{hmac_final, hmac_prepare, hmac_sha256, hmac_update};
pub use pbkdf2::{pbkdf2_sha256, pbkdf2_validate};
pub use sha256::{sha256, sha256_final, sha256_init, sha256_update};

pub const E_OK: c_int = 0;
pub const E_INVAL: c_int = -22;
pub const PBKDF2_ITERS_MAX: u32 = 10_000_000;

pub(crate) const SHA256_LEN: usize = 32;
pub(crate) const SHA256_BLOCK: usize = 64;
pub(crate) const PBKDF2_BLOCKS_MAX: usize = 0xffff_ffff;

#[repr(C)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub struct TSha256 {
    pub h: [u32; 8],
    pub total: u64,
    pub buf: [u8; SHA256_BLOCK],
    pub fill: u32,
}

#[repr(C)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub struct THmac {
    pub inner: TSha256,
    pub outer: TSha256,
}

#[repr(C)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub struct TPbkdf2 {
    pub pw: *const u8,
    pub pwlen: usize,
    pub salt: *const u8,
    pub saltlen: usize,
    pub iters: u32,
}

#[repr(C)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub struct TChacha20 {
    pub state: [u32; 16],
}

pub(crate) const fn blank_sha() -> TSha256 {
    TSha256 {
        h: [0; 8],
        total: 0,
        buf: [0; SHA256_BLOCK],
        fill: 0,
    }
}

pub(crate) const fn blank_hmac() -> THmac {
    THmac {
        inner: blank_sha(),
        outer: blank_sha(),
    }
}

pub(crate) fn wipe<T>(v: &mut T) {
    let n = core::mem::size_of::<T>();
    if n == 0 {
        return;
    }
    let p = core::ptr::from_mut(v).cast::<c_void>();
    // SAFETY: v est un objet mutable entier, remplissage compris.
    unsafe { secure_zero(p, n) };
}

#[unsafe(no_mangle)]
/// # Safety
/// `a` et `b` sont nuls ou pointent `n` octets. Rien n'est lu si `n` vaut 0 ou si un pointeur est nul.
pub unsafe extern "C" fn ct_equal(a: *const c_void, b: *const c_void, n: usize) -> c_int {
    if n == 0 {
        return 1;
    }
    if a.is_null() || b.is_null() {
        return 0;
    }
    let a = a.cast::<u8>();
    let b = b.cast::<u8>();
    let mut diff = 0u32;
    let mut i = 0usize;
    while i < n {
        // SAFETY: a et b sont non nuls et couvrent n octets, i < n.
        let (ca, cb) = unsafe { (a.add(i).read(), b.add(i).read()) };
        diff |= u32::from(ca ^ cb);
        i += 1;
    }
    c_int::from(diff == 0)
}

#[unsafe(no_mangle)]
/// # Safety
/// `p` est nul ou pointe `n` octets accessibles en écriture.
pub unsafe extern "C" fn secure_zero(p: *mut c_void, n: usize) {
    if p.is_null() || n == 0 {
        return;
    }
    let b = p.cast::<u8>();
    let mut i = 0usize;
    while i < n {
        // SAFETY: p n'est pas nul et couvre n octets en écriture, i < n.
        unsafe { b.add(i).write_volatile(0) };
        i += 1;
    }
}

const _: () = assert!(core::mem::size_of::<TSha256>() == 112);
const _: () = assert!(core::mem::align_of::<TSha256>() == 8);
const _: () = assert!(core::mem::size_of::<THmac>() == 224);
const _: () = assert!(core::mem::align_of::<THmac>() == 8);
const _: () = assert!(core::mem::size_of::<TPbkdf2>() == 40);
const _: () = assert!(core::mem::align_of::<TPbkdf2>() == 8);
const _: () = assert!(core::mem::size_of::<TChacha20>() == 64);
const _: () = assert!(core::mem::offset_of!(TSha256, h) == 0);
const _: () = assert!(core::mem::offset_of!(TSha256, total) == 32);
const _: () = assert!(core::mem::offset_of!(TSha256, buf) == 40);
const _: () = assert!(core::mem::offset_of!(TSha256, fill) == 104);
const _: () = assert!(core::mem::offset_of!(TPbkdf2, pw) == 0);
const _: () = assert!(core::mem::offset_of!(TPbkdf2, pwlen) == 8);
const _: () = assert!(core::mem::offset_of!(TPbkdf2, salt) == 16);
const _: () = assert!(core::mem::offset_of!(TPbkdf2, saltlen) == 24);
const _: () = assert!(core::mem::offset_of!(TPbkdf2, iters) == 32);
const _: () = assert!(core::mem::offset_of!(THmac, inner) == 0);
const _: () = assert!(core::mem::offset_of!(THmac, outer) == 112);
const _: () = assert!(core::mem::offset_of!(TChacha20, state) == 0);

#[cfg(test)]
mod tests;
