use core::ffi::{c_char, c_int};

use crate::{t_daacc, t_darray, t_dfacc, t_dinvoke, t_dmethod, t_dnewarr, t_dref, t_dvm};

#[cfg(all(not(test), velum_host, feature = "runtime", not(feature = "interp")))]
use crate::ext::host_call;
#[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
use crate::ext::{
    host_array, host_array_length, host_array_view, host_catches, host_const_class,
    host_const_string, host_field, host_fill_array, host_instance_of, host_new_array,
    host_new_instance, host_resolve, host_throw,
};

pub(crate) unsafe fn go_throw(vm: *mut t_dvm, desc: *const c_char, msg: *const c_char) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: dvm_throw est le symbole du fake hôte, mêmes arguments que le C.
        unsafe { host_throw(vm, desc, msg) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvm_throw.
        unsafe { crate::cls::dvm_throw_inner(vm, desc, msg) }
    }
}

pub(crate) unsafe fn go_call(
    vm: *mut t_dvm,
    m: *const t_dmethod,
    args: *const u32,
    ret: *mut u64,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "runtime", not(feature = "interp")))]
    {
        // SAFETY: dvm_call est le symbole du fake hôte d08.
        unsafe { host_call(vm, m, args, ret) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "runtime", not(feature = "interp"))))]
    {
        // SAFETY: même contrat que dvm_call.
        unsafe { crate::ins::dvm_call_inner(vm, m, args, ret) }
    }
}

pub(crate) unsafe fn go_array_view(
    vm: *mut t_dvm,
    r: t_dref,
    out: *mut crate::t_darrview,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: dvm_array_view est le symbole du fake hôte d09.
        unsafe { host_array_view(vm, r, out) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvm_array_view.
        unsafe { crate::cls::dvm_array_view_inner(vm, r, out) }
    }
}

pub(crate) unsafe fn go_const_string(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    idx: u32,
    out: *mut t_dref,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_const_string(vm, from, idx, out) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_const_string.
        unsafe { crate::dexrt::dvmrt_const_string_inner(vm, from, idx, out) }
    }
}

pub(crate) unsafe fn go_const_class(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    idx: u32,
    out: *mut t_dref,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_const_class(vm, from, idx, out) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_const_class.
        unsafe { crate::dexrt::dvmrt_const_class_inner(vm, from, idx, out) }
    }
}

pub(crate) unsafe fn go_new_instance(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    idx: u32,
    out: *mut t_dref,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_new_instance(vm, from, idx, out) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_new_instance.
        unsafe { crate::dexrt::dvmrt_new_instance_inner(vm, from, idx, out) }
    }
}

pub(crate) unsafe fn go_new_array(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    rq: *mut t_dnewarr,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_new_array(vm, from, rq) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_new_array.
        unsafe { crate::dexrt::dvmrt_new_array_inner(vm, from, rq) }
    }
}

pub(crate) unsafe fn go_instance_of(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    idx: u32,
    obj: t_dref,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_instance_of(vm, from, idx, obj) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_instance_of.
        unsafe { crate::dexrt::dvmrt_instance_of_inner(vm, from, idx, obj) }
    }
}

pub(crate) unsafe fn go_catches(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    idx: u32,
    exc: t_dref,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_catches(vm, from, idx, exc) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_catches.
        unsafe { crate::dexrt::dvmrt_catches_inner(vm, from, idx, exc) }
    }
}

pub(crate) unsafe fn go_field(vm: *mut t_dvm, from: *const t_dmethod, acc: *mut t_dfacc) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_field(vm, from, acc) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_field.
        unsafe { crate::dexrt::dvmrt_field_inner(vm, from, acc) }
    }
}

pub(crate) unsafe fn go_array(vm: *mut t_dvm, acc: *mut t_daacc) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_array(vm, acc) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_array.
        unsafe { crate::dexrt::dvmrt_array_inner(vm, acc) }
    }
}

pub(crate) unsafe fn go_array_length(vm: *mut t_dvm, arr: t_dref, len: *mut u32) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_array_length(vm, arr, len) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_array_length.
        unsafe { crate::dexrt::dvmrt_array_length_inner(vm, arr, len) }
    }
}

pub(crate) unsafe fn go_fill_array(vm: *mut t_dvm, arr: t_dref, data: *const t_darray) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_fill_array(vm, arr, data) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_fill_array.
        unsafe { crate::dexrt::dvmrt_fill_array_inner(vm, arr, data) }
    }
}

pub(crate) unsafe fn go_resolve(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    inv: *mut t_dinvoke,
) -> c_int {
    #[cfg(all(not(test), velum_host, feature = "interp", not(feature = "runtime")))]
    {
        // SAFETY: symbole du fake hôte d09.
        unsafe { host_resolve(vm, from, inv) }
    }
    #[cfg(not(all(not(test), velum_host, feature = "interp", not(feature = "runtime"))))]
    {
        // SAFETY: même contrat que dvmrt_resolve.
        unsafe { crate::dexrt::dvmrt_resolve_inner(vm, from, inv) }
    }
}
