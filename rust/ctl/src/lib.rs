#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod abi;
mod clip;
mod color;
mod edit;
mod ffi;
mod focus;
mod key;
mod list;
mod mem;
mod menu;
mod mouse;
mod notify;
mod ops;
mod paint;
mod radio;
mod raw;
mod scroll;
mod text;
mod tree;
mod utf8;
mod widgets;

#[cfg(not(test))]
mod anchor;
#[cfg(not(test))]
pub use anchor::anchor_syms;

#[cfg(test)]
mod stub;

#[cfg(test)]
mod tests;

use core::mem::{align_of, offset_of, size_of};

use abi::{
    t_clip, t_ctl, t_ctlmenuitem, t_ctlmenuspec, t_ctlops, t_ctlroot, t_ctlspec, t_ctltext, t_edit,
    t_edview, t_focusscan, t_font, t_glyph, t_inpevent, t_list, t_lstgeom, t_lunabtn,
    t_lunametrics, t_menu, t_menulevel, t_mouseev, t_point, t_pool, t_progress, t_rect, t_region,
    t_rootpriv, t_sbctx, t_sblo, t_scrst, t_surface, t_textreq, t_textrun, t_wmhdr, t_wmmouse,
};

const _: () = assert!(size_of::<t_point>() == 8 && align_of::<t_point>() == 4);
const _: () = assert!(size_of::<t_rect>() == 16 && align_of::<t_rect>() == 4);
const _: () = assert!(size_of::<t_surface>() == 40 && align_of::<t_surface>() == 8);
const _: () = assert!(size_of::<t_region>() == 516 && align_of::<t_region>() == 4);
const _: () = assert!(size_of::<t_glyph>() == 12 && align_of::<t_glyph>() == 4);
const _: () = assert!(size_of::<t_font>() == 40 && align_of::<t_font>() == 8);
const _: () = assert!(size_of::<t_textreq>() == 40 && align_of::<t_textreq>() == 8);
const _: () = assert!(size_of::<t_lunametrics>() == 48 && align_of::<t_lunametrics>() == 4);
const _: () = assert!(size_of::<t_lunabtn>() == 24 && align_of::<t_lunabtn>() == 4);
const _: () = assert!(size_of::<t_inpevent>() == 32 && align_of::<t_inpevent>() == 8);
const _: () = assert!(size_of::<t_wmmouse>() == 48 && align_of::<t_wmmouse>() == 4);
const _: () = assert!(size_of::<t_wmhdr>() == 24 && align_of::<t_wmhdr>() == 4);
const _: () = assert!(size_of::<t_ctl>() == 344 && align_of::<t_ctl>() == 8);
const _: () = assert!(size_of::<t_ctlroot>() == 576 && align_of::<t_ctlroot>() == 8);
const _: () = assert!(size_of::<t_ctlspec>() == 40 && align_of::<t_ctlspec>() == 8);
const _: () = assert!(size_of::<t_ctlmenuitem>() == 32 && align_of::<t_ctlmenuitem>() == 8);
const _: () = assert!(size_of::<t_ctlmenuspec>() == 32 && align_of::<t_ctlmenuspec>() == 8);
const _: () = assert!(size_of::<t_mouseev>() == 20 && align_of::<t_mouseev>() == 4);
const _: () = assert!(size_of::<t_scrst>() == 20 && align_of::<t_scrst>() == 4);
const _: () = assert!(size_of::<t_sblo>() == 68 && align_of::<t_sblo>() == 4);
const _: () = assert!(size_of::<t_sbctx>() == 40 && align_of::<t_sbctx>() == 8);
const _: () = assert!(size_of::<t_ctltext>() == 48 && align_of::<t_ctltext>() == 8);
const _: () = assert!(size_of::<t_textrun>() == 296 && align_of::<t_textrun>() == 8);
const _: () = assert!(size_of::<t_edit>() == 16 && align_of::<t_edit>() == 4);
const _: () = assert!(size_of::<t_edview>() == 280 && align_of::<t_edview>() == 8);
const _: () = assert!(size_of::<t_clip>() == 260 && align_of::<t_clip>() == 4);
const _: () = assert!(size_of::<t_list>() == 40 && align_of::<t_list>() == 8);
const _: () = assert!(size_of::<t_lstgeom>() == 44 && align_of::<t_lstgeom>() == 4);
const _: () = assert!(size_of::<t_menulevel>() == 32 && align_of::<t_menulevel>() == 8);
const _: () = assert!(size_of::<t_menu>() == 144 && align_of::<t_menu>() == 8);
const _: () = assert!(size_of::<t_pool>() == 16 && align_of::<t_pool>() == 8);
const _: () = assert!(size_of::<t_progress>() == 8 && align_of::<t_progress>() == 4);
const _: () = assert!(size_of::<t_rootpriv>() == 40 && align_of::<t_rootpriv>() == 8);
const _: () = assert!(size_of::<t_focusscan>() == 32 && align_of::<t_focusscan>() == 8);
const _: () = assert!(size_of::<t_ctlops>() == 64 && align_of::<t_ctlops>() == 8);
const _: () = assert!(size_of::<bool>() == 1 && align_of::<bool>() == 1);

