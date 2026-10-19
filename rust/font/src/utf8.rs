use core::ffi::c_char;

const REPLACEMENT: u32 = 0xFFFD;

fn lead_need(lead: u8) -> i32 {
    if (0xC2..=0xDF).contains(&lead) {
        return 1;
    }
    if (0xE0..=0xEF).contains(&lead) {
        return 2;
    }
    if (0xF0..=0xF4).contains(&lead) {
        return 3;
    }
    0
}

fn continuation_ok(lead: u8, index: i32, byte: u8) -> bool {
    let mut low = 0x80u8;
    let mut high = 0xBFu8;
    if index == 1 && lead == 0xE0 {
        low = 0xA0;
    }
    if index == 1 && lead == 0xF0 {
        low = 0x90;
    }
    if index == 1 && lead == 0xED {
        high = 0x9F;
    }
    if index == 1 && lead == 0xF4 {
        high = 0x8F;
    }
    byte >= low && byte <= high
}

fn byte_at(p: *const c_char, index: i32) -> u8 {
    let Ok(i) = usize::try_from(index) else {
        return 0;
    };
    // SAFETY: index est dans [0, fin - début).
    unsafe { p.add(i).cast::<u8>().read() }
}

fn valid_tail(p: *const c_char, avail: isize, lead: u8, need: i32) -> i32 {
    let mut n = 0i32;
    while n < need {
        let Some(next) = n.checked_add(1) else {
            break;
        };
        let Ok(step) = isize::try_from(next) else {
            break;
        };
        if step >= avail {
            break;
        }
        if !continuation_ok(lead, next, byte_at(p, next)) {
            break;
        }
        n = next;
    }
    n
}

fn decode_sequence(p: *const c_char, need: i32) -> u32 {
    let shift = u32::try_from(need).unwrap_or(0);
    let mask = u8::try_from(0x3Fu32 >> shift).unwrap_or(0);
    let mut cp = u32::from(byte_at(p, 0) & mask);
    let mut i = 1i32;
    while i <= need {
        cp = (cp << 6) | u32::from(byte_at(p, i) & 0x3F);
        i += 1;
    }
    cp
}

pub(crate) fn next(s: *mut *const c_char, end: *const c_char) -> u32 {
    if s.is_null() {
        return 0;
    }
    // SAFETY: s est non nul. On lit le pointeur courant, sans l'écrire si on s'arrête.
    let cur = unsafe { *s };
    if cur.is_null() || end.is_null() || cur >= end {
        return 0;
    }
    // SAFETY: cur < end, les deux bornes du même intervalle passé par l'appelant.
    let avail = unsafe { end.offset_from(cur) };
    if avail <= 0 {
        return 0;
    }
    let lead = byte_at(cur, 0);
    if lead < 0x80 {
        // SAFETY: avail >= 1, cur + 1 ne dépasse pas end.
        unsafe { *s = cur.add(1) };
        return u32::from(lead);
    }
    let need = lead_need(lead);
    let got = valid_tail(cur, avail, lead, need);
    let step = usize::try_from(1i32 + got).unwrap_or(1);
    // SAFETY: 1 + got <= avail, donc cur + step reste dans [cur, end].
    unsafe { *s = cur.add(step) };
    if need == 0 || got < need {
        return REPLACEMENT;
    }
    decode_sequence(cur, need)
}
