use core::ffi::c_void;

use crate::{SHA256_BLOCK, SHA256_LEN, TSha256, blank_sha, wipe};

const IV: [u32; 8] = [
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
];

const K: [u32; 64] = [
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
];

fn rotr(x: u32, n: u32) -> u32 {
    x.rotate_right(n)
}

fn ch(x: u32, y: u32, z: u32) -> u32 {
    (x & y) ^ (!x & z)
}

fn maj(x: u32, y: u32, z: u32) -> u32 {
    (x & y) ^ (x & z) ^ (y & z)
}

fn big(x: u32, a: u32, b: u32, c: u32) -> u32 {
    rotr(x, a) ^ rotr(x, b) ^ rotr(x, c)
}

fn small(x: u32, a: u32, b: u32, c: u32) -> u32 {
    rotr(x, a) ^ rotr(x, b) ^ (x >> c)
}

fn load_be32(p: &[u8]) -> u32 {
    u32::from_be_bytes([p[0], p[1], p[2], p[3]])
}

fn sha256_compress(h: &mut [u32; 8], block: &[u8; SHA256_BLOCK]) {
    let mut w = [0u32; 64];
    let mut i = 0usize;
    while i < 16 {
        w[i] = load_be32(&block[i * 4..i * 4 + 4]);
        i += 1;
    }
    while i < 64 {
        w[i] = small(w[i - 2], 17, 19, 10)
            .wrapping_add(w[i - 7])
            .wrapping_add(small(w[i - 15], 7, 18, 3))
            .wrapping_add(w[i - 16]);
        i += 1;
    }
    let mut s = *h;
    i = 0;
    while i < 64 {
        let kw = K[i].wrapping_add(w[i]);
        let t1 = s[7]
            .wrapping_add(big(s[4], 6, 11, 25))
            .wrapping_add(ch(s[4], s[5], s[6]))
            .wrapping_add(kw);
        let t2 = big(s[0], 2, 13, 22).wrapping_add(maj(s[0], s[1], s[2]));
        s[7] = s[6];
        s[6] = s[5];
        s[5] = s[4];
        s[4] = s[3].wrapping_add(t1);
        s[3] = s[2];
        s[2] = s[1];
        s[1] = s[0];
        s[0] = t1.wrapping_add(t2);
        i += 1;
    }
    for (dst, src) in h.iter_mut().zip(s.iter()) {
        *dst = dst.wrapping_add(*src);
    }
    wipe(&mut w);
    wipe(&mut s);
}

pub(crate) fn sha256_init_bytes(c: &mut TSha256) {
    wipe(c);
    c.h = IV;
    c.total = 0;
    c.fill = 0;
}

pub(crate) fn sha256_update_bytes(c: &mut TSha256, mut data: &[u8]) {
    if data.is_empty() {
        return;
    }
    c.total = c.total.wrapping_add(data.len() as u64);
    if c.fill != 0 {
        let fill = c.fill as usize;
        let take = (SHA256_BLOCK - fill).min(data.len());
        c.buf[fill..fill + take].copy_from_slice(&data[..take]);
        c.fill = (fill + take) as u32;
        data = &data[take..];
        if c.fill == SHA256_BLOCK as u32 {
            let block = c.buf;
            sha256_compress(&mut c.h, &block);
            c.fill = 0;
        }
    }
    while data.len() >= SHA256_BLOCK {
        let mut block = [0u8; SHA256_BLOCK];
        block.copy_from_slice(&data[..SHA256_BLOCK]);
        sha256_compress(&mut c.h, &block);
        data = &data[SHA256_BLOCK..];
    }
    if !data.is_empty() {
        let n = data.len();
        c.buf[..n].copy_from_slice(data);
        c.fill = n as u32;
    }
}

