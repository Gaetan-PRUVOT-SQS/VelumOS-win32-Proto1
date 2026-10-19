#[cfg(not(test))]
use core::ffi::{c_char, c_int};

#[cfg(not(test))]
use crate::{
    t_darray, t_dcodelimits, t_dex, t_dexcatch, t_dexcdata, t_dexclass, t_dexcode, t_dexfield,
    t_dexhit, t_dexmember, t_dexmethod, t_dexproto, t_dexstr, t_dexvalue, t_dinsn, t_dswitch,
    t_span, t_text,
};

#[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
use crate::{t_daacc, t_dinvoke, t_dmethod, t_dnewarr, t_dref, t_dvm};
#[cfg(all(not(test), velum_host, feature = "runtime", not(feature = "interp")))]
use crate::{t_dmethod, t_dvm};

#[cfg(not(test))]
unsafe extern "C" {
    pub fn dex_open(d: *mut t_dex, file: t_span) -> c_int;
    pub fn dex_string(d: *const t_dex, idx: u32, out: *mut t_dexstr) -> c_int;
    pub fn dex_type(d: *const t_dex, type_idx: u32, out: *mut t_dexstr) -> c_int;
    pub fn dex_proto(d: *const t_dex, proto_idx: u32, out: *mut t_dexproto) -> c_int;
    pub fn dex_field(d: *const t_dex, field_idx: u32, out: *mut t_dexfield) -> c_int;
    pub fn dex_method(d: *const t_dex, method_idx: u32, out: *mut t_dexmethod) -> c_int;
    pub fn dex_class(d: *const t_dex, def_idx: u32, out: *mut t_dexclass) -> c_int;
    pub fn dex_find_class(d: *const t_dex, descriptor: *const c_char) -> c_int;
    pub fn dex_cdata_open(d: *const t_dex, c: *const t_dexclass, it: *mut t_dexcdata) -> c_int;
    pub fn dex_cdata_next(it: *mut t_dexcdata, out: *mut t_dexmember) -> c_int;
    pub fn dex_code(d: *const t_dex, code_off: u32, out: *mut t_dexcode) -> c_int;
    pub fn dex_try_find(d: *const t_dex, code: *const t_dexcode, pc: u32) -> c_int;
    pub fn dex_handler_open(
        d: *const t_dex,
        code: *const t_dexcode,
        try_index: u32,
        it: *mut t_dexhit,
    ) -> c_int;
    pub fn dex_handler_next(it: *mut t_dexhit, out: *mut t_dexcatch) -> c_int;
    pub fn dex_static_value(
        d: *const t_dex,
        c: *const t_dexclass,
        ordinal: u32,
        out: *mut t_dexvalue,
    ) -> c_int;
    pub fn dex_list_at(l: *const crate::t_dexlist, i: u32) -> u32;
    pub fn mutf8_to_utf8(input: *const c_char, out: t_text) -> c_int;
    pub fn dexcode_decode(insns: t_span, pc: u32, out: *mut t_dinsn) -> c_int;
    pub fn dexcode_verify(insns: t_span, lim: *const t_dcodelimits, starts: *mut u8) -> c_int;
    pub fn dexcode_switch(insns: t_span, payload_pc: u32, out: *mut t_dswitch) -> c_int;
    pub fn dexcode_array_data(insns: t_span, payload_pc: u32, out: *mut t_darray) -> c_int;
    pub fn dexcode_rd32(p: *const u8) -> i32;
}

#[cfg(all(not(test), velum_host, feature = "runtime", not(feature = "interp")))]
unsafe extern "C" {
    pub fn dvm_call(vm: *mut t_dvm, m: *const t_dmethod, args: *const u32, ret: *mut u64) -> c_int;
}

#[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
unsafe extern "C" {
    pub fn dvm_throw(vm: *mut t_dvm, desc: *const c_char, msg: *const c_char) -> c_int;
    pub fn dvm_array_view(vm: *mut t_dvm, r: t_dref, out: *mut crate::t_darrview) -> c_int;
    pub fn dvmrt_const_string(
        vm: *mut t_dvm,
        from: *const t_dmethod,
        idx: u32,
        out: *mut t_dref,
    ) -> c_int;
    pub fn dvmrt_const_class(
        vm: *mut t_dvm,
        from: *const t_dmethod,
        idx: u32,
        out: *mut t_dref,
    ) -> c_int;
    pub fn dvmrt_new_instance(
        vm: *mut t_dvm,
        from: *const t_dmethod,
        idx: u32,
        out: *mut t_dref,
    ) -> c_int;
    pub fn dvmrt_new_array(vm: *mut t_dvm, from: *const t_dmethod, rq: *mut t_dnewarr) -> c_int;
    pub fn dvmrt_instance_of(
        vm: *mut t_dvm,
        from: *const t_dmethod,
        idx: u32,
        obj: t_dref,
    ) -> c_int;
    pub fn dvmrt_catches(vm: *mut t_dvm, from: *const t_dmethod, idx: u32, exc: t_dref) -> c_int;
    pub fn dvmrt_field(vm: *mut t_dvm, from: *const t_dmethod, acc: *mut crate::t_dfacc) -> c_int;
    pub fn dvmrt_array(vm: *mut t_dvm, acc: *mut t_daacc) -> c_int;
    pub fn dvmrt_array_length(vm: *mut t_dvm, arr: t_dref, len: *mut u32) -> c_int;
    pub fn dvmrt_fill_array(vm: *mut t_dvm, arr: t_dref, data: *const t_darray) -> c_int;
    pub fn dvmrt_resolve(vm: *mut t_dvm, from: *const t_dmethod, inv: *mut t_dinvoke) -> c_int;
}

#[cfg(all(not(test), velum_host, feature = "runtime", not(feature = "interp")))]
pub(crate) use crate::ext::dvm_call as host_call;

#[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
pub(crate) use crate::ext::{
    dvm_array_view as host_array_view, dvm_throw as host_throw, dvmrt_array as host_array,
    dvmrt_array_length as host_array_length, dvmrt_catches as host_catches,
    dvmrt_const_class as host_const_class, dvmrt_const_string as host_const_string,
    dvmrt_field as host_field, dvmrt_fill_array as host_fill_array,
    dvmrt_instance_of as host_instance_of, dvmrt_new_array as host_new_array,
    dvmrt_new_instance as host_new_instance, dvmrt_resolve as host_resolve,
};
