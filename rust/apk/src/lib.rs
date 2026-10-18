#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

use core::ffi::{c_char, c_int};

mod ffi;
mod man;
mod mem;
mod open;
mod rd;
mod read;
mod sig;

pub use man::apk_manifest;
pub use open::apk_open;
pub use read::{apk_read, apk_reason};
pub use sig::apk_verify;

pub const E_OK: c_int = 0;
pub const E_PERM: c_int = -1;
pub const E_NOENT: c_int = -2;
pub const E_IO: c_int = -5;
pub const E_NOMEM: c_int = -12;
pub const E_ACCES: c_int = -13;
pub const E_EXIST: c_int = -17;
pub const E_INVAL: c_int = -22;
pub const E_NOSPC: c_int = -28;
pub const E_RANGE: c_int = -34;
pub const E_OVERFLOW: c_int = -75;
pub const E_NOTSUP: c_int = -95;

const _: [c_int; 12] = [
    E_OK, E_PERM, E_NOENT, E_IO, E_NOMEM, E_ACCES, E_EXIST, E_INVAL, E_NOSPC, E_RANGE, E_OVERFLOW,
    E_NOTSUP,
];

pub const APK_FILE_MAX: usize = 0x400_0000;
pub const APK_ENTRY_MAX: u32 = 0x100_0000;
pub const APK_CHUNK: usize = 0x100_000;
pub const APK_PKG_MAX: usize = 128;
pub const APK_LABEL_MAX: usize = 64;
pub const APK_CLASS_MAX: usize = 192;
pub const APK_DESC_MAX: usize = 196;
pub const APK_VNAME_MAX: usize = 32;
pub const APK_PERM_MAX: usize = 16;
pub const APK_PERM_LEN: usize = 64;
pub const APK_ELEM_MAX: usize = 32;
pub const APK_SIG_V2_ID: u32 = 0x7109_871a;
pub const APK_ALGO_RSA_PKCS1_SHA256: u32 = 0x0103;
pub(crate) const APK_MAGIC: &[u8] = b"APK Sig Block 42";
pub(crate) const APK_EOCD_SIG: u32 = 0x0605_4b50;
pub(crate) const APK_EOCD_LEN: usize = 22;
pub(crate) const SHA256_LEN: usize = 32;

