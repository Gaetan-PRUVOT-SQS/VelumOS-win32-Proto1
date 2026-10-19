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
    velum_keep_wm,
    velum_wm::wmc_activate,
    velum_wm::wmc_attach,
    velum_wm::wmc_capture,
    velum_wm::wmc_channel,
    velum_wm::wmc_connect,
    velum_wm::wmc_create,
    velum_wm::wmc_destroy,
    velum_wm::wmc_detach,
    velum_wm::wmc_disconnect,
    velum_wm::wmc_hdr,
    velum_wm::wmc_next,
    velum_wm::wmc_on_message,
    velum_wm::wmc_present,
    velum_wm::wmc_recv,
    velum_wm::wmc_send,
    velum_wm::wmc_set_cursor,
    velum_wm::wmc_set_icon,
    velum_wm::wmc_set_rect,
    velum_wm::wmc_set_state,
    velum_wm::wmc_set_state_id,
    velum_wm::wmc_set_theme,
    velum_wm::wmc_set_title,
    velum_wm::wmc_set_workarea,
    velum_wm::wmc_simple,
    velum_wm::wmc_slot,
    velum_wm::wmc_stash_pop,
    velum_wm::wmc_stash_push,
    velum_wm::wmc_subscribe,
    velum_wm::wmc_theme_apply,
    velum_wm::wmc_title_copy,
    velum_wm::wmc_valid,
    velum_wm::wmc_wait,
    velum_wm::wmsys_close,
    velum_wm::wmsys_connect,
    velum_wm::wmsys_map,
    velum_wm::wmsys_now,
    velum_wm::wmsys_recv,
    velum_wm::wmsys_send,
    velum_wm::wmsys_unmap,
);

/// # Safety
/// Ancre l'état du client. Aucun appelant ne doit l'utiliser.
#[unsafe(no_mangle)]
unsafe extern "C" fn velum_keep_wm_state() {
    let _ = core::ptr::addr_of!(velum_wm::g_wmc);
}
