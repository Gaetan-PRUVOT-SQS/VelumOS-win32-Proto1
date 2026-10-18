use crate::adler::adler32;
use crate::cdata::{next, open};
use crate::classes::{class_at, find};
use crate::code::{code_item, handler_next, handler_open, try_find};
use crate::ids::{field, list_at, method, proto, string, type_id, type_list};
use crate::mutf8::{check, to_utf8};
use crate::open::open_at;
use crate::rd::{sleb, u16_ptr, u32_ptr, uleb, ulebp1};
use crate::value::static_value;
use crate::{
    DEX_NO_INDEX, E_INVAL, ref_in, ref_out, t_dex, t_dexcatch, t_dexcdata, t_dexclass, t_dexcode,
    t_dexcur, t_dexfield, t_dexhit, t_dexlist, t_dexmember, t_dexmethod, t_dexproto, t_dexstr,
    t_dexvalue, t_span, t_text,
};
use core::ffi::{c_char, c_int};

/// # Safety
/// `d` est nul, ou bien il est aligné et exclusif le temps de l'appel.
/// `file.p` est nul, ou bien il pointe `file.len` octets initialisés.
/// Un pointeur nul n'est pas déréférencé et donne `E_INVAL`. En cas d'échec, `d` est remis à zéro.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_open(d: *mut t_dex, file: t_span) -> c_int {
    let Some(d) = ref_out(d).ok() else {
        return E_INVAL;
    };
    open_at(d, file)
}

/// # Safety
/// `d` est nul, ou bien il est aligné et son tampon `p` reste lisible.
/// `out` est nul, ou bien il est aligné et exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_string(d: *const t_dex, idx: u32, out: *mut t_dexstr) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    string(d, idx, out)
}

/// # Safety
/// `d` et `out` comme pour `dex_string`. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_type(d: *const t_dex, type_idx: u32, out: *mut t_dexstr) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    type_id(d, type_idx, out)
}

/// # Safety
/// `d` est nul, ou bien il est aligné et son tampon reste lisible.
/// `out` est nul, ou bien il est aligné et exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_proto(d: *const t_dex, proto_idx: u32, out: *mut t_dexproto) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    proto(d, proto_idx, out)
}

/// # Safety
/// `d` et `out` sont nuls, ou bien alignés. `out` est exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_field(d: *const t_dex, field_idx: u32, out: *mut t_dexfield) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    field(d, field_idx, out)
}

/// # Safety
/// `d` et `out` sont nuls, ou bien alignés. `out` est exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_method(
    d: *const t_dex,
    method_idx: u32,
    out: *mut t_dexmethod,
) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    method(d, method_idx, out)
}

/// # Safety
/// `d` et `out` sont nuls, ou bien alignés. `out` est exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_class(d: *const t_dex, def_idx: u32, out: *mut t_dexclass) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    class_at(d, def_idx, out)
}

/// # Safety
/// `d` est nul, ou bien il est aligné et son tampon reste lisible.
/// `descriptor` est nul, ou bien une chaîne C terminée par NUL. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_find_class(d: *const t_dex, descriptor: *const c_char) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    if descriptor.is_null() {
        return E_INVAL;
    }
    // SAFETY: descriptor non nul et terminé par NUL.
    let desc = unsafe { core::ffi::CStr::from_ptr(descriptor) }.to_bytes_with_nul();
    find(d, desc)
}

/// # Safety
/// `d`, `c` et `it` sont nuls, ou bien alignés. `it` est exclusif.
/// `d` reste vivant tant que `it` est utilisé. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_cdata_open(
    d: *const t_dex,
    c: *const t_dexclass,
    it: *mut t_dexcdata,
) -> c_int {
    let Some(c) = ref_in(c).ok() else {
        return E_INVAL;
    };
    let Some(it) = ref_out(it).ok() else {
        return E_INVAL;
    };
    if d.is_null() {
        return E_INVAL;
    }
    open(d, c, it)
}

/// # Safety
/// `it` et `out` sont nuls, ou bien alignés. `out` est exclusif.
/// `it.d` est nul, ou bien il pointe un `t_dex` encore lisible. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_cdata_next(it: *mut t_dexcdata, out: *mut t_dexmember) -> c_int {
    let Some(it) = ref_out(it).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    next(it, out)
}

/// # Safety
/// `d` et `out` sont nuls, ou bien alignés. `out` est exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_code(d: *const t_dex, code_off: u32, out: *mut t_dexcode) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    code_item(d, code_off, out)
}

/// # Safety
/// `d` et `code` sont nuls, ou bien alignés et lisibles. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_try_find(d: *const t_dex, code: *const t_dexcode, pc: u32) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(code) = ref_in(code).ok() else {
        return E_INVAL;
    };
    try_find(d, code, pc)
}

