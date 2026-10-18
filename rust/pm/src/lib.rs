#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

use core::ffi::{c_char, c_int, c_void};

mod check;
mod mem;
mod ops;
mod raw;
mod text;

pub use check::{
    pm_act_ok, pm_dec_u32, pm_hex_cert, pm_info_ok, pm_label_ok, pm_nlen, pm_pkg_ok, pm_same_pkg,
    pm_utf8_ok,
};
pub use mem::{pm_alloc, pm_free};
pub use ops::{pm_drop, pm_find, pm_install, pm_list, pm_remove, pm_store, pm_work_open};
pub use text::{
    pm_defaults, pm_index, pm_path, pm_registry_format, pm_registry_parse, pm_registry_scan,
};

pub(crate) const E_OK: c_int = 0;
pub(crate) const E_PERM: c_int = -1;
pub(crate) const E_NOENT: c_int = -2;
pub(crate) const E_IO: c_int = -5;
pub(crate) const E_NOMEM: c_int = -12;
pub(crate) const E_ACCES: c_int = -13;
pub(crate) const E_EXIST: c_int = -17;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_NOSPC: c_int = -28;
pub(crate) const E_RANGE: c_int = -34;
pub(crate) const E_OVERFLOW: c_int = -75;
pub(crate) const E_NOTSUP: c_int = -95;

const _: [c_int; 12] = [
    E_OK, E_PERM, E_NOENT, E_IO, E_NOMEM, E_ACCES, E_EXIST, E_INVAL, E_NOSPC, E_RANGE, E_OVERFLOW,
    E_NOTSUP,
];

pub const PM_PKG_MAX: usize = 128;
pub const PM_LABEL_MAX: usize = 64;
pub const PM_ACT_MAX: usize = 192;
pub const PM_CERT_LEN: usize = 32;
pub const PM_PATH_MAX: usize = 192;
pub const PM_ROOT_MAX: usize = 48;
pub const PM_MAX_PACKAGES: u32 = 64;
pub const PM_APK_MAX: u32 = 67_108_864;
pub const PM_REG_MAX: u32 = 65_536;
pub const PM_ORIGIN_SYSTEM: u32 = 0;
pub const PM_ORIGIN_DATA: u32 = 1;
pub(crate) const PM_LINE_MAX: usize = 460;
pub(crate) const PM_REG_NAME: &[u8] = b"paquets\0";

const TEXT_CAP: usize = PM_REG_MAX as usize + 1;
const DATA_SLOTS: usize = PM_MAX_PACKAGES as usize + 1;

