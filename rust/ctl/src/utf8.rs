use core::ffi::c_char;

use crate::raw::bytes;

fn lead_need(b: u8) -> u32 {
    if (0xc2..=0xdf).contains(&b) {
        return 1;
    }
    if (0xe0..=0xef).contains(&b) {
        return 2;
    }
    if (0xf0..=0xf4).contains(&b) {
        return 3;
    }
    0
}

fn second_ok(lead: u8, b: u8) -> bool {
    if lead == 0xe0 {
        return (0xa0..=0xbf).contains(&b);
    }
    if lead == 0xed {
        return (0x80..=0x9f).contains(&b);
    }
    if lead == 0xf0 {
        return (0x90..=0xbf).contains(&b);
    }
    if lead == 0xf4 {
        return (0x80..=0x8f).contains(&b);
    }
    b & 0xc0 == 0x80
}

fn u8_len(s: &[u8]) -> u32 {
    if s.is_empty() {
        return 0;
    }
    if s[0] < 0x80 {
        return 1;
    }
    let need = lead_need(s[0]);
    if need == 0 || need >= s.len() as u32 || !second_ok(s[0], s[1]) {
        return 1;
    }
    let mut i = 2u32;
    while i <= need {
        if s[i as usize] & 0xc0 != 0x80 {
            return 1;
        }
        i += 1;
    }
    need + 1
}

/// # Safety
/// `avail` vaut 0, ou `s` pointe au moins `avail` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_u8_len(s: *const c_char, avail: u32) -> u32 {
    u8_len(bytes(s, avail))
}

/// # Safety
/// `pos` octets à partir de `s` sont lisibles si `pos < len`. `s` couvre `len` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_u8_next(s: *const c_char, len: u32, pos: u32) -> u32 {
    if pos >= len {
        return len;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    pos + u8_len(bytes(unsafe { s.add(pos as usize) }, len - pos))
}

/// # Safety
/// `s` couvre `len` octets, ou `len` vaut 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_u8_count(s: *const c_char, len: u32) -> u32 {
    let mut pos = 0u32;
    let mut n = 0u32;
    while pos < len {
        // SAFETY: pos < len et s couvre len octets.
        pos = unsafe { ctl_u8_next(s, len, pos) };
        n += 1;
    }
    n
}

fn enc2(cp: u32, out: &mut [u8]) -> u32 {
    out[0] = (0xc0 | (cp >> 6)) as u8;
    out[1] = (0x80 | (cp & 0x3f)) as u8;
    2
}

fn enc3(cp: u32, out: &mut [u8]) -> u32 {
    if (0xd800..=0xdfff).contains(&cp) {
        return 0;
    }
    out[0] = (0xe0 | (cp >> 12)) as u8;
    out[1] = (0x80 | ((cp >> 6) & 0x3f)) as u8;
    out[2] = (0x80 | (cp & 0x3f)) as u8;
    3
}

fn enc4(cp: u32, out: &mut [u8]) -> u32 {
    if cp > 0x10ffff {
        return 0;
    }
    out[0] = (0xf0 | (cp >> 18)) as u8;
    out[1] = (0x80 | ((cp >> 12) & 0x3f)) as u8;
    out[2] = (0x80 | ((cp >> 6) & 0x3f)) as u8;
    out[3] = (0x80 | (cp & 0x3f)) as u8;
    4
}

/// # Safety
/// `out` a la place du résultat : 1 à 4 octets. Un point de code refusé n'écrit rien.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_u8_encode(cp: u32, out: *mut c_char) -> u32 {
    let n = if cp < 0x80 {
        1
    } else if cp < 0x800 {
        2
    } else if cp < 0x10000 {
        if (0xd800..=0xdfff).contains(&cp) {
            return 0;
        }
        3
    } else if cp > 0x10ffff {
        return 0;
    } else {
        4
    };
    // SAFETY: out couvre n octets, n est 1, 2, 3 ou 4.
    let buf = unsafe { core::slice::from_raw_parts_mut(out.cast::<u8>(), n) };
    if n == 1 {
        buf[0] = cp as u8;
        return 1;
    }
    if n == 2 {
        return enc2(cp, buf);
    }
    if n == 3 {
        return enc3(cp, buf);
    }
    enc4(cp, buf)
}

/// # Safety
/// `s` couvre `pos` octets si `pos` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_u8_prev(s: *const c_char, pos: u32) -> u32 {
    if pos == 0 {
        return 0;
    }
    let mut p = pos - 1;
    while p > 0 && pos - p < 4 {
        // SAFETY: p < pos, donc dans le tampon.
        let b = unsafe { s.add(p as usize).read() } as u8;
        if b & 0xc0 != 0x80 {
            break;
        }
        p -= 1;
    }
    // SAFETY: p < pos et s couvre pos octets.
    let got = u8_len(bytes(unsafe { s.add(p as usize) }, pos - p));
    if p + got == pos {
        return p;
    }
    pos - 1
}
