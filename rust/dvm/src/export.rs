use core::ffi::{c_char, c_int, c_void};

use crate::{
    s_dclasses, s_dheap, t_daacc, t_dbuiltin, t_dclass, t_dex, t_dexclass, t_dexmember, t_dexproto,
    t_dheapstats, t_dlimits, t_dmethod, t_dname, t_dnative, t_dnewobj, t_dobj, t_dref, t_dstr16,
    t_dvm, t_span, t_text,
};

#[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
use crate::{t_darray, t_darrview, t_dfacc, t_dinvoke, t_dnewarr};

macro_rules! cexp {
    ($(#[$meta:meta])* fn $name:ident($($p:ident: $t:ty),* $(,)?) -> $ret:ty = $path:path;) => {
        $(#[$meta])*
        #[unsafe(no_mangle)]
        pub unsafe extern "C" fn $name($($p: $t),*) -> $ret {
            // SAFETY: le corps Rust tient le contrat C du même nom.
            unsafe { $path($($p),*) }
        }
    };
    ($(#[$meta:meta])* fn $name:ident($($p:ident: $t:ty),* $(,)?) = $path:path;) => {
        $(#[$meta])*
        #[unsafe(no_mangle)]
        pub unsafe extern "C" fn $name($($p: $t),*) {
            // SAFETY: le corps Rust tient le contrat C du même nom.
            unsafe { $path($($p),*) }
        }
    };
}

cexp!(fn rt_obj(vm: *const t_dvm, r: t_dref, kind: u8) -> *mut t_dobj = crate::heap::rt_obj;);
cexp!(fn rt_words(o: *const t_dobj) -> *mut u32 = crate::heap::rt_words;);
cexp!(fn rt_grow(h: *mut s_dheap) -> c_int = crate::heap::rt_grow;);
cexp!(fn rt_oom_retype(vm: *mut t_dvm) = crate::cls::rt_oom_retype;);
cexp!(fn rt_release(h: *mut s_dheap, i: u32) = crate::heap::rt_release;);
cexp!(fn rt_alloc(vm: *mut t_dvm, rq: *const t_dnewobj, out: *mut t_dref) -> c_int = crate::heap::rt_alloc;);
cexp!(fn rt_find(vm: *const t_dvm, desc: *const c_char) -> *mut t_dclass = crate::cls::rt_find;);
cexp!(fn rt_define(vm: *mut t_dvm, b: *const t_dbuiltin, out: *mut *mut t_dclass) -> c_int = crate::cls::rt_define;);
cexp!(fn rt_assignable(from: *const t_dclass, to: *const t_dclass) -> c_int = crate::cls::rt_assignable;);
cexp!(fn rt_array_class(vm: *mut t_dvm, desc: *const c_char, out: *mut *mut t_dclass) -> c_int = crate::cls::rt_array_class;);
cexp!(fn rt_exc_new(vm: *mut t_dvm, c: *mut t_dclass, msg: *const c_char, out: *mut t_dref) -> c_int = crate::cls::rt_exc_new;);
cexp!(fn rt_utf8_to16(utf8: *const c_char, out: *mut u16) -> c_int = crate::cls::rt_utf8_to16;);
cexp!(fn rt_utf16_to8(s: t_dstr16, out: t_text) -> c_int = crate::cls::rt_utf16_to8;);
cexp!(fn rt_mark(vm: *const t_dvm, value: u32) = crate::heap::rt_mark;);
cexp!(fn rt_roots_classes(vm: *const t_dvm) = crate::heap::rt_roots_classes;);
cexp!(fn rt_dup(s: *const c_char) -> *mut c_char = crate::cls::rt_dup;);
cexp!(fn rt_class_free(c: *mut t_dclass) = crate::heap::rt_class_free;);
cexp!(fn rt_dstr(d: *const t_dex, idx: u32) -> *const c_char = crate::dexrt::rt_dstr;);
cexp!(fn rt_dtype(d: *const t_dex, idx: u32) -> *const c_char = crate::dexrt::rt_dtype;);
cexp!(fn rt_sig_words(s: *const c_char) -> u16 = crate::dexrt::rt_sig_words;);
cexp!(fn rt_sig_build(d: *const t_dex, p: *const t_dexproto) -> *mut c_char = crate::dexrt::rt_sig_build;);
cexp!(fn rt_verr(vm: *mut t_dvm, c: *const t_dclass) -> c_int = crate::dexrt::rt_verr;);
cexp!(fn rt_method_own(vm: *const t_dvm, c: *const t_dclass, nm: *const t_dname) -> *mut t_dmethod = crate::dexrt::rt_method_own;);
cexp!(fn rt_method_find(vm: *const t_dvm, c: *const t_dclass, nm: *const t_dname) -> *mut t_dmethod = crate::dexrt::rt_method_find;);
cexp!(fn rt_clinit(vm: *mut t_dvm, c: *mut t_dclass) -> c_int = crate::dexrt::rt_clinit;);
cexp!(fn rt_link(vm: *mut t_dvm, desc: *const c_char, out: *mut *mut t_dclass) -> c_int = crate::dexrt::rt_link;);
cexp!(fn rt_members(vm: *mut t_dvm, dc: *const t_dexclass, c: *mut t_dclass) -> c_int = crate::dexrt::rt_members;);
cexp!(fn rt_method_fill(vm: *mut t_dvm, mb: *const t_dexmember, c: *mut t_dclass, i: u32) -> c_int = crate::dexrt::rt_method_fill;);
cexp!(fn rt_verify(vm: *mut t_dvm, c: *mut t_dclass) -> c_int = crate::dexrt::rt_verify;);
cexp!(fn rt_intern(vm: *mut t_dvm, idx: u32, out: *mut t_dref) -> c_int = crate::dexrt::rt_intern;);
cexp!(fn rt_statics_init(vm: *mut t_dvm, dc: *const t_dexclass, c: *mut t_dclass) -> c_int = crate::dexrt::rt_statics_init;);
cexp!(fn rt_cat(buf: *mut c_char, n: usize, s: *const c_char) -> usize = crate::dexrt::rt_cat;);
cexp!(fn rt_class_idx(vm: *mut t_dvm, type_idx: u32, out: *mut *mut t_dclass) -> c_int = crate::dexrt::rt_class_idx;);
cexp!(fn rt_aput(o: *const t_dobj, a: *const t_daacc) = crate::dexrt::rt_aput;);
cexp!(fn dvm_create(out: *mut *mut t_dvm, lim: *const t_dlimits) -> c_int = crate::cls::dvm_create_inner;);
cexp!(fn dvm_destroy(vm: *mut t_dvm) = crate::heap::dvm_destroy_inner;);
cexp!(fn dvm_load_dex(vm: *mut t_dvm, dex: t_span) -> c_int = crate::dexrt::dvm_load_dex;);
cexp!(fn dvm_builtins(vm: *mut t_dvm, tab: *const t_dbuiltin, n: u32) -> c_int = crate::cls::dvm_builtins_inner;);
cexp!(fn dvm_natives(vm: *mut t_dvm, tab: *const t_dnative, n: u32) -> c_int = crate::dexrt::dvm_natives;);
cexp!(fn dvm_class(vm: *mut t_dvm, desc: *const c_char, out: *mut *mut t_dclass) -> c_int = crate::cls::dvm_class_inner;);
cexp!(fn dvm_method(vm: *mut t_dvm, c: *mut t_dclass, name: *const t_dname, out: *mut *const t_dmethod) -> c_int = crate::dexrt::dvm_method;);
cexp!(fn dvm_new(vm: *mut t_dvm, c: *mut t_dclass, out: *mut t_dref) -> c_int = crate::heap::dvm_new_inner;);
cexp!(fn dvm_class_of(vm: *mut t_dvm, r: t_dref) -> *mut t_dclass = crate::heap::dvm_class_of_inner;);
cexp!(fn dvm_class_name(c: *const t_dclass) -> *const c_char = crate::cls::dvm_class_name_inner;);
cexp!(fn dvm_is_instance(vm: *mut t_dvm, r: t_dref, c: *const t_dclass) -> c_int = crate::cls::dvm_is_instance_inner;);
cexp!(fn dvm_payload(vm: *mut t_dvm, r: t_dref, builtin: *const t_dclass) -> *mut c_void = crate::heap::dvm_payload_inner;);
cexp!(fn dvm_string_utf8_new(vm: *mut t_dvm, utf8: *const c_char, out: *mut t_dref) -> c_int = crate::cls::dvm_string_utf8_new_inner;);
cexp!(fn dvm_string_utf16_new(vm: *mut t_dvm, s: t_dstr16, out: *mut t_dref) -> c_int = crate::cls::dvm_string_utf16_new_inner;);
cexp!(fn dvm_string_get(vm: *mut t_dvm, r: t_dref, out: *mut t_dstr16) -> c_int = crate::cls::dvm_string_get_inner;);
cexp!(fn dvm_string_utf8(vm: *mut t_dvm, r: t_dref, out: t_text) -> c_int = crate::cls::dvm_string_utf8_inner;);
cexp!(fn dvm_array_new(vm: *mut t_dvm, desc: *const c_char, len: i32, out: *mut t_dref) -> c_int = crate::cls::dvm_array_new_inner;);
cexp!(fn dvm_local(vm: *mut t_dvm, r: t_dref) -> c_int = crate::heap::dvm_local_inner;);
cexp!(fn dvm_local_pop(vm: *mut t_dvm, n: u32) = crate::heap::dvm_local_pop_inner;);
cexp!(fn dvm_pin(vm: *mut t_dvm, r: t_dref) -> c_int = crate::heap::dvm_pin_inner;);
cexp!(fn dvm_unpin(vm: *mut t_dvm, r: t_dref) = crate::heap::dvm_unpin_inner;);
cexp!(fn dvm_gc(vm: *mut t_dvm) = crate::heap::dvm_gc_inner;);
cexp!(fn dvm_heap_stats(vm: *const t_dvm, out: *mut t_dheapstats) = crate::heap::dvm_heap_stats_inner;);
cexp!(fn dvm_throwable_message(vm: *mut t_dvm, exc: t_dref, msg: *mut t_dref) -> c_int = crate::cls::dvm_throwable_message_inner;);
cexp!(fn dvm_throwable_set_message(vm: *mut t_dvm, exc: t_dref, msg: t_dref) -> c_int = crate::cls::dvm_throwable_set_message_inner;);
cexp!(fn dvm_method_start(m: *const t_dmethod, pc: u32) -> c_int = crate::dexrt::dvm_method_start;);

cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvm_array_view(vm: *mut t_dvm, r: t_dref, out: *mut t_darrview) -> c_int = crate::cls::dvm_array_view_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvm_throw(vm: *mut t_dvm, desc: *const c_char, msg: *const c_char) -> c_int = crate::cls::dvm_throw_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "runtime", not(feature = "interp"))))]
    fn dvm_call(vm: *mut t_dvm, m: *const t_dmethod, args: *const u32, ret: *mut u64) -> c_int = crate::ins::dvm_call_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_const_string(vm: *mut t_dvm, from: *const t_dmethod, idx: u32, out: *mut t_dref) -> c_int = crate::dexrt::dvmrt_const_string_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_const_class(vm: *mut t_dvm, from: *const t_dmethod, idx: u32, out: *mut t_dref) -> c_int = crate::dexrt::dvmrt_const_class_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_new_instance(vm: *mut t_dvm, from: *const t_dmethod, idx: u32, out: *mut t_dref) -> c_int = crate::dexrt::dvmrt_new_instance_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_new_array(vm: *mut t_dvm, from: *const t_dmethod, rq: *mut t_dnewarr) -> c_int = crate::dexrt::dvmrt_new_array_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_instance_of(vm: *mut t_dvm, from: *const t_dmethod, idx: u32, obj: t_dref) -> c_int = crate::dexrt::dvmrt_instance_of_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_catches(vm: *mut t_dvm, from: *const t_dmethod, idx: u32, exc: t_dref) -> c_int = crate::dexrt::dvmrt_catches_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_field(vm: *mut t_dvm, from: *const t_dmethod, acc: *mut t_dfacc) -> c_int = crate::dexrt::dvmrt_field_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_array(vm: *mut t_dvm, acc: *mut t_daacc) -> c_int = crate::dexrt::dvmrt_array_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_array_length(vm: *mut t_dvm, arr: t_dref, len: *mut u32) -> c_int = crate::dexrt::dvmrt_array_length_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_fill_array(vm: *mut t_dvm, arr: t_dref, data: *const t_darray) -> c_int = crate::dexrt::dvmrt_fill_array_inner;
);
cexp!(
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    fn dvmrt_resolve(vm: *mut t_dvm, from: *const t_dmethod, inv: *mut t_dinvoke) -> c_int = crate::dexrt::dvmrt_resolve_inner;
);

const _: usize = core::mem::size_of::<s_dclasses>();
