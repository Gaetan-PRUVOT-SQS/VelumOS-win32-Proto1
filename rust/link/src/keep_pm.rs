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
    velum_keep_pm,
    velum_pm::pm_alloc,
    velum_pm::pm_free,
    velum_pm::pm_drop,
    velum_pm::pm_find,
    velum_pm::pm_install,
    velum_pm::pm_list,
    velum_pm::pm_remove,
    velum_pm::pm_store,
    velum_pm::pm_work_open,
    velum_pm::pm_act_ok,
    velum_pm::pm_dec_u32,
    velum_pm::pm_hex_cert,
    velum_pm::pm_info_ok,
    velum_pm::pm_label_ok,
    velum_pm::pm_nlen,
    velum_pm::pm_pkg_ok,
    velum_pm::pm_same_pkg,
    velum_pm::pm_utf8_ok,
    velum_pm::pm_defaults,
    velum_pm::pm_index,
    velum_pm::pm_path,
    velum_pm::pm_registry_format,
    velum_pm::pm_registry_parse,
    velum_pm::pm_registry_scan,
);
