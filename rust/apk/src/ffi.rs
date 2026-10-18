use core::ffi::{c_char, c_int, c_void};

use crate::{t_span, t_zip, t_zipent};

pub(crate) const RES_T_NULL: u32 = 0x00;
pub(crate) const RES_T_REFERENCE: u32 = 0x01;
pub(crate) const RES_T_STRING: u32 = 0x03;
pub(crate) const RES_T_INT_DEC: u32 = 0x10;
pub(crate) const RES_T_INT_HEX: u32 = 0x11;

pub(crate) const AXML_ATTR_LABEL: u32 = 0x0101_0001;
pub(crate) const AXML_ATTR_NAME: u32 = 0x0101_0003;
pub(crate) const AXML_ATTR_MIN_SDK: u32 = 0x0101_020c;
pub(crate) const AXML_ATTR_VERSION_CODE: u32 = 0x0101_021b;
pub(crate) const AXML_ATTR_VERSION_NAME: u32 = 0x0101_021c;
pub(crate) const AXML_ATTR_TARGET_SDK: u32 = 0x0101_0270;

pub(crate) const AXML_START: c_int = 1;
pub(crate) const AXML_END: c_int = 2;

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_text {
    pub p: *mut c_char,
    pub cap: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_respool {
    pub p: *const u8,
    pub size: u32,
    pub hsize: u32,
    pub count: u32,
    pub strings: u32,
    pub utf8: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_axmlattr {
    pub name: u32,
    pub res_id: u32,
    pub raw: u32,
    pub r#type: u32,
    pub data: u32,
    pub ns: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_axml {
    pub file: t_span,
    pub pool: t_respool,
    pub map_off: u32,
    pub map_count: u32,
    pub pos: u32,
    pub depth: u32,
    pub ev: u32,
    pub ev_off: u32,
    pub attr_off: u32,
    pub attr_size: u32,
    pub attr_count: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_resquery {
    pub res_id: u32,
    pub lang: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_arsc {
    pub file: t_span,
    pub strings: t_respool,
    pub body: u32,
    pub has_pool: u32,
    pub packages: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_sha256 {
    pub h: [u32; 8],
    pub total: u64,
    pub buf: [u8; 64],
    pub fill: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub(crate) struct t_rsapub {
    pub n: [u8; 512],
    pub nlen: u32,
    pub e: u32,
}

impl t_text {
    pub(crate) fn from_buf(buf: &mut [u8]) -> Self {
        Self {
            p: buf.as_mut_ptr().cast(),
            cap: buf.len(),
        }
    }
}

impl t_respool {
    pub(crate) const fn zero() -> Self {
        Self {
            p: core::ptr::null(),
            size: 0,
            hsize: 0,
            count: 0,
            strings: 0,
            utf8: 0,
        }
    }
}

impl t_axmlattr {
    pub(crate) const fn zero() -> Self {
        Self {
            name: 0,
            res_id: 0,
            raw: 0,
            r#type: 0,
            data: 0,
            ns: 0,
        }
    }
}

impl t_axml {
    pub(crate) const fn zero() -> Self {
        Self {
            file: t_span::zero(),
            pool: t_respool::zero(),
            map_off: 0,
            map_count: 0,
            pos: 0,
            depth: 0,
            ev: 0,
            ev_off: 0,
            attr_off: 0,
            attr_size: 0,
            attr_count: 0,
        }
    }
}

impl t_arsc {
    pub(crate) const fn zero() -> Self {
        Self {
            file: t_span::zero(),
            strings: t_respool::zero(),
            body: 0,
            has_pool: 0,
            packages: 0,
        }
    }
}

impl t_sha256 {
    pub(crate) const fn zero() -> Self {
        Self {
            h: [0; 8],
            total: 0,
            buf: [0; 64],
            fill: 0,
        }
    }
}

impl t_rsapub {
    pub(crate) const fn zero() -> Self {
        Self {
            n: [0; 512],
            nlen: 0,
            e: 0,
        }
    }
}

const _: () = assert!(core::mem::size_of::<t_text>() == 16);
const _: () = assert!(core::mem::align_of::<t_text>() == 8);
const _: () = assert!(core::mem::offset_of!(t_text, cap) == 8);
const _: () = assert!(core::mem::size_of::<t_respool>() == 32);
const _: () = assert!(core::mem::align_of::<t_respool>() == 8);
const _: () = assert!(core::mem::offset_of!(t_respool, size) == 8);
const _: () = assert!(core::mem::offset_of!(t_respool, utf8) == 24);
const _: () = assert!(core::mem::size_of::<t_axmlattr>() == 24);
const _: () = assert!(core::mem::align_of::<t_axmlattr>() == 4);
const _: () = assert!(core::mem::size_of::<t_axml>() == 88);
const _: () = assert!(core::mem::align_of::<t_axml>() == 8);
const _: () = assert!(core::mem::offset_of!(t_axml, pool) == 16);
const _: () = assert!(core::mem::offset_of!(t_axml, map_off) == 48);
const _: () = assert!(core::mem::offset_of!(t_axml, attr_count) == 80);
const _: () = assert!(core::mem::size_of::<t_resquery>() == 16);
const _: () = assert!(core::mem::align_of::<t_resquery>() == 8);
const _: () = assert!(core::mem::offset_of!(t_resquery, lang) == 8);
const _: () = assert!(core::mem::size_of::<t_arsc>() == 64);
const _: () = assert!(core::mem::align_of::<t_arsc>() == 8);
const _: () = assert!(core::mem::offset_of!(t_arsc, strings) == 16);
const _: () = assert!(core::mem::offset_of!(t_arsc, body) == 48);
const _: () = assert!(core::mem::offset_of!(t_arsc, packages) == 56);
const _: () = assert!(core::mem::size_of::<t_sha256>() == 112);
const _: () = assert!(core::mem::align_of::<t_sha256>() == 8);
const _: () = assert!(core::mem::offset_of!(t_sha256, total) == 32);
const _: () = assert!(core::mem::offset_of!(t_sha256, buf) == 40);
const _: () = assert!(core::mem::offset_of!(t_sha256, fill) == 104);
const _: () = assert!(core::mem::size_of::<t_rsapub>() == 520);
const _: () = assert!(core::mem::align_of::<t_rsapub>() == 4);
const _: () = assert!(core::mem::offset_of!(t_rsapub, nlen) == 512);
const _: () = assert!(core::mem::offset_of!(t_rsapub, e) == 516);

unsafe extern "C" {
    pub(crate) fn zip_open(z: *mut t_zip, file: t_span) -> c_int;
    pub(crate) fn zip_entry(z: *const t_zip, index: u32, out: *mut t_zipent) -> c_int;
    pub(crate) fn zip_find(z: *const t_zip, name: *const c_char, out: *mut t_zipent) -> c_int;
    pub(crate) fn zip_extract(z: *const t_zip, e: *const t_zipent, out: *mut u8, cap: usize)
    -> i64;
    pub(crate) fn zip_name_safe(name: *const c_char, len: usize) -> c_int;

    pub(crate) fn sha256_init(c: *mut t_sha256);
    pub(crate) fn sha256_update(c: *mut t_sha256, data: *const c_void, n: usize);
    pub(crate) fn sha256_final(c: *mut t_sha256, out: *mut u8);
    pub(crate) fn sha256(data: *const c_void, n: usize, out: *mut u8);
    pub(crate) fn ct_equal(a: *const c_void, b: *const c_void, n: usize) -> c_int;

    pub(crate) fn x509_spki(cert: t_span, spki: *mut t_span) -> c_int;
    pub(crate) fn der_spki_rsa(spki: t_span, out: *mut t_rsapub) -> c_int;
    pub(crate) fn rsa_pkcs1_sha256_verify(
        k: *const t_rsapub,
        sig: t_span,
        digest: *const u8,
    ) -> c_int;

    pub(crate) fn axml_open(x: *mut t_axml, file: t_span) -> c_int;
    pub(crate) fn axml_next(x: *mut t_axml) -> c_int;
    pub(crate) fn axml_name(x: *const t_axml, out: t_text) -> c_int;
    pub(crate) fn axml_attr_count(x: *const t_axml) -> u32;
    pub(crate) fn axml_attr(x: *const t_axml, i: u32, out: *mut t_axmlattr) -> c_int;
    pub(crate) fn axml_attr_find(x: *const t_axml, res_id: u32, out: *mut t_axmlattr) -> c_int;
    pub(crate) fn axml_string(x: *const t_axml, index: u32, out: t_text) -> c_int;

    pub(crate) fn arsc_open(a: *mut t_arsc, file: t_span) -> c_int;
    pub(crate) fn arsc_string(a: *const t_arsc, q: *const t_resquery, out: t_text) -> c_int;
}