const _: () = assert!(offset_of!(t_ctl, text) == 32 && offset_of!(t_ctl, parent) == 288);
const _: () = assert!(offset_of!(t_ctl, r#priv) == 328 && offset_of!(t_ctl, font) == 336);
const _: () = assert!(offset_of!(t_ctlroot, dirty) == 40);
const _: () = assert!(offset_of!(t_ctlroot, command) == 560 && offset_of!(t_ctlroot, user) == 568);
const _: () = assert!(offset_of!(t_ctlspec, rect) == 12 && offset_of!(t_ctlspec, text) == 32);
const _: () = assert!(offset_of!(t_ctlmenuspec, at) == 4 && offset_of!(t_ctlmenuspec, items) == 16);
const _: () = assert!(offset_of!(t_ctlmenuspec, count) == 24);
const _: () = assert!(offset_of!(t_mouseev, at) == 4 && offset_of!(t_mouseev, wheel) == 12);
const _: () = assert!(offset_of!(t_mouseev, dbl) == 16);
const _: () = assert!(offset_of!(t_sblo, vertical) == 64);
const _: () = assert!(offset_of!(t_ctltext, flags) == 24 && offset_of!(t_ctltext, color) == 28);
const _: () =
    assert!(offset_of!(t_ctltext, enabled) == 32 && offset_of!(t_ctltext, mnemonic) == 33);
const _: () = assert!(offset_of!(t_ctltext, font) == 40);
const _: () = assert!(offset_of!(t_ctlops, supported) == 8 && offset_of!(t_ctlops, hot) == 9);
const _: () = assert!(offset_of!(t_ctlops, init) == 16 && offset_of!(t_ctlops, destroy) == 24);
const _: () = assert!(offset_of!(t_ctlops, paint) == 32 && offset_of!(t_ctlops, mouse) == 40);
const _: () = assert!(offset_of!(t_ctlops, key) == 48 && offset_of!(t_ctlops, activate) == 56);
const _: () = assert!(offset_of!(t_list, sb) == 20);
const _: () = assert!(offset_of!(t_menu, lv) == 16);
const _: () = assert!(offset_of!(t_rootpriv, clock) == 0 && offset_of!(t_rootpriv, click_ns) == 8);
const _: () =
    assert!(offset_of!(t_rootpriv, click_at) == 16 && offset_of!(t_rootpriv, click_ctl) == 24);
const _: () = assert!(offset_of!(t_rootpriv, buttons) == 32);
const _: () = assert!(offset_of!(t_edview, col) == 276);
const _: () = assert!(offset_of!(t_textrun, ellipsis) == 280 && offset_of!(t_textrun, at) == 284);
const _: () = assert!(offset_of!(t_scrst, zone) == 16);
const _: () = assert!(offset_of!(t_edit, max_chars) == 12);
const _: () = {
    assert!(abi::E_OK == 0);
    assert!(abi::E_PERM == -1);
    assert!(abi::E_NOENT == -2);
    assert!(abi::E_IO == -5);
    assert!(abi::E_BADF == -9);
    assert!(abi::E_CHILD == -10);
    assert!(abi::E_AGAIN == -11);
    assert!(abi::E_NOMEM == -12);
    assert!(abi::E_ACCES == -13);
    assert!(abi::E_FAULT == -14);
    assert!(abi::E_BUSY == -16);
    assert!(abi::E_EXIST == -17);
    assert!(abi::E_NODEV == -19);
    assert!(abi::E_NOTDIR == -20);
    assert!(abi::E_ISDIR == -21);
    assert!(abi::E_INVAL == -22);
    assert!(abi::E_NFILE == -23);
    assert!(abi::E_MFILE == -24);
    assert!(abi::E_NOSPC == -28);
    assert!(abi::E_PIPE == -32);
    assert!(abi::E_RANGE == -34);
    assert!(abi::E_DEADLK == -35);
    assert!(abi::E_NOSYS == -38);
    assert!(abi::E_NOTEMPTY == -39);
    assert!(abi::E_PROTO == -71);
    assert!(abi::E_OVERFLOW == -75);
    assert!(abi::E_NOTSUP == -95);
    assert!(abi::E_TIMEOUT == -110);
    assert!(abi::E_CANCELED == -125);
    assert!(abi::CTL_MULTILINE == 0x20);
    assert!(abi::CTL_ID_OK == 1);
    assert!(abi::CT_IMAGE == 11);
    assert!(abi::INP_KEY_UP == 2);
    assert!(abi::FONT_INS_BOLD_24 == 16);
    assert!(abi::LS_DEFAULT == 4);
    assert!(abi::LS_FOCUSED == 5);
    assert!(abi::LUNA_THEME_LIGHT == 0);
};
