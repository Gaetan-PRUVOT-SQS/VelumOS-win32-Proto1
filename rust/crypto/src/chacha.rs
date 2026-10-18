use crate::{TChacha20, wipe};

const KEY_LEN: usize = 32;
const NONCE_LEN: usize = 12;
const BLOCK_LEN: usize = 64;

fn load_le32(p: &[u8]) -> u32 {
    u32::from_le_bytes([p[0], p[1], p[2], p[3]])
}

fn store_le32(p: &mut [u8], v: u32) {
    p[..4].copy_from_slice(&v.to_le_bytes());
}

fn rotl(x: u32, n: u32) -> u32 {
    x.rotate_left(n)
}

fn qr(s: &mut [u32; 16], a: usize, b: usize, c: usize, d: usize) {
    s[a] = s[a].wrapping_add(s[b]);
    s[d] = rotl(s[d] ^ s[a], 16);
    s[c] = s[c].wrapping_add(s[d]);
    s[b] = rotl(s[b] ^ s[c], 12);
    s[a] = s[a].wrapping_add(s[b]);
    s[d] = rotl(s[d] ^ s[a], 8);
    s[c] = s[c].wrapping_add(s[d]);
    s[b] = rotl(s[b] ^ s[c], 7);
}

fn double_round(x: &mut [u32; 16]) {
    qr(x, 0, 4, 8, 12);
    qr(x, 1, 5, 9, 13);
    qr(x, 2, 6, 10, 14);
    qr(x, 3, 7, 11, 15);
    qr(x, 0, 5, 10, 15);
    qr(x, 1, 6, 11, 12);
    qr(x, 2, 7, 8, 13);
    qr(x, 3, 4, 9, 14);
}

fn chacha20_state(key: &[u8; KEY_LEN], nonce: &[u8; NONCE_LEN], counter: u32) -> TChacha20 {
    let mut state = [0u32; 16];
    state[0] = 0x6170_7865;
    state[1] = 0x3320_646e;
    state[2] = 0x7962_2d32;
    state[3] = 0x6b20_6574;
    let mut i = 0usize;
    while i < 8 {
        state[4 + i] = load_le32(&key[i * 4..i * 4 + 4]);
        i += 1;
    }
    state[12] = counter;
    i = 0;
    while i < 3 {
        state[13 + i] = load_le32(&nonce[i * 4..i * 4 + 4]);
        i += 1;
    }
    TChacha20 { state }
}

fn chacha20_block_bytes(c: &mut TChacha20, out: &mut [u8; BLOCK_LEN]) {
    let mut x = c.state;
    let mut i = 0usize;
    while i < 10 {
        double_round(&mut x);
        i += 1;
    }
    i = 0;
    while i < 16 {
        store_le32(&mut out[i * 4..i * 4 + 4], x[i].wrapping_add(c.state[i]));
        i += 1;
    }
    c.state[12] = c.state[12].wrapping_add(1);
    wipe(&mut x);
}

#[unsafe(no_mangle)]
/// # Safety
/// `c` est nul ou pointe un `t_chacha20` accessible en écriture.
/// `key` pointe 32 octets et `nonce` 12, ou l'un des trois pointeurs est nul : alors rien n'est écrit.
pub unsafe extern "C" fn chacha20_init(
    c: *mut TChacha20,
    key: *const u8,
    nonce: *const u8,
    counter: u32,
) {
    if c.is_null() || key.is_null() || nonce.is_null() {
        return;
    }
    // SAFETY: clé et nonce sont non nuls, 32 et 12 octets. c reçoit l'état, sans lecture préalable.
    unsafe {
        let key = &*key.cast::<[u8; KEY_LEN]>();
        let nonce = &*nonce.cast::<[u8; NONCE_LEN]>();
        c.write(chacha20_state(key, nonce, counter));
    }
}

#[unsafe(no_mangle)]
/// # Safety
/// `c` est nul ou pointe un état initialisé. `out` est nul ou pointe 64 octets.
/// Si l'un est nul, l'état n'est pas modifié.
pub unsafe extern "C" fn chacha20_block(c: *mut TChacha20, out: *mut u8) {
    if c.is_null() || out.is_null() {
        return;
    }
    // SAFETY: c et out sont non nuls, état initialisé et 64 octets accessibles.
    unsafe { chacha20_block_bytes(&mut *c, &mut *out.cast::<[u8; BLOCK_LEN]>()) };
}
