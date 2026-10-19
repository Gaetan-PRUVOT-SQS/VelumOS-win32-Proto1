#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

use core::ffi::{c_char, c_int};

mod arsc;
mod pool;
mod rd;
mod utf;
mod xml;

pub(crate) const E_OK: c_int = 0;
pub(crate) const E_NOENT: c_int = -2;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_RANGE: c_int = -34;
pub(crate) const E_OVERFLOW: c_int = -75;
pub(crate) const E_NOTSUP: c_int = -95;

pub(crate) const AXML_DONE: c_int = 0;
pub(crate) const AXML_START: c_int = 1;
pub(crate) const AXML_END: c_int = 2;
pub(crate) const AXML_TEXT: c_int = 3;
pub(crate) const AXML_SKIP: c_int = 4;
pub(crate) const AXML_MAX_DEPTH: u32 = 64;
pub(crate) const AXML_NO_STRING: u32 = 0xffff_ffff;

pub(crate) const RES_T_REFERENCE: u32 = 0x01;
pub(crate) const RES_T_STRING: u32 = 0x03;

pub(crate) const RES_STRING_POOL: u32 = 0x0001;
pub(crate) const RES_TABLE: u32 = 0x0002;
pub(crate) const RES_XML: u32 = 0x0003;
pub(crate) const RES_XML_START_NS: u32 = 0x0100;
pub(crate) const RES_XML_END_NS: u32 = 0x0101;
pub(crate) const RES_XML_START: u32 = 0x0102;
pub(crate) const RES_XML_END: u32 = 0x0103;
pub(crate) const RES_XML_CDATA: u32 = 0x0104;
pub(crate) const RES_XML_RESMAP: u32 = 0x0180;
pub(crate) const RES_TABLE_PACKAGE: u32 = 0x0200;
pub(crate) const RES_TABLE_TYPE: u32 = 0x0201;
pub(crate) const RES_POOL_UTF8: u32 = 0x100;
pub(crate) const RES_POOL_HEADER: u32 = 28;
pub(crate) const RES_SIZE_MAX: u32 = 0x7fff_ffff;
pub(crate) const RES_NO_ENTRY: u32 = 0xffff_ffff;
pub(crate) const RES_ENTRY_COMPLEX: u32 = 0x0001;
pub(crate) const RES_ENTRY_COMPACT: u32 = 0x0008;
pub(crate) const ARSC_MAX_REF: i32 = 8;

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_span {
    pub p: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_text {
    pub p: *mut c_char,
    pub cap: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_reschunk {
    pub off: u32,
    pub r#type: u32,
    pub hsize: u32,
    pub size: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_respool {
    pub p: *const u8,
    pub size: u32,
    pub hsize: u32,
    pub count: u32,
    pub strings: u32,
    pub utf8: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_axmlattr {
    pub name: u32,
    pub res_id: u32,
    pub raw: u32,
    pub r#type: u32,
    pub data: u32,
    pub ns: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_axml {
    pub file: t_span,
    pub pool: t_respool,
    pub map_off: u32,
    pub map_count: u32,
    pub pos: u32,
    pub depth: u32,
    pub ev: u32,
    pub ev_off: u32,
    pub attr_off: u32,
    pub attr_size: u32,
    pub attr_count: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_resvalue {
    pub r#type: u32,
    pub data: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_resquery {
    pub res_id: u32,
    pub lang: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(non_camel_case_types)]
pub struct t_arsc {
    pub file: t_span,
    pub strings: t_respool,
    pub body: u32,
    pub has_pool: u32,
    pub packages: u32,
}

const _: () = assert!(core::mem::size_of::<t_span>() == 16);
const _: () = assert!(core::mem::align_of::<t_span>() == 8);
const _: () = assert!(core::mem::size_of::<t_text>() == 16);
const _: () = assert!(core::mem::align_of::<t_text>() == 8);
const _: () = assert!(core::mem::size_of::<t_reschunk>() == 16);
const _: () = assert!(core::mem::align_of::<t_reschunk>() == 4);
const _: () = assert!(core::mem::size_of::<t_respool>() == 32);
const _: () = assert!(core::mem::align_of::<t_respool>() == 8);
const _: () = assert!(core::mem::size_of::<t_axmlattr>() == 24);
const _: () = assert!(core::mem::align_of::<t_axmlattr>() == 4);
const _: () = assert!(core::mem::size_of::<t_axml>() == 88);
const _: () = assert!(core::mem::align_of::<t_axml>() == 8);
const _: () = assert!(core::mem::size_of::<t_resvalue>() == 8);
const _: () = assert!(core::mem::align_of::<t_resvalue>() == 4);
const _: () = assert!(core::mem::size_of::<t_resquery>() == 16);
const _: () = assert!(core::mem::align_of::<t_resquery>() == 8);
const _: () = assert!(core::mem::size_of::<t_arsc>() == 64);
const _: () = assert!(core::mem::align_of::<t_arsc>() == 8);

fn aligned<T>(p: *const T) -> bool {
    !p.is_null() && p.is_aligned()
}

fn aligned_mut<T>(p: *mut T) -> bool {
    !p.is_null() && p.is_aligned()
}

/// # Safety
/// `p` est nul, ou bien aligné et exclusif le temps de l'appel.
/// `chunk.p` est nul, ou bien il pointe `chunk.len` octets initialisés.
/// Au-delà de `RES_SIZE_MAX`, ou si le morceau n'est pas une table de chaînes, `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn respool_open(p: *mut t_respool, chunk: t_span) -> c_int {
    if !aligned_mut(p) {
        return E_INVAL;
    }
    // SAFETY: p non nul et aligné, accès exclusif le temps de l'appel.
    let p = unsafe { &mut *p };
    pool::open(p, chunk)
}

/// # Safety
/// `p` est nul, ou bien aligné et sa plage `p.p`/`p.size` reste lisible.
/// `out.p` est nul, ou bien il pointe `out.cap` octets inscriptibles qui ne chevauchent pas la table.
/// Un indice hors table donne `E_RANGE`. Un tampon trop petit donne `E_OVERFLOW` sans octet utile garanti.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn respool_get(p: *const t_respool, index: u32, out: t_text) -> c_int {
    if !aligned(p) {
        return E_INVAL;
    }
    // SAFETY: p non nul et aligné. La copie tient les champs, le tampon de sortie n'est pas emprunté ici.
    let view = unsafe { *p };
    pool::get_copied(view, index, out)
}

/// # Safety
/// `x` est nul, ou bien aligné et exclusif le temps de l'appel.
/// `file.p` est nul, ou bien il pointe `file.len` octets initialisés.
/// Un document refusé laisse `file.p` à nul. La table peut être partiellement écrite si son en-tête tenait.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_open(x: *mut t_axml, file: t_span) -> c_int {
    if !aligned_mut(x) {
        return E_INVAL;
    }
    // SAFETY: x non nul et aligné, accès exclusif le temps de l'appel.
    let x = unsafe { &mut *x };
    xml::open(x, file)
}

/// # Safety
/// `x` est nul, ou bien aligné, exclusif, et `x.file` reste lisible.
/// Une erreur reste sur le même morceau au rappel suivant.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_next(x: *mut t_axml) -> c_int {
    if !aligned_mut(x) {
        return E_INVAL;
    }
    // SAFETY: x non nul et aligné, accès exclusif le temps de l'appel.
    let x = unsafe { &mut *x };
    xml::next_ev(x)
}

/// # Safety
/// `x` est nul, ou bien aligné et `x.file` reste lisible.
/// `out.p` est nul, ou bien il pointe `out.cap` octets inscriptibles hors de `x.file`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_name(x: *const t_axml, out: t_text) -> c_int {
    if !aligned(x) {
        return E_INVAL;
    }
    // SAFETY: x non nul et aligné, lecture seule le temps de l'appel.
    let x = unsafe { &*x };
    xml::name(x, out)
}

/// # Safety
/// `x` est nul, ou bien aligné et lisible. Hors élément ouvert, le résultat est 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_attr_count(x: *const t_axml) -> u32 {
    if !aligned(x) {
        return 0;
    }
    // SAFETY: x non nul et aligné, lecture seule le temps de l'appel.
    let x = unsafe { &*x };
    xml::attr_count(x)
}

/// # Safety
/// `x` est nul, ou bien aligné et `x.file` reste lisible.
/// `out` est nul, ou bien aligné et exclusif. L'indice hors liste donne `E_RANGE`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_attr(x: *const t_axml, i: u32, out: *mut t_axmlattr) -> c_int {
    if !aligned(x) || !aligned_mut(out) {
        return E_INVAL;
    }
    // SAFETY: x non nul et aligné, lecture seule le temps de l'appel.
    let x = unsafe { &*x };
    // SAFETY: out non nul et aligné, exclusif le temps de l'appel.
    let out = unsafe { &mut *out };
    xml::attr(x, i, out)
}

