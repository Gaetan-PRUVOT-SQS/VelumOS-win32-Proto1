use crate::{Font, glyph, utf8};
use core::ffi::c_char;

fn byte_at(text: *const c_char, index: usize) -> u8 {
    // SAFETY: index a été borné par len ou par le NUL déjà lu juste avant.
    unsafe { text.add(index).cast::<u8>().read() }
}

pub(crate) fn end(text: *const c_char, len: i32) -> *const c_char {
    if text.is_null() {
        return core::ptr::null();
    }
    let mut i = 0usize;
    loop {
        if let Ok(limit) = usize::try_from(len)
            && i >= limit
        {
            break;
        }
        if byte_at(text, i) == 0 {
            break;
        }
        let Some(next) = i.checked_add(1) else {
            break;
        };
        i = next;
    }
    // SAFETY: i octets ont été lus, text + i reste dans la chaîne ou sur len.
    unsafe { text.add(i) }
}

fn moved(cur: *const c_char, prev: *const c_char) -> bool {
    cur != prev
}

pub(crate) fn width(f: *const Font, text: *const c_char, len: i32) -> i32 {
    if f.is_null() || text.is_null() {
        return 0;
    }
    let stop = end(text, len);
    let mut cur = text;
    let mut sum = 0i64;
    while cur < stop {
        let prev = cur;
        let cp = utf8::next(&mut cur, stop);
        if !moved(cur, prev) {
            break;
        }
        let g = glyph::glyph(f, cp);
        if g.is_null() {
            continue;
        }
        // SAFETY: g non nul, glyphe de la police f.
        let adv = i64::from(unsafe { (*g).advance });
        sum = match sum.checked_add(adv) {
            Some(v) => v,
            None => return i32::MAX,
        };
    }
    if sum > i64::from(i32::MAX) {
        return i32::MAX;
    }
    i32::try_from(sum).unwrap_or(i32::MAX)
}

fn byte_count(from: *const c_char, to: *const c_char) -> i32 {
    // SAFETY: to est avancé depuis from dans la même chaîne.
    let diff = unsafe { to.offset_from(from) };
    match i32::try_from(diff) {
        Ok(n) => n,
        Err(_) if diff > 0 => i32::MAX,
        Err(_) => 0,
    }
}

pub(crate) fn fit(f: *const Font, text: *const c_char, max_w: i32) -> i32 {
    if f.is_null() || text.is_null() || max_w < 0 {
        return 0;
    }
    let stop = end(text, -1);
    let mut cur = text;
    let mut left = i64::from(max_w);
    while cur < stop {
        let mut next = cur;
        let cp = utf8::next(&mut next, stop);
        if !moved(next, cur) {
            break;
        }
        let g = glyph::glyph(f, cp);
        if !g.is_null() {
            // SAFETY: g non nul, glyphe de la police f.
            let adv = i64::from(unsafe { (*g).advance });
            if adv > left {
                break;
            }
            left -= adv;
        }
        cur = next;
    }
    byte_count(text, cur)
}
