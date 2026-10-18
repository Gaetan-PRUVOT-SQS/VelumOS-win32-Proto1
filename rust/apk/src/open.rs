use core::ffi::c_int;

use crate::ffi::{zip_entry, zip_find, zip_name_safe, zip_open};
use crate::rd::uz;
use crate::{
    APK_ENTRY_MAX, APK_FILE_MAX, APKR_ARCHIVE, APKR_MANQUE, APKR_MULTIDEX, APKR_NATIF, APKR_NOM,
    APKR_OK, APKR_TAILLE, E_INVAL, E_NOENT, E_NOTSUP, E_OK, E_RANGE, t_apk, t_span, t_zipent,
};

fn refuse(a: &mut t_apk, reason: c_int, err: c_int) -> c_int {
    a.reason = reason;
    a.file = t_span::zero();
    err
}

fn entry_name(e: &t_zipent) -> Option<&[u8]> {
    if e.name.is_null() {
        return None;
    }
    let n = uz(e.name_len);
    if n > isize::MAX as usize {
        return None;
    }
    // SAFETY: zip_entry garantit name_len octets lisibles, ou le nom a déjà été refusé.
    Some(unsafe { core::slice::from_raw_parts(e.name.cast(), n) })
}

fn is_native(e: &t_zipent) -> bool {
    let Some(name) = entry_name(e) else {
        return false;
    };
    name.len() >= 7 && name.starts_with(b"lib/") && name.ends_with(b".so")
}

fn is_extra_dex(e: &t_zipent) -> bool {
    let Some(name) = entry_name(e) else {
        return false;
    };
    if name.len() < 12 || !name.starts_with(b"classes") || !name.ends_with(b".dex") {
        return false;
    }
    let mid_end = name.len() - 4;
    let mut i = 7usize;
    while i < mid_end {
        if !name[i].is_ascii_digit() {
            return false;
        }
        i += 1;
    }
    true
}

fn scan(a: &mut t_apk) -> c_int {
    let mut i = 0u32;
    while i < a.zip.count {
        let mut e = t_zipent::zero();
        // SAFETY: a.zip est initialisé par zip_open. e est exclusif le temps de l'appel.
        let r = unsafe { zip_entry(core::ptr::from_ref(&a.zip), i, core::ptr::from_mut(&mut e)) };
        if r < 0 {
            return refuse(a, APKR_ARCHIVE, E_INVAL);
        }
        // SAFETY: e.name est nul ou couvre e.name_len octets, contrat de zip_entry.
        let safe = unsafe { zip_name_safe(e.name, uz(e.name_len)) };
        if safe != 1 {
            return refuse(a, APKR_NOM, E_INVAL);
        }
        if is_native(&e) {
            return refuse(a, APKR_NATIF, E_NOTSUP);
        }
        if is_extra_dex(&e) {
            return refuse(a, APKR_MULTIDEX, E_NOTSUP);
        }
        i += 1;
    }
    E_OK
}

/// # Safety
/// `a` est nul, ou bien aligné et exclusif le temps de l'appel.
/// `file.p` est nul, ou bien il pointe `file.len` octets initialisés qui restent valides
/// tant que `a` sert. Au-dessus de `APK_FILE_MAX`, rien n'est lu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn apk_open(a: *mut t_apk, file: t_span) -> c_int {
    if a.is_null() || !a.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: a est non nul et aligné, accès exclusif le temps de l'appel.
    let a = unsafe { &mut *a };
    *a = t_apk::zero();
    if file.p.is_null() {
        return refuse(a, APKR_ARCHIVE, E_INVAL);
    }
    if file.len > APK_FILE_MAX {
        return refuse(a, APKR_TAILLE, E_RANGE);
    }
    // SAFETY: a.zip est exclusif. file.p couvre file.len octets pour l'appel.
    let r = unsafe { zip_open(core::ptr::from_mut(&mut a.zip), file) };
    if r < 0 {
        return refuse(a, APKR_ARCHIVE, E_INVAL);
    }
    a.file = file;
    let r = scan(a);
    if r < 0 {
        return r;
    }
    let manifest = c"AndroidManifest.xml";
    let dex = c"classes.dex";
    // SAFETY: a.zip reste lisible. Les noms sont des chaînes C statiques. Les sorties sont exclusives.
    let r_man = unsafe {
        zip_find(
            core::ptr::from_ref(&a.zip),
            manifest.as_ptr(),
            core::ptr::from_mut(&mut a.manifest),
        )
    };
    if r_man < 0 {
        return refuse(a, APKR_MANQUE, E_NOENT);
    }
    // SAFETY: même contrat que la recherche du manifeste.
    let r_dex = unsafe {
        zip_find(
            core::ptr::from_ref(&a.zip),
            dex.as_ptr(),
            core::ptr::from_mut(&mut a.dex),
        )
    };
    if r_dex < 0 {
        return refuse(a, APKR_MANQUE, E_NOENT);
    }
    if a.manifest.r#usize > APK_ENTRY_MAX || a.dex.r#usize > APK_ENTRY_MAX {
        return refuse(a, APKR_TAILLE, E_RANGE);
    }
    a.reason = APKR_OK;
    E_OK
}