pub const APKR_OK: c_int = 0;
pub const APKR_ARCHIVE: c_int = 1;
pub const APKR_TAILLE: c_int = 2;
pub const APKR_NOM: c_int = 3;
pub const APKR_NATIF: c_int = 4;
pub const APKR_MULTIDEX: c_int = 5;
pub const APKR_MANQUE: c_int = 6;
pub const APKR_SIGNATURE: c_int = 7;
pub const APKR_ALGO: c_int = 8;
pub const APKR_SIGNATAIRES: c_int = 9;
pub const APKR_MANIFESTE: c_int = 10;
pub const APKR_PAQUET: c_int = 11;
pub const APKR_ACTIVITE: c_int = 12;
pub const APKR_LIBELLE: c_int = 13;
pub const APKR_MEMOIRE: c_int = 14;
pub const APKR_COUNT: c_int = 15;

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_span {
    pub p: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_zip {
    pub file: t_span,
    pub cd_off: u32,
    pub cd_size: u32,
    pub count: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_zipent {
    pub name: *const c_char,
    pub name_len: u32,
    pub method: u32,
    pub crc: u32,
    pub csize: u32,
    pub r#usize: u32,
    pub data_off: u32,
    pub lfh_off: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_apk {
    pub file: t_span,
    pub zip: t_zip,
    pub manifest: t_zipent,
    pub dex: t_zipent,
    pub reason: c_int,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_apksig {
    pub cert_sha256: [u8; SHA256_LEN],
    pub algo: u32,
    pub reason: c_int,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_apkmanifest {
    pub package: [u8; APK_PKG_MAX],
    pub version_name: [u8; APK_VNAME_MAX],
    pub label: [u8; APK_LABEL_MAX],
    pub activity: [u8; APK_CLASS_MAX],
    pub activity_desc: [u8; APK_DESC_MAX],
    pub perms: [[u8; APK_PERM_LEN]; APK_PERM_MAX],
    pub perm_count: u32,
    pub version_code: u32,
    pub min_sdk: u32,
    pub target_sdk: u32,
    pub reason: c_int,
}

impl t_span {
    pub(crate) const fn zero() -> Self {
        Self {
            p: core::ptr::null(),
            len: 0,
        }
    }
}

impl t_zip {
    pub(crate) const fn zero() -> Self {
        Self {
            file: t_span::zero(),
            cd_off: 0,
            cd_size: 0,
            count: 0,
        }
    }
}

impl t_zipent {
    pub(crate) const fn zero() -> Self {
        Self {
            name: core::ptr::null(),
            name_len: 0,
            method: 0,
            crc: 0,
            csize: 0,
            r#usize: 0,
            data_off: 0,
            lfh_off: 0,
        }
    }
}

impl t_apk {
    pub(crate) const fn zero() -> Self {
        Self {
            file: t_span::zero(),
            zip: t_zip::zero(),
            manifest: t_zipent::zero(),
            dex: t_zipent::zero(),
            reason: 0,
        }
    }
}

impl t_apksig {
    pub(crate) const fn zero() -> Self {
        Self {
            cert_sha256: [0; SHA256_LEN],
            algo: 0,
            reason: 0,
        }
    }
}

impl t_apkmanifest {
    pub(crate) const fn zero() -> Self {
        Self {
            package: [0; APK_PKG_MAX],
            version_name: [0; APK_VNAME_MAX],
            label: [0; APK_LABEL_MAX],
            activity: [0; APK_CLASS_MAX],
            activity_desc: [0; APK_DESC_MAX],
            perms: [[0; APK_PERM_LEN]; APK_PERM_MAX],
            perm_count: 0,
            version_code: 0,
            min_sdk: 0,
            target_sdk: 0,
            reason: 0,
        }
    }
}

const _: () = assert!(core::mem::size_of::<t_span>() == 16);
const _: () = assert!(core::mem::align_of::<t_span>() == 8);
const _: () = assert!(core::mem::offset_of!(t_span, p) == 0);
const _: () = assert!(core::mem::offset_of!(t_span, len) == 8);
const _: () = assert!(core::mem::size_of::<t_zip>() == 32);
const _: () = assert!(core::mem::align_of::<t_zip>() == 8);
const _: () = assert!(core::mem::offset_of!(t_zip, file) == 0);
const _: () = assert!(core::mem::offset_of!(t_zip, cd_off) == 16);
const _: () = assert!(core::mem::offset_of!(t_zip, cd_size) == 20);
const _: () = assert!(core::mem::offset_of!(t_zip, count) == 24);
const _: () = assert!(core::mem::size_of::<t_zipent>() == 40);
const _: () = assert!(core::mem::align_of::<t_zipent>() == 8);
const _: () = assert!(core::mem::offset_of!(t_zipent, name) == 0);
const _: () = assert!(core::mem::offset_of!(t_zipent, name_len) == 8);
const _: () = assert!(core::mem::offset_of!(t_zipent, method) == 12);
const _: () = assert!(core::mem::offset_of!(t_zipent, crc) == 16);
const _: () = assert!(core::mem::offset_of!(t_zipent, csize) == 20);
const _: () = assert!(core::mem::offset_of!(t_zipent, r#usize) == 24);
const _: () = assert!(core::mem::offset_of!(t_zipent, data_off) == 28);
const _: () = assert!(core::mem::offset_of!(t_zipent, lfh_off) == 32);
const _: () = assert!(core::mem::size_of::<t_apk>() == 136);
const _: () = assert!(core::mem::align_of::<t_apk>() == 8);
const _: () = assert!(core::mem::offset_of!(t_apk, file) == 0);
const _: () = assert!(core::mem::offset_of!(t_apk, zip) == 16);
const _: () = assert!(core::mem::offset_of!(t_apk, manifest) == 48);
const _: () = assert!(core::mem::offset_of!(t_apk, dex) == 88);
const _: () = assert!(core::mem::offset_of!(t_apk, reason) == 128);
const _: () = assert!(core::mem::size_of::<t_apksig>() == 40);
const _: () = assert!(core::mem::align_of::<t_apksig>() == 4);
const _: () = assert!(core::mem::offset_of!(t_apksig, cert_sha256) == 0);
const _: () = assert!(core::mem::offset_of!(t_apksig, algo) == 32);
const _: () = assert!(core::mem::offset_of!(t_apksig, reason) == 36);
const _: () = assert!(core::mem::size_of::<t_apkmanifest>() == 1656);
const _: () = assert!(core::mem::align_of::<t_apkmanifest>() == 4);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, package) == 0);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, version_name) == 128);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, label) == 160);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, activity) == 224);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, activity_desc) == 416);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, perms) == 612);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, perm_count) == 1636);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, version_code) == 1640);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, min_sdk) == 1644);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, target_sdk) == 1648);
const _: () = assert!(core::mem::offset_of!(t_apkmanifest, reason) == 1652);
const _: () = assert!(APK_MAGIC.len() == 16);
const _: () = assert!(APKR_COUNT == 15);

#[cfg(test)]
mod tests;
