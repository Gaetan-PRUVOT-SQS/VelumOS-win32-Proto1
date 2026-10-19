use core::ffi::c_char;

use crate::abi::NOTIF_CAP_BADGE;

const NS_S: u64 = 1_000_000_000;
const SEC_MIN: u64 = 60;
const SEC_H: u64 = 3_600;
const SEC_D: u64 = 86_400;

fn put_str(out: &mut [u8], mut at: usize, src: &[u8]) -> usize {
    let mut i = 0usize;
    while i < src.len() {
        let Some(next) = at.checked_add(1) else {
            break;
        };
        if next >= out.len() {
            break;
        }
        out[at] = src[i];
        at = next;
        i += 1;
    }
    if at < out.len() {
        out[at] = 0;
    }
    at
}

fn dec(v: u64) -> u8 {
    (v % 10) as u8
}

fn put_num(out: &mut [u8], mut at: usize, mut v: u64) -> usize {
    let mut rev = [0u8; 20];
    let mut n = 0usize;
    rev[n] = b'0' + dec(v);
    n += 1;
    v /= 10;
    while v > 0 && n < rev.len() {
        rev[n] = b'0' + dec(v);
        n += 1;
        v /= 10;
    }
    while n > 0 {
        let Some(next) = at.checked_add(1) else {
            break;
        };
        if next >= out.len() {
            break;
        }
        n -= 1;
        out[at] = rev[n];
        at = next;
    }
    if at < out.len() {
        out[at] = 0;
    }
    at
}

fn put_age(out: &mut [u8], v: u64, unit: &[u8]) -> usize {
    let at = put_str(out, 0, "il y a ".as_bytes());
    let at = put_num(out, at, v);
    let at = put_str(out, at, b" ");
    put_str(out, at, unit)
}

fn buf_mut<'a>(out: *mut c_char, size: usize) -> Option<&'a mut [u8]> {
    if out.is_null() || size == 0 {
        return None;
    }
    Some(unsafe {
        // SAFETY: out n'est pas nul et size octets sont exclusifs en écriture.
        core::slice::from_raw_parts_mut(out.cast::<u8>(), size)
    })
}

/// # Safety
/// `out` est nul, ou bien `size` octets sont exclusifs en écriture. Nul ou `size` nul :
/// retour 0, sans écriture.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_badge_text(count: u32, out: *mut c_char, size: usize) -> usize {
    let Some(buf) = buf_mut(out, size) else {
        return 0;
    };
    buf[0] = 0;
    if count == 0 {
        return 0;
    }
    if count > NOTIF_CAP_BADGE {
        let at = put_num(buf, 0, u64::from(NOTIF_CAP_BADGE));
        return put_str(buf, at, b"+");
    }
    put_num(buf, 0, u64::from(count))
}

/// # Safety
/// `out` est nul, ou bien `size` octets sont exclusifs en écriture. Nul ou `size` nul :
/// retour 0, sans écriture. `now_ns < stamp_ns` ne sous-dépasse pas.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_age_text(
    now_ns: u64,
    stamp_ns: u64,
    out: *mut c_char,
    size: usize,
) -> usize {
    let Some(buf) = buf_mut(out, size) else {
        return 0;
    };
    buf[0] = 0;
    let Some(delta) = now_ns.checked_sub(stamp_ns) else {
        return put_str(buf, 0, "à l'instant".as_bytes());
    };
    let secs = delta / NS_S;
    if secs == 0 {
        return put_str(buf, 0, "à l'instant".as_bytes());
    }
    let (v, unit): (u64, &[u8]) = if secs < SEC_MIN {
        (secs, b"s")
    } else if secs < SEC_H {
        (secs / SEC_MIN, b"min")
    } else if secs < SEC_D {
        (secs / SEC_H, b"h")
    } else {
        (secs / SEC_D, b"j")
    };
    put_age(buf, v, unit)
}
