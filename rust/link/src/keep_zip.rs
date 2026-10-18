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
    velum_keep_zip,
    velum_zip::zip_open,
    velum_zip::zip_entry,
    velum_zip::zip_find,
    velum_zip::zip_extract,
    velum_zip::zip_name_safe,
    velum_zip::inflate_raw,
    velum_zip::zip_crc32,
);
