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
    velum_keep_gfx,
    velum_gfx::gfx_argb,
    velum_gfx::gfx_blend,
    velum_gfx::gfx_lerp,
    velum_gfx::rect_make,
    velum_gfx::rect_intersect,
    velum_gfx::rect_union,
    velum_gfx::rect_empty,
    velum_gfx::rect_contains,
    velum_gfx::gfx_surface_init,
    velum_gfx::gfx_surface_sub,
    velum_gfx::gfx_set_clip,
    velum_gfx::gfx_put,
    velum_gfx::gfx_get,
    velum_gfx::gfx_fill,
    velum_gfx::gfx_hline,
    velum_gfx::gfx_vline,
    velum_gfx::gfx_line,
    velum_gfx::gfx_frame,
    velum_gfx::gfx_gradient,
    velum_gfx::gfx_rrect_fill,
    velum_gfx::gfx_blit,
    velum_gfx::gfx_blit_alpha,
    velum_gfx::gfx_blit_scaled,
    velum_gfx::gfx_scroll,
    velum_gfx::region_clear,
    velum_gfx::region_add,
    velum_gfx::region_subtract,
    velum_gfx::region_intersect,
    velum_gfx::region_bounds,
    velum_gfx::region_empty,
    velum_gfx::gfx_blit_smooth,
    velum_gfx::gfx_selftest,
    velum_gfx::gfx_px_at,
    velum_gfx::gfx_span_fill,
    velum_gfx::gfx_ramp_init,
    velum_gfx::gfx_ramp_seek,
    velum_gfx::gfx_ramp_next,
);
