macro_rules! touch {
    ($name:ident, $($path:path),* $(,)?) => {
        /// # Safety
        /// Ancre les symboles dans l'archive. Aucun appelant ne doit l'utiliser.
        #[unsafe(no_mangle)]
        unsafe extern "C" fn $name() {
            $(
                let _ = core::hint::black_box($path as *const ());
            )*
        }
    };
}

touch!(
    velum_keep_font,
    velum_font::font_get,
    velum_font::font_glyph,
    velum_font::font_utf8_next,
    velum_font::font_text_end,
    velum_font::font_text_width,
    velum_font::font_fit,
    velum_font::font_pen_start,
    velum_font::font_glyph_cut,
    velum_font::font_glyph_rows,
    velum_font::font_draw,
    velum_font::font_selftest,
);

/// # Safety
/// Ancre les polices dans l'archive. Aucun appelant ne doit l'utiliser.
#[unsafe(no_mangle)]
unsafe extern "C" fn velum_keep_font_data() {
    let _ = core::ptr::addr_of!(velum_font::g_font_ui);
    let _ = core::ptr::addr_of!(velum_font::g_font_ui_bold);
    let _ = core::ptr::addr_of!(velum_font::g_font_title);
    let _ = core::ptr::addr_of!(velum_font::g_font_mono);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_11);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_12);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_semi_12);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_13);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_semi_13);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_bold_13);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_14);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_semi_14);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_bold_14);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_15);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_22);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_semi_22);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_bold_24);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_bold_26);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_bold_28);
    let _ = core::ptr::addr_of!(velum_font::g_font_ins_semi_120);
    let _ = core::ptr::addr_of!(velum_font::g_font_jbm_14);
}