/// # Safety
/// `x` est nul, ou bien aligné et `x.file` reste lisible.
/// `out` est nul, ou bien aligné et exclusif. `res_id` nul donne `E_INVAL` sans lire `x` ni `out`.
/// Absent de la liste : `E_NOENT`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_attr_find(
    x: *const t_axml,
    res_id: u32,
    out: *mut t_axmlattr,
) -> c_int {
    if res_id == 0 {
        return E_INVAL;
    }
    if !aligned(x) || !aligned_mut(out) {
        return E_INVAL;
    }
    // SAFETY: x non nul et aligné, lecture seule le temps de l'appel.
    let x = unsafe { &*x };
    // SAFETY: out non nul et aligné, exclusif le temps de l'appel.
    let out = unsafe { &mut *out };
    xml::attr_find(x, res_id, out)
}

/// # Safety
/// `x` est nul, ou bien aligné et sa table reste lisible.
/// `out` comme pour `respool_get`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_string(x: *const t_axml, index: u32, out: t_text) -> c_int {
    if !aligned(x) {
        return E_INVAL;
    }
    // SAFETY: x non nul et aligné, lecture seule le temps de l'appel.
    let x = unsafe { &*x };
    xml::string(x, index, out)
}

/// # Safety
/// `x` est nul, ou bien aligné et `x.file` reste lisible.
/// `out` comme pour `respool_get`. Hors nœud texte, `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn axml_text(x: *const t_axml, out: t_text) -> c_int {
    if !aligned(x) {
        return E_INVAL;
    }
    // SAFETY: x non nul et aligné, lecture seule le temps de l'appel.
    let x = unsafe { &*x };
    xml::text(x, out)
}

