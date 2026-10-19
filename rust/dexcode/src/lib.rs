#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

use core::ffi::{c_char, c_int};

mod check;
mod decode;
mod payload;
mod read;
mod tab;
mod verify;

pub(crate) const E_OK: c_int = 0;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_RANGE: c_int = -34;
pub(crate) const E_NOTSUP: c_int = -95;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_span {
    pub p: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_dinsn {
    pub op: u8,
    pub fmt: u8,
    pub len: u8,
    pub argc: u8,
    pub a: u32,
    pub b: u32,
    pub c: u32,
    pub lit: i64,
    pub idx: u32,
    pub idx2: u32,
    pub args: [u16; 5],
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_dcodelimits {
    pub registers: u32,
    pub ins: u32,
    pub outs: u32,
    pub strings: u32,
    pub types: u32,
    pub fields: u32,
    pub methods: u32,
    pub protos: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_dswitch {
    pub sparse: u8,
    pub count: u32,
    pub first_key: i32,
    pub keys: *const u8,
    pub targets: *const u8,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[allow(non_camel_case_types)]
pub struct t_darray {
    pub width: u32,
    pub count: u32,
    pub data: *const u8,
}

/// # Safety
/// `p` est nul, ou bien il pointe `len` octets initialisés et le prêt dure `'a`.
/// `len` au-delà de `isize::MAX` donne `E_INVAL`.
unsafe fn bytes_of<'a>(p: *const u8, len: usize) -> Result<&'a [u8], c_int> {
    if p.is_null() || len > isize::MAX as usize {
        return Err(E_INVAL);
    }
    if len == 0 {
        return Ok(&[]);
    }
    // SAFETY: p non nul, len borné par isize::MAX, len octets initialisés pour 'a.
    Ok(unsafe { core::slice::from_raw_parts(p, len) })
}

/// # Safety
/// `base` pointe `len` octets. `off` est au plus `len`.
unsafe fn ptr_at(base: *const u8, len: usize, off: usize) -> *const u8 {
    if base.is_null() || off > len || off > isize::MAX as usize {
        return core::ptr::null();
    }
    if off == 0 {
        return base;
    }
    // SAFETY: off <= len <= isize::MAX, base vise cette allocation. add reste dedans ou juste après.
    unsafe { base.add(off) }
}

/// # Safety
/// `insns.p` est nul, ou bien il pointe `insns.len` octets initialisés.
/// `out` est nul, ou bien il est aligné et exclusif le temps de l'appel.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_decode(insns: t_span, pc: u32, out: *mut t_dinsn) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: contrat de dexcode_decode sur insns.
    let code = match unsafe { bytes_of(insns.p, insns.len) } {
        Ok(code) => code,
        Err(e) => return e,
    };
    // SAFETY: out non nul et aligné, accès exclusif pour l'appel.
    let out = unsafe { &mut *out };
    decode::decode_at(code, pc, out)
}

/// # Safety
/// Aucune. `op` couvre tout `u8`. Le pointeur rendu vise une chaîne statique.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_name(op: u8) -> *const c_char {
    tab::op_info(op).name.as_ptr()
}

/// # Safety
/// Aucune. `op` couvre tout `u8`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_format(op: u8) -> u8 {
    tab::op_info(op).fmt
}

/// # Safety
/// Aucune. `op` couvre tout `u8`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_index_kind(op: u8) -> u8 {
    tab::op_info(op).kind
}

