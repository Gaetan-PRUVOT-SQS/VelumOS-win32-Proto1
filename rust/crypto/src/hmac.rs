use core::ffi::c_void;

use crate::sha256::{sha256_bytes, sha256_final_bytes, sha256_init_bytes, sha256_update_bytes};
use crate::{SHA256_BLOCK, SHA256_LEN, THmac, TPbkdf2, blank_hmac, wipe};

fn xor_block(k: &mut [u8; SHA256_BLOCK], v: u8) {
    for b in k {
        *b ^= v;
    }
}

pub(crate) fn hmac_prepare_bytes(h: &mut THmac, key: &[u8]) {
    let mut k = [0u8; SHA256_BLOCK];
    if key.len() > SHA256_BLOCK {
        let dig = sha256_bytes(key);
        k[..SHA256_LEN].copy_from_slice(&dig);
    } else {
        k[..key.len()].copy_from_slice(key);
    }
    xor_block(&mut k, 0x36);
    sha256_init_bytes(&mut h.inner);
    sha256_update_bytes(&mut h.inner, &k);
    xor_block(&mut k, 0x36 ^ 0x5c);
    sha256_init_bytes(&mut h.outer);
    sha256_update_bytes(&mut h.outer, &k);
    wipe(&mut k);
}

pub(crate) fn hmac_prepare_key(h: &mut THmac, key: *const u8, keylen: usize) {
    if key.is_null() || keylen == 0 {
        hmac_prepare_bytes(h, &[]);
        return;
    }
    // SAFETY: keylen > 0 et key n'est pas nul. L'appelant garantit keylen octets.
    let bytes = unsafe { core::slice::from_raw_parts(key, keylen) };
    hmac_prepare_bytes(h, bytes);
}

pub(crate) fn hmac_update_bytes(h: &mut THmac, data: &[u8]) {
    sha256_update_bytes(&mut h.inner, data);
}

pub(crate) fn hmac_update_ptr(h: &mut THmac, data: *const u8, n: usize) {
    if n == 0 || data.is_null() {
        return;
    }
    // SAFETY: data n'est pas nul et couvre n octets lisibles.
    let bytes = unsafe { core::slice::from_raw_parts(data, n) };
    hmac_update_bytes(h, bytes);
}

pub(crate) fn hmac_final_bytes(h: &mut THmac, out: &mut [u8; SHA256_LEN]) {
    let mut digest = [0u8; SHA256_LEN];
    sha256_final_bytes(&mut h.inner, &mut digest);
    sha256_update_bytes(&mut h.outer, &digest);
    sha256_final_bytes(&mut h.outer, out);
    wipe(&mut digest);
}

fn hmac_mac(key: *const u8, keylen: usize, msg: &[u8]) -> [u8; SHA256_LEN] {
    let mut h = blank_hmac();
    hmac_prepare_key(&mut h, key, keylen);
    hmac_update_bytes(&mut h, msg);
    let mut out = [0u8; SHA256_LEN];
    hmac_final_bytes(&mut h, &mut out);
    out
}

#[unsafe(no_mangle)]
/// # Safety
/// `h` est nul ou pointe un `t_hmac` accessible en écriture, même non initialisé.
/// `key` est nul ou pointe `keylen` octets. Une clé nulle est une clé vide, quelle que soit `keylen`.
pub unsafe extern "C" fn hmac_prepare(h: *mut THmac, key: *const u8, keylen: usize) {
    if h.is_null() {
        return;
    }
    // SAFETY: h n'est pas nul et couvre un t_hmac. L'effacement précède la référence.
    unsafe {
        crate::secure_zero(h.cast(), core::mem::size_of::<THmac>());
        hmac_prepare_key(&mut *h, key, keylen);
    }
}

#[unsafe(no_mangle)]
/// # Safety
/// `h` est nul ou pointe un HMAC préparé. `data` est nul ou pointe `n` octets.
/// `n` nul, ou un `data` nul, ne modifie pas `h`.
pub unsafe extern "C" fn hmac_update(h: *mut THmac, data: *const c_void, n: usize) {
    if h.is_null() || n == 0 || data.is_null() {
        return;
    }
    // SAFETY: h n'est pas nul et pointe un HMAC préparé. data couvre n octets.
    unsafe {
        let bytes = core::slice::from_raw_parts(data.cast::<u8>(), n);
        hmac_update_bytes(&mut *h, bytes);
    }
}

#[unsafe(no_mangle)]
/// # Safety
/// `h` est nul ou pointe un HMAC préparé. `out` est nul ou pointe 32 octets.
/// Si l'un est nul, `h` n'est pas modifié.
pub unsafe extern "C" fn hmac_final(h: *mut THmac, out: *mut u8) {
    if h.is_null() || out.is_null() {
        return;
    }
    // SAFETY: h et out sont non nuls, HMAC préparé et 32 octets accessibles.
    unsafe { hmac_final_bytes(&mut *h, &mut *out.cast::<[u8; SHA256_LEN]>()) };
}

#[unsafe(no_mangle)]
/// # Safety
/// `k` est nul (clé vide) ou pointe un `t_pbkdf2`. `m` est nul ou pointe `n` octets.
/// `o` est nul ou pointe 32 octets. `n` nul accepte `m` nul. Sinon rien n'est écrit si `m` ou `o` manque.
/// Une clé dont `pw` est nul est vide, même si `pwlen` est non nul.
pub unsafe extern "C" fn hmac_sha256(k: *const TPbkdf2, m: *const c_void, n: usize, o: *mut u8) {
    if o.is_null() {
        return;
    }
    if n > 0 && m.is_null() {
        return;
    }
    let (pw, pwlen) = if k.is_null() {
        (core::ptr::null(), 0)
    } else {
        // SAFETY: k n'est pas nul et pointe un t_pbkdf2 lisible.
        let k = unsafe { &*k };
        (k.pw, k.pwlen)
    };
    let msg = if n == 0 {
        &[][..]
    } else {
        // SAFETY: n > 0 et m n'est pas nul. L'appelant garantit n octets.
        unsafe { core::slice::from_raw_parts(m.cast::<u8>(), n) }
    };
    let mac = hmac_mac(pw, pwlen, msg);
    // SAFETY: o n'est pas nul et pointe 32 octets.
    let dst = unsafe { &mut *o.cast::<[u8; SHA256_LEN]>() };
    *dst = mac;
}
