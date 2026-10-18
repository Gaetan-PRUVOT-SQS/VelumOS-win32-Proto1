use crate::{
    FONT_IDS, FONT_INS_11, FONT_INS_12, FONT_INS_13, FONT_INS_14, FONT_INS_15, FONT_INS_22,
    FONT_INS_BOLD_13, FONT_INS_BOLD_14, FONT_INS_BOLD_24, FONT_INS_BOLD_26, FONT_INS_BOLD_28,
    FONT_INS_SEMI_12, FONT_INS_SEMI_13, FONT_INS_SEMI_14, FONT_INS_SEMI_22, FONT_INS_SEMI_120,
    FONT_JBM_14, FONT_MONO, FONT_TITLE, FONT_UI, FONT_UI_BOLD, Font, data,
};
use core::ffi::c_int;

pub(crate) fn by_id(id: c_int) -> *const Font {
    match id {
        FONT_UI => &raw const data::g_font_ui,
        FONT_UI_BOLD => &raw const data::g_font_ui_bold,
        FONT_TITLE => &raw const data::g_font_title,
        FONT_MONO => &raw const data::g_font_mono,
        FONT_INS_11 => &raw const data::g_font_ins_11,
        FONT_INS_12 => &raw const data::g_font_ins_12,
        FONT_INS_SEMI_12 => &raw const data::g_font_ins_semi_12,
        FONT_INS_13 => &raw const data::g_font_ins_13,
        FONT_INS_SEMI_13 => &raw const data::g_font_ins_semi_13,
        FONT_INS_BOLD_13 => &raw const data::g_font_ins_bold_13,
        FONT_INS_14 => &raw const data::g_font_ins_14,
        FONT_INS_SEMI_14 => &raw const data::g_font_ins_semi_14,
        FONT_INS_BOLD_14 => &raw const data::g_font_ins_bold_14,
        FONT_INS_15 => &raw const data::g_font_ins_15,
        FONT_INS_22 => &raw const data::g_font_ins_22,
        FONT_INS_SEMI_22 => &raw const data::g_font_ins_semi_22,
        FONT_INS_BOLD_24 => &raw const data::g_font_ins_bold_24,
        FONT_INS_BOLD_26 => &raw const data::g_font_ins_bold_26,
        FONT_INS_BOLD_28 => &raw const data::g_font_ins_bold_28,
        FONT_INS_SEMI_120 => &raw const data::g_font_ins_semi_120,
        FONT_JBM_14 => &raw const data::g_font_jbm_14,
        _ => core::ptr::null(),
    }
}

pub(crate) fn get(id: c_int) -> *const Font {
    if !(0..FONT_IDS).contains(&id) {
        return core::ptr::null();
    }
    by_id(id)
}
