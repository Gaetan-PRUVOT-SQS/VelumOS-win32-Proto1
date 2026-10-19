macro_rules! c_fn {
    ($name:ident ( $($an:ident : $ty:ty),* $(,)? ) -> $ret:ty $body:block) => {
        /// # Safety
        /// Pointeurs selon le C du même nom : nuls seulement si le corps le teste, sinon alignés et vivants pendant l'appel.
        #[unsafe(no_mangle)]
        pub unsafe extern "C" fn $name($($an: $ty),*) -> $ret $body
    };
    ($name:ident ( $($an:ident : $ty:ty),* $(,)? ) $body:block) => {
        /// # Safety
        /// Pointeurs selon le C du même nom : nuls seulement si le corps le teste, sinon alignés et vivants pendant l'appel.
        #[unsafe(no_mangle)]
        pub unsafe extern "C" fn $name($($an: $ty),*) $body
    };
}
