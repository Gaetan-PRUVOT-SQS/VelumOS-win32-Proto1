use crate::{FONT_IDS, FONT_MONO, glyph, table, text, utf8};
use core::ffi::{c_char, c_int};

const E_OK: c_int = 0;
const E_INVAL: c_int = -22;
const REPLACEMENT: u32 = 0xFFFD;

fn tables() -> c_int {
    let mut id = 0;
    while id < FONT_IDS {
        let f = table::get(id);
        id += 1;
        if f.is_null() {
            return E_INVAL;
        }
        // SAFETY: f non nul, police du tableau statique.
        let font = unsafe { &*f };
        let Some(sum) = font.ascent.checked_add(font.descent) else {
            return E_INVAL;
        };
        if font.nglyphs <= 0 || font.height != sum || font.glyphs.is_null() {
            return E_INVAL;
        }
        let mut k = 1i32;
        while k < font.nglyphs {
            let Ok(prev_i) = usize::try_from(k - 1) else {
                return E_INVAL;
            };
            let Ok(cur_i) = usize::try_from(k) else {
                return E_INVAL;
            };
            // SAFETY: prev_i < nglyphs, glyphs non nul.
            let prev = unsafe { (*font.glyphs.add(prev_i)).cp };
            // SAFETY: cur_i < nglyphs, glyphs non nul.
            let cur = unsafe { (*font.glyphs.add(cur_i)).cp };
            if prev >= cur {
                return E_INVAL;
            }
            k += 1;
        }
        let zero = glyph::glyph(f, u32::from(b'0'));
        let missing = glyph::glyph(f, 0x1F);
        if zero.is_null() || missing.is_null() {
            return E_INVAL;
        }
        // SAFETY: zero non nul, glyphe de la table.
        let zero_cp = unsafe { (*zero).cp };
        // SAFETY: missing non nul, glyphe de la table.
        let missing_cp = unsafe { (*missing).cp };
        if zero_cp != u32::from(b'0') || missing_cp != REPLACEMENT {
            return E_INVAL;
        }
    }
    if !table::get(FONT_IDS).is_null() {
        return E_INVAL;
    }
    E_OK
}

fn utf8_sample() -> c_int {
    let buf = *b"\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80\xC0\x80\xE2\x82";
    let mut s = buf.as_ptr().cast::<c_char>();
    // SAFETY: buf est un tableau local de 13 octets, end est sa fin.
    let end = unsafe { s.add(buf.len()) };
    if utf8::next(&mut s, end) != 0xE9 || utf8::next(&mut s, end) != 0x20AC {
        return E_INVAL;
    }
    // SAFETY: end - 4 reste dans buf, après les 9 premiers octets.
    let after_emoji = unsafe { end.sub(4) };
    if utf8::next(&mut s, end) != 0x1F600 || s != after_emoji {
        return E_INVAL;
    }
    if utf8::next(&mut s, end) != REPLACEMENT || utf8::next(&mut s, end) != REPLACEMENT {
        return E_INVAL;
    }
    if utf8::next(&mut s, end) != REPLACEMENT || s != end {
        return E_INVAL;
    }
    if utf8::next(&mut s, end) != 0 || s != end {
        return E_INVAL;
    }
    E_OK
}

fn metrics() -> c_int {
    let mono = table::get(FONT_MONO);
    let dem = *b"D\xC3\xA9marrer\0";
    let text = dem.as_ptr().cast::<c_char>();
    let ab = *b"ab";
    if text::width(mono, text, -1) != 64 {
        return E_INVAL;
    }
    if text::fit(mono, text, 63) != 8 || text::fit(mono, text, 24) != 4 {
        return E_INVAL;
    }
    if text::width(mono, ab.as_ptr().cast(), 1) != 8 || text::width(core::ptr::null(), text, 1) != 0
    {
        return E_INVAL;
    }
    E_OK
}

pub(crate) fn run() -> c_int {
    let mut rc = tables();
    if rc == E_OK {
        rc = utf8_sample();
    }
    if rc == E_OK {
        rc = metrics();
    }
    rc
}
