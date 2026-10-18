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
    velum_keep_dex,
    velum_dex::dex_open,
    velum_dex::dex_string,
    velum_dex::dex_type,
    velum_dex::dex_proto,
    velum_dex::dex_field,
    velum_dex::dex_method,
    velum_dex::dex_class,
    velum_dex::dex_find_class,
    velum_dex::dex_cdata_open,
    velum_dex::dex_cdata_next,
    velum_dex::dex_code,
    velum_dex::dex_try_find,
    velum_dex::dex_handler_open,
    velum_dex::dex_handler_next,
    velum_dex::dex_static_value,
    velum_dex::dex_type_list,
    velum_dex::mutf8_to_utf8,
    velum_dex::dex_adler32,
    velum_dex::dex_list_at,
    velum_dex::dex_u16,
    velum_dex::dex_u32,
    velum_dex::dex_uleb,
    velum_dex::dex_sleb,
    velum_dex::dex_ulebp1,
    velum_dex::dex_mutf8_check,
);
