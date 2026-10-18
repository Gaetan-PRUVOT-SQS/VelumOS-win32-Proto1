#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod abi;
mod geom;
mod high;
mod low;
mod sys;

pub use abi::*;
pub use high::*;
pub use low::*;
pub use sys::*;

#[cfg(test)]
mod tests;

use core::mem::{align_of, offset_of, size_of};

const _: () = assert!(size_of::<t_rect>() == 16 && align_of::<t_rect>() == 4);
const _: () = assert!(offset_of!(t_rect, x) == 0 && offset_of!(t_rect, y) == 4);
const _: () = assert!(offset_of!(t_rect, w) == 8 && offset_of!(t_rect, h) == 12);

const _: () = assert!(size_of::<t_surface>() == 40 && align_of::<t_surface>() == 8);
const _: () = assert!(offset_of!(t_surface, px) == 0 && offset_of!(t_surface, w) == 8);
const _: () = assert!(offset_of!(t_surface, h) == 12 && offset_of!(t_surface, stride) == 16);
const _: () = assert!(offset_of!(t_surface, clip) == 20);

const _: () = assert!(size_of::<t_inpevent>() == 32 && align_of::<t_inpevent>() == 8);
const _: () = assert!(offset_of!(t_inpevent, r#type) == 0 && offset_of!(t_inpevent, code) == 4);
const _: () = assert!(offset_of!(t_inpevent, x) == 8 && offset_of!(t_inpevent, y) == 12);
const _: () = assert!(offset_of!(t_inpevent, mods) == 16 && offset_of!(t_inpevent, scancode) == 20);
const _: () = assert!(offset_of!(t_inpevent, time_ns) == 24);

const _: () = assert!(size_of::<abi::t_chansend>() == 32 && align_of::<abi::t_chansend>() == 8);
const _: () = assert!(offset_of!(abi::t_chansend, msg) == 0);
const _: () = assert!(offset_of!(abi::t_chansend, len) == 8);
const _: () = assert!(offset_of!(abi::t_chansend, handles) == 16);
const _: () = assert!(offset_of!(abi::t_chansend, nhandles) == 24);
const _: () = assert!(offset_of!(abi::t_chansend, reserved) == 28);

const _: () = assert!(size_of::<abi::t_chanrecv>() == 56 && align_of::<abi::t_chanrecv>() == 8);
const _: () = assert!(offset_of!(abi::t_chanrecv, buf) == 0);
const _: () = assert!(offset_of!(abi::t_chanrecv, buf_len) == 8);
const _: () = assert!(offset_of!(abi::t_chanrecv, handles) == 16);
const _: () = assert!(offset_of!(abi::t_chanrecv, handles_max) == 24);
const _: () = assert!(offset_of!(abi::t_chanrecv, reserved) == 28);
const _: () = assert!(offset_of!(abi::t_chanrecv, timeout_ns) == 32);
const _: () = assert!(offset_of!(abi::t_chanrecv, out_len) == 40);
const _: () = assert!(offset_of!(abi::t_chanrecv, out_nhandles) == 48);
const _: () = assert!(offset_of!(abi::t_chanrecv, reserved2) == 52);

const _: () = assert!(size_of::<t_wmhdr>() == 24 && align_of::<t_wmhdr>() == 4);
const _: () = assert!(offset_of!(t_wmhdr, magic) == 0 && offset_of!(t_wmhdr, r#type) == 4);
const _: () = assert!(offset_of!(t_wmhdr, size) == 8 && offset_of!(t_wmhdr, window) == 12);
const _: () = assert!(offset_of!(t_wmhdr, seq) == 16 && offset_of!(t_wmhdr, status) == 20);

const _: () = assert!(size_of::<t_wmhello>() == 52 && align_of::<t_wmhello>() == 4);
const _: () = assert!(offset_of!(t_wmhello, h) == 0 && offset_of!(t_wmhello, version) == 24);
const _: () =
    assert!(offset_of!(t_wmhello, screen_w) == 28 && offset_of!(t_wmhello, screen_h) == 32);
const _: () = assert!(offset_of!(t_wmhello, workarea) == 36);

const _: () = assert!(size_of::<t_wmcreate>() == 112 && align_of::<t_wmcreate>() == 4);
const _: () = assert!(offset_of!(t_wmcreate, h) == 0 && offset_of!(t_wmcreate, rect) == 24);
const _: () = assert!(offset_of!(t_wmcreate, style) == 40 && offset_of!(t_wmcreate, state) == 44);
const _: () = assert!(offset_of!(t_wmcreate, title) == 48);

const _: () = assert!(size_of::<t_wmcreated>() == 40 && align_of::<t_wmcreated>() == 4);
const _: () = assert!(offset_of!(t_wmcreated, h) == 0 && offset_of!(t_wmcreated, stride) == 24);
const _: () =
    assert!(offset_of!(t_wmcreated, width) == 28 && offset_of!(t_wmcreated, height) == 32);
const _: () = assert!(offset_of!(t_wmcreated, reserved) == 36);

const _: () = assert!(size_of::<t_wmpresent>() == 284 && align_of::<t_wmpresent>() == 4);
const _: () = assert!(offset_of!(t_wmpresent, h) == 0 && offset_of!(t_wmpresent, nrects) == 24);
const _: () = assert!(offset_of!(t_wmpresent, rects) == 28);

const _: () = assert!(size_of::<t_wmkey>() == 56 && align_of::<t_wmkey>() == 8);
const _: () = assert!(offset_of!(t_wmkey, h) == 0 && offset_of!(t_wmkey, ev) == 24);

const _: () = assert!(size_of::<t_wmmouse>() == 48 && align_of::<t_wmmouse>() == 4);
const _: () = assert!(offset_of!(t_wmmouse, h) == 0 && offset_of!(t_wmmouse, r#type) == 24);
const _: () = assert!(offset_of!(t_wmmouse, x) == 28 && offset_of!(t_wmmouse, y) == 32);
const _: () = assert!(offset_of!(t_wmmouse, buttons) == 36 && offset_of!(t_wmmouse, wheel) == 40);
const _: () = assert!(offset_of!(t_wmmouse, hit) == 44);

const _: () = assert!(size_of::<t_wminfo>() == 108 && align_of::<t_wminfo>() == 4);
const _: () = assert!(offset_of!(t_wminfo, h) == 0 && offset_of!(t_wminfo, pid) == 24);
const _: () = assert!(offset_of!(t_wminfo, style) == 28 && offset_of!(t_wminfo, state) == 32);
const _: () = assert!(offset_of!(t_wminfo, icon) == 36 && offset_of!(t_wminfo, active) == 40);
const _: () = assert!(offset_of!(t_wminfo, title) == 44);

const _: () = assert!(size_of::<t_wmtheme>() == 36 && align_of::<t_wmtheme>() == 4);
const _: () = assert!(offset_of!(t_wmtheme, h) == 0 && offset_of!(t_wmtheme, dark) == 24);
const _: () =
    assert!(offset_of!(t_wmtheme, reduced) == 28 && offset_of!(t_wmtheme, reserved) == 32);

const _: () = assert!(size_of::<t_wmwin>() == 64 && align_of::<t_wmwin>() == 8);
const _: () = assert!(offset_of!(t_wmwin, id) == 0 && offset_of!(t_wmwin, chan) == 4);
const _: () = assert!(offset_of!(t_wmwin, section) == 8 && offset_of!(t_wmwin, surface) == 16);
const _: () = assert!(offset_of!(t_wmwin, style) == 56);

const _: () = assert!(size_of::<t_wmarg>() == 32 && align_of::<t_wmarg>() == 4);
const _: () = assert!(offset_of!(t_wmarg, h) == 0 && offset_of!(t_wmarg, value) == 24);
const _: () = assert!(offset_of!(t_wmarg, reserved) == 28);

const _: () = assert!(size_of::<t_wmrect>() == 40 && align_of::<t_wmrect>() == 4);
const _: () = assert!(offset_of!(t_wmrect, h) == 0 && offset_of!(t_wmrect, rect) == 24);

const _: () = assert!(size_of::<t_wmtitle>() == 88 && align_of::<t_wmtitle>() == 4);
const _: () = assert!(offset_of!(t_wmtitle, h) == 0 && offset_of!(t_wmtitle, title) == 24);

const _: () = assert!(size_of::<t_wmcmsg>() == 328 && align_of::<t_wmcmsg>() == 8);
const _: () = assert!(offset_of!(t_wmcmsg, buf) == 0 && offset_of!(t_wmcmsg, len) == 320);
const _: () = assert!(offset_of!(t_wmcmsg, handle) == 324);

const _: () = assert!(size_of::<t_wmcstate>() == 11408 && align_of::<t_wmcstate>() == 8);
const _: () = assert!(offset_of!(t_wmcstate, chan) == 0 && offset_of!(t_wmcstate, seq) == 4);
const _: () = assert!(offset_of!(t_wmcstate, lost) == 8 && offset_of!(t_wmcstate, info) == 12);
const _: () = assert!(offset_of!(t_wmcstate, head) == 64 && offset_of!(t_wmcstate, n) == 68);
const _: () = assert!(offset_of!(t_wmcstate, stash) == 72 && offset_of!(t_wmcstate, rx) == 10568);
const _: () = assert!(offset_of!(t_wmcstate, wins) == 10896);
const _: () = assert!(size_of::<WmCell<t_wmcstate>>() == 11408);
const _: () = assert!(align_of::<WmCell<t_wmcstate>>() == 8);