fn sha256_pad(c: &mut TSha256) {
    let bits = c.total.wrapping_shl(3);
    let mut fill = c.fill as usize;
    c.buf[fill] = 0x80;
    fill += 1;
    if fill > SHA256_BLOCK - 8 {
        c.buf[fill..].fill(0);
        let block = c.buf;
        sha256_compress(&mut c.h, &block);
        fill = 0;
    }
    c.buf[fill..SHA256_BLOCK - 8].fill(0);
    c.buf[SHA256_BLOCK - 8..].copy_from_slice(&bits.to_be_bytes());
    let block = c.buf;
    sha256_compress(&mut c.h, &block);
    c.fill = fill as u32;
}

pub(crate) fn sha256_final_bytes(c: &mut TSha256, out: &mut [u8; SHA256_LEN]) {
    sha256_pad(c);
    let mut i = 0usize;
    while i < 8 {
        let be = c.h[i].to_be_bytes();
        out[i * 4..i * 4 + 4].copy_from_slice(&be);
        i += 1;
    }
    wipe(c);
}

pub(crate) fn sha256_bytes(data: &[u8]) -> [u8; SHA256_LEN] {
    let mut c = blank_sha();
    sha256_init_bytes(&mut c);
    sha256_update_bytes(&mut c, data);
    let mut out = [0u8; SHA256_LEN];
    sha256_final_bytes(&mut c, &mut out);
    out
}

fn bytes_or_empty<'a>(data: *const c_void, n: usize) -> Option<&'a [u8]> {
    if n == 0 {
        return Some(&[]);
    }
    if data.is_null() {
        return None;
    }
    // SAFETY: n > 0 et data n'est pas nul. L'appelant garantit n octets lisibles.
    Some(unsafe { core::slice::from_raw_parts(data.cast::<u8>(), n) })
}

#[unsafe(no_mangle)]
/// # Safety
/// `c` est nul, ou bien il pointe un `t_sha256` accessible en écriture, même non initialisé.
pub unsafe extern "C" fn sha256_init(c: *mut TSha256) {
    if c.is_null() {
        return;
    }
    // SAFETY: c n'est pas nul et couvre un t_sha256. L'effacement précède la référence.
    unsafe {
        crate::secure_zero(c.cast(), core::mem::size_of::<TSha256>());
        (*c).h = IV;
    }
}

#[unsafe(no_mangle)]
/// # Safety
/// `c` est nul ou pointe un contexte initialisé. `data` est nul ou pointe `n` octets.
/// `n` nul ne lit pas `data`. Un pointeur nul avec `n` > 0 ne modifie pas le contexte.
pub unsafe extern "C" fn sha256_update(c: *mut TSha256, data: *const c_void, n: usize) {
    if c.is_null() || n == 0 {
        return;
    }
    if data.is_null() {
        return;
    }
    // SAFETY: n > 0, data et c sont non nuls, contexte initialisé, data couvre n octets.
    unsafe {
        let bytes = core::slice::from_raw_parts(data.cast::<u8>(), n);
        sha256_update_bytes(&mut *c, bytes);
    }
}

#[unsafe(no_mangle)]
/// # Safety
/// `c` est nul ou pointe un contexte initialisé. `out` est nul ou pointe 32 octets.
/// Si l'un est nul, le contexte n'est pas modifié.
pub unsafe extern "C" fn sha256_final(c: *mut TSha256, out: *mut u8) {
    if c.is_null() || out.is_null() {
        return;
    }
    // SAFETY: c et out sont non nuls, contexte initialisé et 32 octets accessibles.
    unsafe { sha256_final_bytes(&mut *c, &mut *out.cast::<[u8; SHA256_LEN]>()) };
}

#[unsafe(no_mangle)]
/// # Safety
/// `data` est nul ou pointe `n` octets. `out` est nul ou pointe 32 octets.
/// `n` nul accepte un `data` nul. Sinon rien n'est écrit si un pointeur manque.
pub unsafe extern "C" fn sha256(data: *const c_void, n: usize, out: *mut u8) {
    if out.is_null() {
        return;
    }
    let Some(bytes) = bytes_or_empty(data, n) else {
        return;
    };
    let dig = sha256_bytes(bytes);
    // SAFETY: out n'est pas nul et pointe 32 octets.
    let dst = unsafe { &mut *out.cast::<[u8; SHA256_LEN]>() };
    *dst = dig;
}
