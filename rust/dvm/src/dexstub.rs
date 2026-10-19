use core::ffi::{c_char, c_int};

use crate::{
    E_INVAL, E_NOENT, E_NOTSUP, t_darray, t_dcodelimits, t_dex, t_dexcatch, t_dexcdata, t_dexclass,
    t_dexcode, t_dexfield, t_dexhit, t_dexlist, t_dexmember, t_dexmethod, t_dexproto, t_dexstr,
    t_dexvalue, t_dinsn, t_dswitch, t_span, t_text,
};

pub(crate) fn dex_open(_: *mut t_dex, _: t_span) -> c_int {
    E_NOTSUP
}
pub(crate) fn dex_string(_: *const t_dex, _: u32, _: *mut t_dexstr) -> c_int {
    E_INVAL
}
pub(crate) fn dex_type(_: *const t_dex, _: u32, _: *mut t_dexstr) -> c_int {
    E_INVAL
}
pub(crate) fn dex_proto(_: *const t_dex, _: u32, _: *mut t_dexproto) -> c_int {
    E_INVAL
}
pub(crate) fn dex_field(_: *const t_dex, _: u32, _: *mut t_dexfield) -> c_int {
    E_INVAL
}
pub(crate) fn dex_method(_: *const t_dex, _: u32, _: *mut t_dexmethod) -> c_int {
    E_INVAL
}
pub(crate) fn dex_class(_: *const t_dex, _: u32, _: *mut t_dexclass) -> c_int {
    E_INVAL
}
pub(crate) fn dex_find_class(_: *const t_dex, _: *const c_char) -> c_int {
    E_NOENT
}
pub(crate) fn dex_cdata_open(_: *const t_dex, _: *const t_dexclass, _: *mut t_dexcdata) -> c_int {
    E_INVAL
}
pub(crate) fn dex_cdata_next(_: *mut t_dexcdata, _: *mut t_dexmember) -> c_int {
    E_INVAL
}
pub(crate) fn dex_code(_: *const t_dex, _: u32, _: *mut t_dexcode) -> c_int {
    E_INVAL
}
pub(crate) fn dex_try_find(_: *const t_dex, _: *const t_dexcode, _: u32) -> c_int {
    E_INVAL
}
pub(crate) fn dex_handler_open(
    _: *const t_dex,
    _: *const t_dexcode,
    _: u32,
    _: *mut t_dexhit,
) -> c_int {
    E_INVAL
}
pub(crate) fn dex_handler_next(_: *mut t_dexhit, _: *mut t_dexcatch) -> c_int {
    E_INVAL
}
pub(crate) fn dex_static_value(
    _: *const t_dex,
    _: *const t_dexclass,
    _: u32,
    _: *mut t_dexvalue,
) -> c_int {
    E_INVAL
}
pub(crate) fn dex_list_at(_: *const t_dexlist, _: u32) -> u32 {
    0
}
pub(crate) fn mutf8_to_utf8(_: *const c_char, _: t_text) -> c_int {
    E_NOTSUP
}
pub(crate) fn dexcode_decode(_: t_span, _: u32, _: *mut t_dinsn) -> c_int {
    E_INVAL
}
pub(crate) fn dexcode_verify(_: t_span, _: *const t_dcodelimits, _: *mut u8) -> c_int {
    E_INVAL
}
pub(crate) fn dexcode_switch(_: t_span, _: u32, _: *mut t_dswitch) -> c_int {
    E_INVAL
}
pub(crate) fn dexcode_array_data(_: t_span, _: u32, _: *mut t_darray) -> c_int {
    E_INVAL
}
pub(crate) fn dexcode_rd32(_: *const u8) -> i32 {
    0
}
