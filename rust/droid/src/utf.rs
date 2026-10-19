use core::ffi::c_char;

use crate::abi::{memcpy, t_dstr16};

fn cp(s: t_dstr16, i: &mut u32) -> u32 {
    // SAFETY: i < s.n et s.p pointe s.n unités. L'appelant ne passe pas un pointeur nul.
    let c = unsafe { s.p.add(*i as usize).read() } as u32;
    *i += 1;
    if (0xd800..=0xdbff).contains(&c) && *i < s.n {
        // SAFETY: *i < s.n, la seconde unité est dans le tampon.
        let n = unsafe { s.p.add(*i as usize).read() } as u32;
        if (0xdc00..=0xdfff).contains(&n) {
            *i += 1;
            return 0x10000 + ((c - 0xd800) << 10) + (n - 0xdc00);
        }
    }
    if (0xd800..=0xdfff).contains(&c) {
        return 0xfffd;
    }
    c
}

fn enc(c: u32, b: &mut [u8; 4]) -> usize {
    if c < 0x80 {
        b[0] = c as u8;
        return 1;
    }
    if c < 0x800 {
        b[0] = (0xc0 | (c >> 6)) as u8;
        b[1] = (0x80 | (c & 0x3f)) as u8;
        return 2;
    }
    if c < 0x10000 {
        b[0] = (0xe0 | (c >> 12)) as u8;
        b[1] = (0x80 | ((c >> 6) & 0x3f)) as u8;
        b[2] = (0x80 | (c & 0x3f)) as u8;
        return 3;
    }
    b[0] = (0xf0 | (c >> 18)) as u8;
    b[1] = (0x80 | ((c >> 12) & 0x3f)) as u8;
    b[2] = (0x80 | ((c >> 6) & 0x3f)) as u8;
    b[3] = (0x80 | (c & 0x3f)) as u8;
    4
}

/// # Safety
/// `s.p` est nul seulement si `s.n` vaut 0, sinon il pointe `s.n` unités UTF-16.
/// `out` est nul seulement si `cap` vaut 0, sinon il a `cap` octets exclusifs.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_utf8(s: t_dstr16, out: *mut c_char, cap: usize) -> usize {
    let mut i = 0u32;
    let mut n = 0usize;
    if cap == 0 {
        return 0;
    }
    while i < s.n {
        let mut b = [0u8; 4];
        let k = enc(cp(s, &mut i), &mut b);
        if n + k >= cap {
            break;
        }
        // SAFETY: out a cap octets, n + k < cap, b contient k octets initialisés.
        unsafe { memcpy(out.add(n).cast(), b.as_ptr().cast(), k) };
        n += k;
    }
    // SAFETY: n < cap, out[n] est dans le tampon.
    unsafe { out.add(n).write(0) };
    n
}

/// # Safety
/// `out` pointe au moins 20 unités UTF-16 exclusives.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_dec16(v: i64, out: *mut u16) -> u32 {
    let mut tmp = [0u16; 20];
    let mut u = v as u64;
    if v < 0 {
        u = u.wrapping_neg();
    }
    let mut n = 0u32;
    while u != 0 || n == 0 {
        tmp[n as usize] = u16::from(b'0') + (u % 10) as u16;
        u /= 10;
        n += 1;
    }
    let mut k = 0u32;
    if v < 0 {
        // SAFETY: out a de la place pour le signe et les chiffres, au plus 20 unités.
        unsafe { out.add(k as usize).write(u16::from(b'-')) };
        k += 1;
    }
    while n > 0 {
        n -= 1;
        // SAFETY: k reste inférieur à 20, out est exclusif.
        unsafe { out.add(k as usize).write(tmp[n as usize]) };
        k += 1;
    }
    k
}

/// # Safety
/// `desc` est nul ou une chaîne C. `out` est nul seulement si `cap` vaut 0, sinon il a `cap` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_dotted(desc: *const c_char, out: *mut c_char, cap: usize) -> usize {
    let mut n = 0usize;
    let mut i = 0usize;
    if desc.is_null() || cap == 0 {
        return 0;
    }
    // SAFETY: desc est une chaîne C. La boucle s'arrête au premier NUL.
    let lead = unsafe { desc.read() };
    if lead == b'L' as c_char {
        i = 1;
    }
    loop {
        // SAFETY: i reste sur un octet de la chaîne C, NUL compris.
        let ch = unsafe { desc.add(i).read() };
        if ch == 0 || n + 1 >= cap || (lead == b'L' as c_char && ch == b';' as c_char) {
            break;
        }
        let mut byte = ch as u8;
        if byte == b'/' {
            byte = b'.';
        }
        // SAFETY: n + 1 < cap, out[n] est dans le tampon.
        unsafe { out.add(n).write(byte as c_char) };
        n += 1;
        i += 1;
    }
    // SAFETY: n < cap.
    unsafe { out.add(n).write(0) };
    n
}
