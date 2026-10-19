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
    velum_keep_crypto,
    velum_crypto::sha256_init,
    velum_crypto::sha256_update,
    velum_crypto::sha256_final,
    velum_crypto::sha256,
    velum_crypto::hmac_prepare,
    velum_crypto::hmac_update,
    velum_crypto::hmac_final,
    velum_crypto::hmac_sha256,
    velum_crypto::pbkdf2_validate,
    velum_crypto::pbkdf2_sha256,
    velum_crypto::chacha20_init,
    velum_crypto::chacha20_block,
    velum_crypto::ct_equal,
    velum_crypto::secure_zero,
);
