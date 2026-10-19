#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]
#![allow(non_camel_case_types)]

mod abi;
mod acopy;
mod act;
mod fail;
mod life;
mod log;
mod math;
mod object;
mod sb;
mod string;
mod tab;
mod text;
mod thr;
mod utf;
mod util;
mod view;
mod widget;

pub use abi::{
    DROID_HORIZONTAL, DROID_KIDS_MAX, DROID_LOG_DEBUG, DROID_LOG_ERROR, DROID_LOG_INFO,
    DROID_LOG_WARN, DROID_TEXT_MAX, DROID_VERTICAL, DROID_VIEWS_MAX, DV_BUTTON, DV_LINEAR, DV_NONE,
    DV_TEXT, DV_VIEW, t_darrview, t_dbuiltin, t_dclass, t_dlimits, t_dmethod, t_dname, t_dnative,
    t_dnativefn, t_droid, t_droidclock, t_droidlog, t_droidview, t_drsb, t_dstr16, t_dvm,
};
pub use acopy::dr_sys_arraycopy;
pub use act::{dr_act_content, dr_act_finish, dr_act_title};
pub use fail::{dr_fail, dr_missing};
pub use life::{dr_release, droid_click, droid_install, droid_start, droid_stop, droid_view};
pub use log::{dr_log_d, dr_log_e, dr_log_i, dr_log_w};
pub use math::{dr_math_abs, dr_math_max, dr_math_min, dr_sys_millis};
pub use object::{dr_nop, dr_obj_equals, dr_obj_hash, dr_obj_tostring};
pub use sb::{
    dr_sb_append_c, dr_sb_append_i, dr_sb_append_j, dr_sb_append_obj, dr_sb_append_z, dr_sb_ascii,
    dr_sb_length, dr_sb_put, dr_sb_tostring,
};
pub use string::{
    dr_int_parse, dr_int_tostring, dr_str_charat, dr_str_concat, dr_str_equals, dr_str_hash,
    dr_str_length, dr_str_self,
};
pub use tab::{dr_classes, dr_tab_android, dr_tab_lang};
pub use text::{dr_cstr, dr_new8, dr_new16, dr_str, dr_text_of};
pub use thr::{dr_thr_init_msg, dr_thr_message};
pub use utf::{dr_dec16, dr_dotted, dr_utf8};
pub use util::{dr_class, dr_host, dr_is, dr_npe};
pub use view::{dr_view_get, dr_view_init, dr_view_listen, dr_view_of};
pub use widget::{dr_ll_orient, dr_tv_gettext, dr_tv_settext, dr_vg_addview};

#[cfg(test)]
mod tests;

use core::mem::{align_of, offset_of, size_of};

const _: () = assert!(size_of::<t_droidview>() == 348 && align_of::<t_droidview>() == 4);
const _: () = assert!(offset_of!(t_droidview, kind) == 0 && offset_of!(t_droidview, id) == 4);
const _: () =
    assert!(offset_of!(t_droidview, parent) == 8 && offset_of!(t_droidview, orientation) == 12);
const _: () = assert!(offset_of!(t_droidview, nkids) == 16 && offset_of!(t_droidview, kids) == 20);
const _: () =
    assert!(offset_of!(t_droidview, self_ref) == 84 && offset_of!(t_droidview, listener) == 88);
const _: () = assert!(offset_of!(t_droidview, text) == 92);

const _: () = assert!(size_of::<t_droid>() == 22840 && align_of::<t_droid>() == 8);
const _: () = assert!(offset_of!(t_droid, vm) == 0 && offset_of!(t_droid, activity) == 8);
const _: () = assert!(offset_of!(t_droid, root) == 12 && offset_of!(t_droid, nviews) == 16);
const _: () = assert!(offset_of!(t_droid, dirty) == 20 && offset_of!(t_droid, finished) == 24);
const _: () = assert!(offset_of!(t_droid, log) == 32 && offset_of!(t_droid, clock) == 40);
const _: () = assert!(offset_of!(t_droid, user) == 48 && offset_of!(t_droid, title) == 56);
const _: () = assert!(offset_of!(t_droid, error) == 312 && offset_of!(t_droid, views) == 568);

const _: () = assert!(size_of::<t_drsb>() == 2052 && align_of::<t_drsb>() == 4);
const _: () = assert!(offset_of!(t_drsb, len) == 0 && offset_of!(t_drsb, buf) == 4);

const _: () = assert!(size_of::<t_dbuiltin>() == 32 && align_of::<t_dbuiltin>() == 8);
const _: () = assert!(offset_of!(t_dbuiltin, desc) == 0 && offset_of!(t_dbuiltin, super_desc) == 8);
const _: () =
    assert!(offset_of!(t_dbuiltin, iface_desc) == 16 && offset_of!(t_dbuiltin, access) == 24);
const _: () = assert!(offset_of!(t_dbuiltin, payload_bytes) == 28);

const _: () = assert!(size_of::<t_dnative>() == 40 && align_of::<t_dnative>() == 8);
const _: () = assert!(offset_of!(t_dnative, cls) == 0 && offset_of!(t_dnative, name) == 8);
const _: () = assert!(offset_of!(t_dnative, sig) == 16 && offset_of!(t_dnative, access) == 24);
const _: () = assert!(offset_of!(t_dnative, r#fn) == 32);

const _: () = assert!(size_of::<t_dname>() == 16 && align_of::<t_dname>() == 8);
const _: () = assert!(offset_of!(t_dname, name) == 0 && offset_of!(t_dname, sig) == 8);

const _: () = assert!(size_of::<t_dstr16>() == 16 && align_of::<t_dstr16>() == 8);
const _: () = assert!(offset_of!(t_dstr16, p) == 0 && offset_of!(t_dstr16, n) == 8);

const _: () = assert!(size_of::<t_darrview>() == 16 && align_of::<t_darrview>() == 8);
const _: () = assert!(offset_of!(t_darrview, data) == 0 && offset_of!(t_darrview, len) == 8);
const _: () = assert!(offset_of!(t_darrview, width) == 12 && offset_of!(t_darrview, is_ref) == 13);

const _: () = assert!(size_of::<t_dlimits>() == 24 && align_of::<t_dlimits>() == 8);
const _: () =
    assert!(offset_of!(t_dlimits, heap_bytes) == 0 && offset_of!(t_dlimits, stack_words) == 4);
const _: () =
    assert!(offset_of!(t_dlimits, depth_max) == 8 && offset_of!(t_dlimits, classes_max) == 12);
const _: () = assert!(offset_of!(t_dlimits, budget) == 16);

const _: () = assert!(size_of::<t_dvm>() == 352 && align_of::<t_dvm>() == 8);
const _: () = assert!(offset_of!(t_dvm, heap) == 0 && offset_of!(t_dvm, classes) == 8);
const _: () = assert!(offset_of!(t_dvm, host) == 16 && offset_of!(t_dvm, stack) == 24);
const _: () = assert!(offset_of!(t_dvm, stack_words) == 32 && offset_of!(t_dvm, sp) == 36);
const _: () = assert!(offset_of!(t_dvm, depth) == 40 && offset_of!(t_dvm, pending) == 44);
const _: () = assert!(offset_of!(t_dvm, result) == 48 && offset_of!(t_dvm, spent) == 56);
const _: () = assert!(offset_of!(t_dvm, lim) == 64 && offset_of!(t_dvm, locals) == 88);
const _: () = assert!(offset_of!(t_dvm, nlocals) == 344);