pub type PmRead = unsafe extern "C" fn(*mut c_void, *const c_char, t_text) -> i64;
pub type PmWrite = unsafe extern "C" fn(*mut c_void, *const c_char, t_span) -> c_int;
pub type PmRename = unsafe extern "C" fn(*mut c_void, *const c_char, *const c_char) -> c_int;
pub type PmUnlink = unsafe extern "C" fn(*mut c_void, *const c_char) -> c_int;
pub type PmMkdir = unsafe extern "C" fn(*mut c_void, *const c_char) -> c_int;
pub type PmInspect = unsafe extern "C" fn(t_span, *mut t_pminfo) -> c_int;

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
pub struct t_text {
    pub p: *mut c_char,
    pub cap: usize,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_pminfo {
    pub package: [u8; PM_PKG_MAX],
    pub version_code: u32,
    pub label: [u8; PM_LABEL_MAX],
    pub activity: [u8; PM_ACT_MAX],
    pub cert: [u8; PM_CERT_LEN],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_pmentry {
    pub info: t_pminfo,
    pub origin: u32,
    pub path: [u8; PM_PATH_MAX],
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_pmreg {
    pub e: *mut t_pmentry,
    pub max: u32,
    pub n: u32,
    pub rejected: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_pmfs {
    pub read: Option<PmRead>,
    pub write_new: Option<PmWrite>,
    pub rename: Option<PmRename>,
    pub unlink: Option<PmUnlink>,
    pub mkdir: Option<PmMkdir>,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_pm {
    pub sys_root: *const c_char,
    pub data_root: *const c_char,
    pub fs: *const t_pmfs,
    pub inspect: Option<PmInspect>,
    pub ctx: *mut c_void,
    pub max_packages: u32,
    pub max_apk: u32,
    pub max_registry: u32,
}

#[repr(C)]
#[allow(non_camel_case_types)]
pub struct t_pmwork {
    pub sys: [t_pmentry; PM_MAX_PACKAGES as usize],
    pub data: [t_pmentry; DATA_SLOTS],
    pub sys_n: u32,
    pub data_n: u32,
    pub slot: u32,
    pub fresh: u32,
    pub info: t_pminfo,
    pub text: [u8; TEXT_CAP],
    pub reg: [u8; PM_PATH_MAX],
    pub reg_tmp: [u8; PM_PATH_MAX],
    pub apk: [u8; PM_PATH_MAX],
    pub apk_tmp: [u8; PM_PATH_MAX],
}

impl t_pminfo {
    pub(crate) const fn zero() -> Self {
        Self {
            package: [0; PM_PKG_MAX],
            version_code: 0,
            label: [0; PM_LABEL_MAX],
            activity: [0; PM_ACT_MAX],
            cert: [0; PM_CERT_LEN],
        }
    }
}

impl t_pmentry {
    pub(crate) const fn zero() -> Self {
        Self {
            info: t_pminfo::zero(),
            origin: 0,
            path: [0; PM_PATH_MAX],
        }
    }
}

const _: () = assert!(core::mem::size_of::<t_span>() == 16);
const _: () = assert!(core::mem::align_of::<t_span>() == 8);
const _: () = assert!(core::mem::offset_of!(t_span, len) == 8);
const _: () = assert!(core::mem::size_of::<t_text>() == 16);
const _: () = assert!(core::mem::align_of::<t_text>() == 8);
const _: () = assert!(core::mem::offset_of!(t_text, cap) == 8);
const _: () = assert!(core::mem::size_of::<t_pminfo>() == 420);
const _: () = assert!(core::mem::align_of::<t_pminfo>() == 4);
const _: () = assert!(core::mem::offset_of!(t_pminfo, version_code) == 128);
const _: () = assert!(core::mem::offset_of!(t_pminfo, label) == 132);
const _: () = assert!(core::mem::offset_of!(t_pminfo, activity) == 196);
const _: () = assert!(core::mem::offset_of!(t_pminfo, cert) == 388);
const _: () = assert!(core::mem::size_of::<t_pmentry>() == 616);
const _: () = assert!(core::mem::align_of::<t_pmentry>() == 4);
const _: () = assert!(core::mem::offset_of!(t_pmentry, origin) == 420);
const _: () = assert!(core::mem::offset_of!(t_pmentry, path) == 424);
const _: () = assert!(core::mem::size_of::<t_pmreg>() == 24);
const _: () = assert!(core::mem::align_of::<t_pmreg>() == 8);
const _: () = assert!(core::mem::offset_of!(t_pmreg, max) == 8);
const _: () = assert!(core::mem::offset_of!(t_pmreg, n) == 12);
const _: () = assert!(core::mem::offset_of!(t_pmreg, rejected) == 16);
const _: () = assert!(core::mem::size_of::<t_pmfs>() == 40);
const _: () = assert!(core::mem::align_of::<t_pmfs>() == 8);
const _: () = assert!(core::mem::size_of::<t_pm>() == 56);
const _: () = assert!(core::mem::align_of::<t_pm>() == 8);
const _: () = assert!(core::mem::offset_of!(t_pm, data_root) == 8);
const _: () = assert!(core::mem::offset_of!(t_pm, fs) == 16);
const _: () = assert!(core::mem::offset_of!(t_pm, inspect) == 24);
const _: () = assert!(core::mem::offset_of!(t_pm, ctx) == 32);
const _: () = assert!(core::mem::offset_of!(t_pm, max_packages) == 40);
const _: () = assert!(core::mem::offset_of!(t_pm, max_apk) == 44);
const _: () = assert!(core::mem::offset_of!(t_pm, max_registry) == 48);
const _: () = assert!(core::mem::size_of::<t_pmwork>() == 146_208);
const _: () = assert!(core::mem::align_of::<t_pmwork>() == 4);
const _: () = assert!(core::mem::offset_of!(t_pmwork, data) == 39_424);
const _: () = assert!(core::mem::offset_of!(t_pmwork, sys_n) == 79_464);
const _: () = assert!(core::mem::offset_of!(t_pmwork, data_n) == 79_468);
const _: () = assert!(core::mem::offset_of!(t_pmwork, slot) == 79_472);
const _: () = assert!(core::mem::offset_of!(t_pmwork, fresh) == 79_476);
const _: () = assert!(core::mem::offset_of!(t_pmwork, info) == 79_480);
const _: () = assert!(core::mem::offset_of!(t_pmwork, text) == 79_900);
const _: () = assert!(core::mem::offset_of!(t_pmwork, reg) == 145_437);
const _: () = assert!(core::mem::offset_of!(t_pmwork, reg_tmp) == 145_629);
const _: () = assert!(core::mem::offset_of!(t_pmwork, apk) == 145_821);
const _: () = assert!(core::mem::offset_of!(t_pmwork, apk_tmp) == 146_013);

#[cfg(test)]
mod tests;