/// # Safety
/// `insns.p` est nul, ou bien il pointe `insns.len` octets initialisés.
/// `lim` est nul, ou bien il est aligné et reste valide. `ins` supérieur à `registers` donne `E_INVAL`.
/// `starts` est nul, ou bien il pointe `(unités + 7) / 8` octets exclusifs, disjoints de `insns`.
/// Longueur nulle, impaire, ou au-delà de 2^32 unités : `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_verify(
    insns: t_span,
    lim: *const t_dcodelimits,
    starts: *mut u8,
) -> c_int {
    if lim.is_null() || !lim.is_aligned() || starts.is_null() {
        return E_INVAL;
    }
    if insns.len == 0 || insns.len & 1 != 0 || insns.len / 2 > u32::MAX as usize {
        return E_INVAL;
    }
    // SAFETY: lim non nul et aligné, lecture seule le temps de l'appel.
    let lim = unsafe { &*lim };
    if lim.ins > lim.registers {
        return E_INVAL;
    }
    // SAFETY: contrat de dexcode_verify sur insns.
    let code = match unsafe { bytes_of(insns.p, insns.len) } {
        Ok(code) => code,
        Err(e) => return e,
    };
    let Ok(n) = u32::try_from(code.len() / 2) else {
        return E_INVAL;
    };
    let Ok(nbytes) = usize::try_from(u64::from(n).div_ceil(8)) else {
        return E_INVAL;
    };
    if nbytes == 0 || nbytes > isize::MAX as usize {
        return E_INVAL;
    }
    // SAFETY: starts non nul, nbytes octets exclusifs, disjoints de code.
    let starts = unsafe { core::slice::from_raw_parts_mut(starts, nbytes) };
    verify::verify_code(code, lim, starts)
}

/// # Safety
/// `insns.p` est nul, ou bien il pointe `insns.len` octets initialisés.
/// `out` est nul, ou bien il est aligné et exclusif le temps de l'appel.
/// `payload_pc` impair, hors code, ou charge illisible : `E_INVAL` sans écrire `out`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_switch(
    insns: t_span,
    payload_pc: u32,
    out: *mut t_dswitch,
) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: contrat de dexcode_switch sur insns.
    let code = match unsafe { bytes_of(insns.p, insns.len) } {
        Ok(code) => code,
        Err(e) => return e,
    };
    let view = match payload::switch_at(code, payload_pc) {
        Ok(view) => view,
        Err(e) => return e,
    };
    // SAFETY: out non nul et aligné, accès exclusif pour l'appel.
    let out = unsafe { &mut *out };
    out.sparse = view.sparse;
    out.count = view.count;
    out.first_key = view.first_key;
    out.keys = match view.keys_off {
        Some(off) => {
            // SAFETY: keys_off est au plus code.len(), base est insns.p.
            unsafe { ptr_at(insns.p, code.len(), off) }
        }
        None => core::ptr::null(),
    };
    // SAFETY: targets_off est au plus code.len().
    out.targets = unsafe { ptr_at(insns.p, code.len(), view.targets_off) };
    E_OK
}

/// # Safety
/// `insns.p` est nul, ou bien il pointe `insns.len` octets initialisés.
/// `out` est nul, ou bien il est aligné et exclusif le temps de l'appel.
/// Charge qui n'est pas un tableau : `E_INVAL` sans écrire `out`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_array_data(
    insns: t_span,
    payload_pc: u32,
    out: *mut t_darray,
) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: contrat de dexcode_array_data sur insns.
    let code = match unsafe { bytes_of(insns.p, insns.len) } {
        Ok(code) => code,
        Err(e) => return e,
    };
    let view = match payload::array_at(code, payload_pc) {
        Ok(view) => view,
        Err(e) => return e,
    };
    // SAFETY: out non nul et aligné, accès exclusif pour l'appel.
    let out = unsafe { &mut *out };
    out.width = view.width;
    out.count = view.count;
    // SAFETY: data_off est au plus code.len().
    out.data = unsafe { ptr_at(insns.p, code.len(), view.data_off) };
    E_OK
}

/// # Safety
/// Aucune. `op` couvre tout `u8`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_flags(op: u8) -> u16 {
    tab::op_info(op).flags
}

/// # Safety
/// Aucune. Un format hors `0..DF_COUNT` vaut 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_format_len(fmt: u8) -> u8 {
    tab::fmt_info(fmt).len
}

/// # Safety
/// `p` est nul, ou bien il pointe 4 octets initialisés. Nul rend 0, sans déréférencement.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dexcode_rd32(p: *const u8) -> i32 {
    if p.is_null() {
        return 0;
    }
    // SAFETY: p non nul. Le contrat exige 4 octets initialisés.
    let bytes = unsafe { core::slice::from_raw_parts(p, 4) };
    read::rd32_bytes(bytes).unwrap_or(0)
}

#[cfg(test)]
mod tests;