/// # Safety
/// `d`, `code` et `it` sont nuls, ou bien alignés. `it` est exclusif.
/// `d` reste vivant tant que `it` sert. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_handler_open(
    d: *const t_dex,
    code: *const t_dexcode,
    try_index: u32,
    it: *mut t_dexhit,
) -> c_int {
    let Some(code) = ref_in(code).ok() else {
        return E_INVAL;
    };
    let Some(it) = ref_out(it).ok() else {
        return E_INVAL;
    };
    if d.is_null() {
        return E_INVAL;
    }
    handler_open(d, code, try_index, it)
}

/// # Safety
/// `it` et `out` sont nuls, ou bien alignés. `out` est exclusif.
/// `it.d` reste lisible. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_handler_next(it: *mut t_dexhit, out: *mut t_dexcatch) -> c_int {
    let Some(it) = ref_out(it).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    handler_next(it, out)
}

/// # Safety
/// `d`, `c` et `out` sont nuls, ou bien alignés. `out` est exclusif.
/// Le tampon de `d` reste lisible. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_static_value(
    d: *const t_dex,
    c: *const t_dexclass,
    ordinal: u32,
    out: *mut t_dexvalue,
) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(c) = ref_in(c).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    static_value(d, c, ordinal, out)
}

/// # Safety
/// `d` et `out` sont nuls, ou bien alignés. `out` est exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_type_list(d: *const t_dex, off: u32, out: *mut t_dexlist) -> c_int {
    let Some(d) = ref_in(d).ok() else {
        return E_INVAL;
    };
    let Some(out) = ref_out(out).ok() else {
        return E_INVAL;
    };
    type_list(d, off, out)
}

/// # Safety
/// `input` est nul, ou bien une chaîne C terminée par NUL.
/// `out.p` est nul, ou bien il pointe `out.cap` octets inscriptibles.
/// Un pointeur nul, une capacité nulle ou supérieure à `0x7fffffff` donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn mutf8_to_utf8(input: *const c_char, out: t_text) -> c_int {
    to_utf8(input, out)
}

/// # Safety
/// `p` est nul, ou bien il pointe `n` octets lisibles. `n == 0` ne lit rien et renvoie 1.
/// `p` nul et `n > 0` ne lit rien et renvoie 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_adler32(p: *const u8, n: usize) -> u32 {
    adler32(p, n)
}

/// # Safety
/// `l` est nul, ou bien il est aligné. Si `i < l.n`, `l.p` est nul ou pointe `2 * l.n` octets.
/// Un pointeur nul ou un indice hors liste renvoie `DEX_NO_INDEX`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_list_at(l: *const t_dexlist, i: u32) -> u32 {
    let Some(l) = ref_in(l).ok() else {
        return DEX_NO_INDEX;
    };
    list_at(l, i)
}

/// # Safety
/// `p` est nul, ou bien il pointe deux octets lisibles. Un pointeur nul renvoie 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_u16(p: *const u8) -> u32 {
    u16_ptr(p).unwrap_or_default()
}

/// # Safety
/// `p` est nul, ou bien il pointe quatre octets lisibles. Un pointeur nul renvoie 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_u32(p: *const u8) -> u32 {
    u32_ptr(p).unwrap_or_default()
}

/// # Safety
/// `c` est nul, ou bien il est aligné et exclusif. `c.p` est nul, ou bien il pointe `c.len` octets.
/// Un pointeur nul n'est pas déréférencé : la fonction renvoie 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_uleb(c: *mut t_dexcur) -> u32 {
    let Some(c) = ref_out(c).ok() else {
        return 0;
    };
    uleb(c)
}

/// # Safety
/// `c` comme pour `dex_uleb`. Un pointeur nul renvoie 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_sleb(c: *mut t_dexcur) -> i32 {
    let Some(c) = ref_out(c).ok() else {
        return 0;
    };
    sleb(c)
}

/// # Safety
/// `c` comme pour `dex_uleb`. Un pointeur nul renvoie 0. Le résultat est non signé : 0 devient `DEX_NO_INDEX`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_ulebp1(c: *mut t_dexcur) -> u32 {
    let Some(c) = ref_out(c).ok() else {
        return 0;
    };
    ulebp1(c)
}

/// # Safety
/// `s.p` est nul, ou bien il pointe `s.len` octets. `size` et `units` sont nuls, ou bien alignés et exclusifs.
/// Un pointeur de sortie nul donne `E_INVAL`. Les sorties ne sont écrites qu'en cas de succès.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dex_mutf8_check(s: t_span, size: *mut u32, units: *mut u32) -> c_int {
    let Some(size) = ref_out(size).ok() else {
        return E_INVAL;
    };
    let Some(units) = ref_out(units).ok() else {
        return E_INVAL;
    };
    check(s, size, units)
}
