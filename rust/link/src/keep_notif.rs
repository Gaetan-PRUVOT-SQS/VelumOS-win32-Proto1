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

#[cfg(velum_user)]
touch!(
    velum_keep_notif_client,
    velum_notif::notif_post,
    velum_notif::notif_client_count,
    velum_notif::notif_client_toggle,
    velum_notif::notif_client_dismiss,
    velum_notif::notif_client_clear,
    velum_notif::notif_client_close,
);

touch!(
    velum_keep_notif,
    velum_notif::notif_store_init,
    velum_notif::notif_store_post,
    velum_notif::notif_store_dismiss,
    velum_notif::notif_store_clear,
    velum_notif::notif_store_count,
    velum_notif::notif_store_get,
    velum_notif::notif_fields_valid,
    velum_notif::notif_msg_decode,
    velum_notif::notif_msg_init,
    velum_notif::notif_msg_post,
    velum_notif::notif_badge_text,
    velum_notif::notif_age_text,
);
