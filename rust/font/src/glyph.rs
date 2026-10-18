use crate::{Font, Glyph};

const ASCII_FIRST: u32 = 0x20;
const ASCII_LAST: u32 = 0x7E;
const SPACE: u32 = 0x20;
const NBSP: u32 = 0xA0;
const NNBSP: u32 = 0x202F;
const REPLACEMENT: u32 = 0xFFFD;

fn at(glyphs: *const Glyph, index: i32) -> *const Glyph {
    let Ok(i) = usize::try_from(index) else {
        return core::ptr::null();
    };
    // SAFETY: l'appelant a vérifié 0 <= index < nglyphs et glyphs non nul.
    unsafe { glyphs.add(i) }
}

fn cp_at(glyphs: *const Glyph, index: i32) -> u32 {
    // SAFETY: at() vise un glyphe de la table.
    unsafe { (*at(glyphs, index)).cp }
}

fn direct(f: &Font, cp: u32) -> *const Glyph {
    if !(ASCII_FIRST..=ASCII_LAST).contains(&cp) {
        return core::ptr::null();
    }
    let slot = cp - ASCII_FIRST;
    let Ok(n) = u32::try_from(f.nglyphs) else {
        return core::ptr::null();
    };
    if slot >= n {
        return core::ptr::null();
    }
    let Ok(index) = i32::try_from(slot) else {
        return core::ptr::null();
    };
    let g = at(f.glyphs, index);
    if cp_at(f.glyphs, index) != cp {
        return core::ptr::null();
    }
    g
}

fn search(f: &Font, cp: u32) -> *const Glyph {
    if f.glyphs.is_null() || f.nglyphs <= 0 {
        return core::ptr::null();
    }
    let mut low = 0i32;
    let Some(mut high) = f.nglyphs.checked_sub(1) else {
        return core::ptr::null();
    };
    while low <= high {
        let Some(span) = high.checked_sub(low) else {
            return core::ptr::null();
        };
        let Some(mid) = low.checked_add(span / 2) else {
            return core::ptr::null();
        };
        let found = cp_at(f.glyphs, mid);
        if found == cp {
            return at(f.glyphs, mid);
        }
        if found < cp {
            let Some(next) = mid.checked_add(1) else {
                return core::ptr::null();
            };
            low = next;
        } else {
            let Some(prev) = mid.checked_sub(1) else {
                return core::ptr::null();
            };
            high = prev;
        }
    }
    core::ptr::null()
}

fn blank(f: &Font, cp: u32) -> *const Glyph {
    if cp != NBSP && cp != NNBSP {
        return core::ptr::null();
    }
    search(f, SPACE)
}

pub(crate) fn glyph(f: *const Font, cp: u32) -> *const Glyph {
    if f.is_null() {
        return core::ptr::null();
    }
    // SAFETY: f non nul, la police reste vivante le temps de l'appel.
    let font = unsafe { &*f };
    if font.glyphs.is_null() || font.nglyphs <= 0 {
        return core::ptr::null();
    }
    let mut found = direct(font, cp);
    if found.is_null() {
        found = search(font, cp);
    }
    if found.is_null() {
        found = blank(font, cp);
    }
    if found.is_null() {
        found = search(font, REPLACEMENT);
    }
    found
}
