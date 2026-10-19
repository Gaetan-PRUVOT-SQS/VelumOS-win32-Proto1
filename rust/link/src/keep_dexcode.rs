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
    velum_keep_dexcode,
    velum_dexcode::dexcode_decode,
    velum_dexcode::dexcode_name,
    velum_dexcode::dexcode_format,
    velum_dexcode::dexcode_index_kind,
    velum_dexcode::dexcode_verify,
    velum_dexcode::dexcode_switch,
    velum_dexcode::dexcode_array_data,
    velum_dexcode::dexcode_flags,
    velum_dexcode::dexcode_format_len,
    velum_dexcode::dexcode_rd32,
);
