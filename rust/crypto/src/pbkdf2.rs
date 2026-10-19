use core::ffi::c_int;

use crate::hmac::{hmac_final_bytes, hmac_prepare_key, hmac_update_bytes, hmac_update_ptr};
use crate::{
    E_INVAL, E_OK, PBKDF2_BLOCKS_MAX, PBKDF2_ITERS_MAX, SHA256_LEN, THmac, TPbkdf2, blank_hmac,
    wipe,
};

fn too_many_blocks(outlen: usize) -> bool {
    (outlen - 1) / SHA256_LEN >= PBKDF2_BLOCKS_MAX
}

pub(crate) fn pbkdf2_check(p: &TPbkdf2, outlen: usize) -> c_int {
    if outlen == 0 || p.iters < 1 || p.iters > PBKDF2_ITERS_MAX {
        return E_INVAL;
    }
    if (p.pw.is_null() && p.pwlen != 0) || (p.salt.is_null() && p.saltlen != 0) {
        return E_INVAL;
    }
    if too_many_blocks(outlen) {
        return E_INVAL;
    }
    E_OK
}

fn xor_into(t: &mut [u8; SHA256_LEN], u: &[u8; SHA256_LEN]) {
    for (tb, ub) in t.iter_mut().zip(u.iter()) {
        *tb ^= *ub;
    }
}

fn pbkdf2_first(tmpl: &THmac, p: &TPbkdf2, idx: u32, u: &mut [u8; SHA256_LEN]) {
    let mut h = *tmpl;
    let be = idx.to_be_bytes();
    hmac_update_ptr(&mut h, p.salt, p.saltlen);
    hmac_update_bytes(&mut h, &be);
    hmac_final_bytes(&mut h, u);
}

fn pbkdf2_block(tmpl: &THmac, p: &TPbkdf2, idx: u32, t: &mut [u8; SHA256_LEN]) {
    let mut u = [0u8; SHA256_LEN];
    pbkdf2_first(tmpl, p, idx, &mut u);
    *t = u;
    let mut i = 1u32;
    while i < p.iters {
        let mut h = *tmpl;
        hmac_update_bytes(&mut h, &u);
        hmac_final_bytes(&mut h, &mut u);
        xor_into(t, &u);
        i = i.wrapping_add(1);
    }
    wipe(&mut u);
}

fn pbkdf2_run(p: &TPbkdf2, out: &mut [u8]) {
    let mut tmpl = blank_hmac();
    hmac_prepare_key(&mut tmpl, p.pw, p.pwlen);
    let mut idx = 1u32;
    let mut done = 0usize;
    let mut t = [0u8; SHA256_LEN];
    while done < out.len() {
        pbkdf2_block(&tmpl, p, idx, &mut t);
        let n = (out.len() - done).min(SHA256_LEN);
        out[done..done + n].copy_from_slice(&t[..n]);
        done += n;
        idx = idx.wrapping_add(1);
    }
    wipe(&mut tmpl);
    wipe(&mut t);
}

#[unsafe(no_mangle)]
/// # Safety
/// `p` est nul ou pointe un `t_pbkdf2` lisible. La fonction n'écrit rien.
pub unsafe extern "C" fn pbkdf2_validate(p: *const TPbkdf2, outlen: usize) -> c_int {
    if p.is_null() {
        return E_INVAL;
    }
    // SAFETY: p n'est pas nul et pointe un t_pbkdf2 lisible.
    let p = unsafe { &*p };
    pbkdf2_check(p, outlen)
}

#[unsafe(no_mangle)]
/// # Safety
/// `p` est nul ou pointe un `t_pbkdf2` lisible. `out` est nul ou pointe `outlen` octets.
/// Retourne `E_INVAL` sans écrire si les paramètres sont refusés ou si `out` est nul.
pub unsafe extern "C" fn pbkdf2_sha256(p: *const TPbkdf2, out: *mut u8, outlen: usize) -> c_int {
    if p.is_null() {
        return E_INVAL;
    }
    // SAFETY: p n'est pas nul. out n'est lu que s'il est non nul et si outlen est accepté.
    unsafe {
        let params = &*p;
        if pbkdf2_check(params, outlen) < 0 || out.is_null() {
            return E_INVAL;
        }
        let dst = core::slice::from_raw_parts_mut(out, outlen);
        pbkdf2_run(params, dst);
    }
    E_OK
}
