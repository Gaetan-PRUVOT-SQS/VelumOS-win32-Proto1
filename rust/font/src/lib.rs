#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod data;
pub use data::{
    g_font_ins_11, g_font_ins_12, g_font_ins_13, g_font_ins_14, g_font_ins_15, g_font_ins_22,
    g_font_ins_bold_13, g_font_ins_bold_14, g_font_ins_bold_24, g_font_ins_bold_26,
    g_font_ins_bold_28, g_font_ins_semi_12, g_font_ins_semi_13, g_font_ins_semi_14,
    g_font_ins_semi_22, g_font_ins_semi_120, g_font_jbm_14, g_font_mono, g_font_title, g_font_ui,
    g_font_ui_bold,
};
mod draw;
mod glyph;
mod pen;
mod selftest;
mod table;
mod text;
mod utf8;

#[cfg(test)]
mod tests;

use core::ffi::{c_char, c_int};

pub const FONT_UI: c_int = 0;
pub const FONT_UI_BOLD: c_int = 1;
pub const FONT_TITLE: c_int = 2;
pub const FONT_MONO: c_int = 3;
pub const FONT_INS_11: c_int = 4;
pub const FONT_INS_12: c_int = 5;
pub const FONT_INS_SEMI_12: c_int = 6;
pub const FONT_INS_13: c_int = 7;
pub const FONT_INS_SEMI_13: c_int = 8;
pub const FONT_INS_BOLD_13: c_int = 9;
pub const FONT_INS_14: c_int = 10;
pub const FONT_INS_SEMI_14: c_int = 11;
pub const FONT_INS_BOLD_14: c_int = 12;
pub const FONT_INS_15: c_int = 13;
pub const FONT_INS_22: c_int = 14;
pub const FONT_INS_SEMI_22: c_int = 15;
pub const FONT_INS_BOLD_24: c_int = 16;
pub const FONT_INS_BOLD_26: c_int = 17;
pub const FONT_INS_BOLD_28: c_int = 18;
pub const FONT_INS_SEMI_120: c_int = 19;
pub const FONT_JBM_14: c_int = 20;
pub const FONT_IDS: c_int = 21;

pub type Color = u32;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Point {
    pub x: i32,
    pub y: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Rect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Surface {
    pub px: *mut u32,
    pub w: i32,
    pub h: i32,
    pub stride: i32,
    pub clip: Rect,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Glyph {
    pub cp: u32,
    pub w: u8,
    pub h: u8,
    pub xoff: i8,
    pub yoff: i8,
    pub advance: u8,
    pub off: u16,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Font {
    pub ascent: i32,
    pub descent: i32,
    pub height: i32,
    pub nglyphs: i32,
    pub glyphs: *const Glyph,
    pub bits: *const u8,
    pub nbits: u32,
}

// SAFETY: glyphs et bits visent des tables immuables, jamais libérées.
unsafe impl Sync for Font {}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct TextReq {
    pub font: *const Font,
    pub at: Point,
    pub color: Color,
    pub text: *const c_char,
    pub len: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct PenBox {
    pub left: i32,
    pub top: i32,
    pub right: i32,
    pub bottom: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Pen {
    pub dst: *mut Surface,
    pub font: *const Font,
    pub color: Color,
    pub x: i64,
    pub y: i64,
    pub bounds: PenBox,
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Cut {
    pub rows: *const u8,
    pub x: i32,
    pub y: i32,
    pub c0: i32,
    pub c1: i32,
    pub r0: i32,
    pub r1: i32,
}

macro_rules! export {
    ($(fn $name:ident($($arg:ident: $ty:ty),*) -> $ret:ty = $path:path);* $(;)?) => {
        $(
            #[unsafe(no_mangle)]
            /// # Safety
            /// Même contrat que la fonction C du même nom. Un pointeur nul n'est pas déréférencé.
            pub unsafe extern "C" fn $name($($arg: $ty),*) -> $ret {
                $path($($arg),*)
            }
        )*
    };
}

export! {
    fn font_get(id: c_int) -> *const Font = table::get;
    fn font_glyph(f: *const Font, cp: u32) -> *const Glyph = glyph::glyph;
    fn font_utf8_next(s: *mut *const c_char, end: *const c_char) -> u32 = utf8::next;
    fn font_text_end(text: *const c_char, len: i32) -> *const c_char = text::end;
    fn font_text_width(f: *const Font, text: *const c_char, len: i32) -> i32 = text::width;
    fn font_fit(f: *const Font, text: *const c_char, max_w: i32) -> i32 = text::fit;
    fn font_pen_start(pen: *mut Pen, dst: *mut Surface, rq: *const TextReq) -> c_int = pen::start;
    fn font_glyph_cut(pen: *const Pen, g: *const Glyph, cut: *mut Cut) -> c_int = pen::cut;
    fn font_glyph_rows(f: *const Font, g: *const Glyph, rows: *mut *const u8) -> c_int = draw::rows;
    fn font_draw(dst: *mut Surface, rq: *const TextReq) -> () = draw::draw;
    fn font_selftest() -> c_int = selftest::run;
}
