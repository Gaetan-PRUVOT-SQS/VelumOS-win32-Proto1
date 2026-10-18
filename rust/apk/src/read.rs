use core::ffi::{c_char, c_int, c_void};

use crate::ffi::zip_extract;
use crate::ffi::zip_find;
use crate::mem::{heap_alloc, heap_free};
use crate::rd::uz;
use crate::{
    APK_ENTRY_MAX, APKR_ACTIVITE, APKR_ALGO, APKR_ARCHIVE, APKR_COUNT, APKR_LIBELLE,
    APKR_MANIFESTE, APKR_MANQUE, APKR_MEMOIRE, APKR_MULTIDEX, APKR_NATIF, APKR_NOM, APKR_OK,
    APKR_PAQUET, APKR_SIGNATAIRES, APKR_SIGNATURE, APKR_TAILLE, E_INVAL, E_NOENT, E_NOMEM,
    E_OVERFLOW, E_RANGE, t_apk, t_zipent,
};

const REASONS: [&[u8]; 15] = [
    b"Aucun refus.\0",
    b"Le fichier n'est pas une archive APK lisible.\0",
    b"L'application ou l'un de ses fichiers est trop volumineux.\0",
    b"L'archive contient un nom de fichier dangereux.\0",
    b"L'application contient du code natif, non pris en charge.\0",
    b"L'application contient plusieurs fichiers DEX, non pris en charge.\0",
    b"Il manque le manifeste ou le code de l'application.\0",
    b"La signature de l'application est absente ou invalide.\0",
    b"L'algorithme de signature n'est pas pris en charge.\0",
    b"L'application a plusieurs signataires, non pris en charge.\0",
    b"Le manifeste de l'application est illisible.\0",
    b"Le nom de paquet de l'application est invalide.\0",
    "L'application n'a pas d'écran principal à lancer.\0".as_bytes(),
    b"Le nom de l'application est introuvable ou invalide.\0",
    "La mémoire manque pour lire l'application.\0".as_bytes(),
];

const UNKNOWN: &[u8] = b"Refus inconnu.\0";

const _: () = assert!(APKR_OK == 0);
const _: () = assert!(APKR_ARCHIVE == 1);
const _: () = assert!(APKR_TAILLE == 2);
const _: () = assert!(APKR_NOM == 3);
const _: () = assert!(APKR_NATIF == 4);
const _: () = assert!(APKR_MULTIDEX == 5);
const _: () = assert!(APKR_MANQUE == 6);
const _: () = assert!(APKR_SIGNATURE == 7);
const _: () = assert!(APKR_ALGO == 8);
const _: () = assert!(APKR_SIGNATAIRES == 9);
const _: () = assert!(APKR_MANIFESTE == 10);
const _: () = assert!(APKR_PAQUET == 11);
const _: () = assert!(APKR_ACTIVITE == 12);
const _: () = assert!(APKR_LIBELLE == 13);
const _: () = assert!(APKR_MEMOIRE == 14);
const _: () = assert!(REASONS.len() == APKR_COUNT as usize);

fn err_i64(e: c_int) -> i64 {
    i64::from(e)
}

/// # Safety
/// `reason` est un entier quelconque. Aucun pointeur n'est lu. Le retour, s'il est non nul,
/// vise une chaîne C statique.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn apk_reason(reason: c_int) -> *const c_char {
    if !(0..APKR_COUNT).contains(&reason) {
        return UNKNOWN.as_ptr().cast();
    }
    let Some(i) = usize::try_from(reason).ok() else {
        return UNKNOWN.as_ptr().cast();
    };
    let Some(s) = REASONS.get(i) else {
        return UNKNOWN.as_ptr().cast();
    };
    s.as_ptr().cast()
}

/// # Safety
/// `a` est nul, ou bien aligné, et `a.file` ainsi que l'archive restent lisibles.
/// `name` est nul, ou bien une chaîne C.
/// `out` est nul, ou bien aligné et exclusif. En cas de succès, `*out` vient de l'allocateur
/// et le tampon fait `retour + 1` octets. L'appelant le libère avec le même allocateur.
/// Un échec laisse `*out` à nul quand `out` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn apk_read(a: *const t_apk, name: *const c_char, out: *mut *mut u8) -> i64 {
    if out.is_null() || !out.is_aligned() {
        return err_i64(E_INVAL);
    }
    // SAFETY: out est non nul et aligné. On n'écrit qu'un pointeur.
    unsafe { *out = core::ptr::null_mut() };
    if a.is_null() || !a.is_aligned() || name.is_null() {
        return err_i64(E_INVAL);
    }
    // SAFETY: a est non nul et aligné, lecture seule le temps de l'appel.
    let a = unsafe { &*a };
    if a.reason != APKR_OK || a.file.p.is_null() {
        return err_i64(E_INVAL);
    }
    let mut e = t_zipent::zero();
    // SAFETY: a.zip reste lisible. name est une chaîne C. e est exclusif.
    let found = unsafe {
        zip_find(
            core::ptr::from_ref(&a.zip),
            name,
            core::ptr::from_mut(&mut e),
        )
    };
    if found < 0 {
        return err_i64(E_NOENT);
    }
    if e.r#usize > APK_ENTRY_MAX {
        return err_i64(E_RANGE);
    }
    let Some(need) = uz(e.r#usize).checked_add(1) else {
        return err_i64(E_OVERFLOW);
    };
    let buf = heap_alloc(need).cast::<u8>();
    if buf.is_null() {
        return err_i64(E_NOMEM);
    }
    // SAFETY: buf couvre e.usize octets inscriptibles, distincts de l'archive. e reste valide.
    let n = unsafe {
        zip_extract(
            core::ptr::from_ref(&a.zip),
            core::ptr::from_ref(&e),
            buf,
            uz(e.r#usize),
        )
    };
    if n == i64::from(e.r#usize) {
        // SAFETY: out est toujours le pointeur exclusif du début de l'appel.
        unsafe { *out = buf };
    } else {
        heap_free(buf.cast::<c_void>());
    }
    if n >= 0 && n != i64::from(e.r#usize) {
        return err_i64(E_INVAL);
    }
    n
}
