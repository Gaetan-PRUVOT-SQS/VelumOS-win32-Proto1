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
    velum_keep_axml,
    velum_axml::axml_open,
    velum_axml::axml_next,
    velum_axml::axml_name,
    velum_axml::axml_attr_count,
    velum_axml::axml_attr,
    velum_axml::axml_attr_find,
    velum_axml::axml_string,
    velum_axml::axml_text,
    velum_axml::respool_open,
    velum_axml::respool_get,
    velum_axml::arsc_open,
    velum_axml::arsc_lookup,
    velum_axml::arsc_string,
);
