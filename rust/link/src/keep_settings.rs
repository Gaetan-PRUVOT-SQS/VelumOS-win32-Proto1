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
    velum_keep_settings,
    velum_settings::set_default,
    velum_settings::set_parse,
    velum_settings::set_format,
    velum_settings::set_dark_at,
    velum_settings::set_mode_get,
    velum_settings::set_load,
    velum_settings::set_save,
);