/// # Safety
/// `a` est nul, ou bien aligné et exclusif le temps de l'appel.
/// `file.p` est nul, ou bien il pointe `file.len` octets initialisés.
/// Un refus après l'en-tête met `file.p` à nul. Un en-tête illisible ne touche pas `a`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn arsc_open(a: *mut t_arsc, file: t_span) -> c_int {
    if !aligned_mut(a) {
        return E_INVAL;
    }
    // SAFETY: a non nul et aligné, accès exclusif le temps de l'appel.
    let a = unsafe { &mut *a };
    arsc::open(a, file)
}

/// # Safety
/// `a` est nul, ou bien aligné et `a.file` reste lisible.
/// `lang` est nul, ou bien une chaîne C dont les deux premiers octets sont lisibles si le premier n'est pas nul.
/// `out` est nul, ou bien aligné et exclusif.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn arsc_lookup(
    a: *const t_arsc,
    res_id: u32,
    lang: *const c_char,
    out: *mut t_resvalue,
) -> c_int {
    if !aligned(a) || !aligned_mut(out) {
        return E_INVAL;
    }
    // SAFETY: a non nul et aligné, lecture seule le temps de l'appel.
    let a = unsafe { &*a };
    match arsc::lookup(a, res_id, lang) {
        Ok(v) => {
            // SAFETY: out non nul et aligné, exclusif le temps de l'appel.
            unsafe { *out = v };
            E_OK
        }
        Err(e) => e,
    }
}

/// # Safety
/// `a` est nul, ou bien aligné et son fichier ainsi que sa table globale restent lisibles.
/// `q` est nul, ou bien aligné. `q.lang` suit le contrat de `arsc_lookup`.
/// `out` comme pour `respool_get`. Une chaîne de références trop longue donne `E_RANGE`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn arsc_string(a: *const t_arsc, q: *const t_resquery, out: t_text) -> c_int {
    if !aligned(a) || !aligned(q) {
        return E_INVAL;
    }
    // SAFETY: a non nul et aligné, lecture seule le temps de l'appel.
    let a = unsafe { &*a };
    // SAFETY: q non nul et aligné, lecture seule le temps de l'appel.
    let q = unsafe { &*q };
    arsc::string(a, q, out)
}

#[cfg(test)]
mod fix;
#[cfg(test)]
mod tests;
