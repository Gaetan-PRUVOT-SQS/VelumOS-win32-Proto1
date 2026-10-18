#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod adler;
mod cdata;
mod classes;
mod code;
mod ffi;
mod ids;
mod mutf8;
mod open;
mod rd;
mod value;

pub use ffi::{
    dex_adler32, dex_cdata_next, dex_cdata_open, dex_class, dex_code, dex_field, dex_find_class,
    dex_handler_next, dex_handler_open, dex_list_at, dex_method, dex_mutf8_check, dex_open,
    dex_proto, dex_sleb, dex_static_value, dex_string, dex_try_find, dex_type, dex_type_list,
    dex_u16, dex_u32, dex_uleb, dex_ulebp1, mutf8_to_utf8,
};

use core::ffi::{c_char, c_int};

pub(crate) const E_OK: c_int = 0;
pub(crate) const E_NOENT: c_int = -2;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_RANGE: c_int = -34;
pub(crate) const E_OVERFLOW: c_int = -75;
pub(crate) const E_NOTSUP: c_int = -95;

pub const DEX_NO_INDEX: u32 = 0xffff_ffff;
pub const DEX_T_STRING: u32 = 0;
pub const DEX_T_TYPE: u32 = 1;
pub const DEX_T_PROTO: u32 = 2;
pub const DEX_T_FIELD: u32 = 3;
pub const DEX_T_METHOD: u32 = 4;
pub const DEX_T_CLASS: u32 = 5;
pub const DEX_M_SFIELD: u32 = 0;
pub const DEX_M_IFIELD: u32 = 1;
pub const DEX_M_DIRECT: u32 = 2;
pub const DEX_M_VIRTUAL: u32 = 3;
pub const DEX_V_BYTE: u32 = 0x00;
pub const DEX_V_SHORT: u32 = 0x02;
pub const DEX_V_CHAR: u32 = 0x03;
pub const DEX_V_INT: u32 = 0x04;
pub const DEX_V_LONG: u32 = 0x06;
pub const DEX_V_FLOAT: u32 = 0x10;
pub const DEX_V_DOUBLE: u32 = 0x11;
pub const DEX_V_STRING: u32 = 0x17;
pub const DEX_V_NULL: u32 = 0x1e;
pub const DEX_V_BOOLEAN: u32 = 0x1f;
pub const ACC_PUBLIC: u32 = 0x1;
pub const ACC_PRIVATE: u32 = 0x2;
pub const ACC_PROTECTED: u32 = 0x4;
pub const ACC_STATIC: u32 = 0x8;
pub const ACC_FINAL: u32 = 0x10;
pub const ACC_SYNCHRONIZED: u32 = 0x20;
pub const ACC_VOLATILE: u32 = 0x40;
pub const ACC_BRIDGE: u32 = 0x40;
pub const ACC_TRANSIENT: u32 = 0x80;
pub const ACC_VARARGS: u32 = 0x80;
pub const ACC_NATIVE: u32 = 0x100;
pub const ACC_INTERFACE: u32 = 0x200;
pub const ACC_ABSTRACT: u32 = 0x400;
pub const ACC_STRICT: u32 = 0x800;
pub const ACC_SYNTHETIC: u32 = 0x1000;
pub const ACC_ANNOTATION: u32 = 0x2000;
pub const ACC_ENUM: u32 = 0x4000;
pub const ACC_CONSTRUCTOR: u32 = 0x10000;
pub const ACC_DECLARED_SYNCHRONIZED: u32 = 0x20000;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_span {
    pub p: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_text {
    pub p: *mut c_char,
    pub cap: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dex {
    pub p: *const u8,
    pub len: usize,
    pub n: [u32; 6],
    pub off: [u32; 6],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexstr {
    pub p: *const c_char,
    pub size: u32,
    pub utf16: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexlist {
    pub p: *const u8,
    pub n: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexproto {
    pub shorty_idx: u32,
    pub return_idx: u32,
    pub params: t_dexlist,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexfield {
    pub class_idx: u32,
    pub type_idx: u32,
    pub name_idx: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexmethod {
    pub class_idx: u32,
    pub proto_idx: u32,
    pub name_idx: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexclass {
    pub class_idx: u32,
    pub access: u32,
    pub superclass_idx: u32,
    pub interfaces: t_dexlist,
    pub source_file_idx: u32,
    pub class_data_off: u32,
    pub static_values_off: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexcdata {
    pub d: *const t_dex,
    pub pos: u32,
    pub left: [u32; 4],
    pub kind: u32,
    pub last: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexmember {
    pub kind: u32,
    pub idx: u32,
    pub access: u32,
    pub code_off: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexcode {
    pub insns: *const u8,
    pub registers: u32,
    pub ins: u32,
    pub outs: u32,
    pub tries: u32,
    pub insns_size: u32,
    pub off: u32,
    pub tries_off: u32,
    pub handlers_off: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexhit {
    pub d: *const t_dex,
    pub pos: u32,
    pub left: u32,
    pub catch_all: u32,
    pub insns_size: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexcatch {
    pub type_idx: u32,
    pub addr: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexvalue {
    pub kind: u32,
    pub i: i64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexcur {
    pub p: *const u8,
    pub len: usize,
    pub pos: usize,
    pub err: c_int,
}

pub(crate) fn ref_in<'a, T>(p: *const T) -> Result<&'a T, c_int> {
    if p.is_null() || !p.is_aligned() {
        return Err(E_INVAL);
    }
    // SAFETY: p non nul et aligné. Le contrat FFI exige un objet vivant le temps de l'appel.
    Ok(unsafe { &*p })
}

pub(crate) fn ref_out<'a, T>(p: *mut T) -> Result<&'a mut T, c_int> {
    if p.is_null() || !p.is_aligned() {
        return Err(E_INVAL);
    }
    // SAFETY: p non nul et aligné. Le contrat FFI exige un objet exclusif le temps de l'appel.
    Ok(unsafe { &mut *p })
}

#[cfg(test)]
mod tests;
