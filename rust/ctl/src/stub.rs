use core::ffi::c_char;

use crate::abi::{
    FONT_INS_BOLD_24, FONT_UI_BOLD, GFX_REGION_MAX, t_font, t_lunametrics, t_point, t_rect,
    t_region, t_surface,
};

const REGION_LIMIT: i32 = 1_073_741_823;

fn box_of(r: t_rect) -> (i64, i64, i64, i64) {
    if r.w <= 0 || r.h <= 0 {
        return (0, 0, 0, 0);
    }
    (
        i64::from(r.x),
        i64::from(r.y),
        i64::from(r.x) + i64::from(r.w),
        i64::from(r.y) + i64::from(r.h),
    )
}

fn box_rect(x0: i64, y0: i64, x1: i64, y1: i64) -> t_rect {
    if x1 <= x0 || y1 <= y0 {
        return t_rect {
            x: 0,
            y: 0,
            w: 0,
            h: 0,
        };
    }
    t_rect {
        x: x0 as i32,
        y: y0 as i32,
        w: (x1 - x0).min(i64::from(i32::MAX)) as i32,
        h: (y1 - y0).min(i64::from(i32::MAX)) as i32,
    }
}

pub(crate) fn rect_intersect(a: t_rect, b: t_rect) -> t_rect {
    let (ax0, ay0, ax1, ay1) = box_of(a);
    let (bx0, by0, bx1, by1) = box_of(b);
    box_rect(ax0.max(bx0), ay0.max(by0), ax1.min(bx1), ay1.min(by1))
}

pub(crate) fn rect_union(a: t_rect, b: t_rect) -> t_rect {
    if (a.w <= 0 || a.h <= 0) && (b.w <= 0 || b.h <= 0) {
        return t_rect {
            x: 0,
            y: 0,
            w: 0,
            h: 0,
        };
    }
    if a.w <= 0 || a.h <= 0 {
        return b;
    }
    if b.w <= 0 || b.h <= 0 {
        return a;
    }
    let (ax0, ay0, ax1, ay1) = box_of(a);
    let (bx0, by0, bx1, by1) = box_of(b);
    box_rect(ax0.min(bx0), ay0.min(by0), ax1.max(bx1), ay1.max(by1))
}

pub(crate) fn rect_contains(r: t_rect, p: t_point) -> bool {
    let (x0, y0, x1, y1) = box_of(r);
    i64::from(p.x) >= x0 && i64::from(p.x) < x1 && i64::from(p.y) >= y0 && i64::from(p.y) < y1
}

pub(crate) fn region_clear(rg: *mut t_region) {
    if rg.is_null() {
        return;
    }
    // SAFETY: rg n'est pas nul. Le test possède la région.
    unsafe { (*rg).n = 0 };
}

pub(crate) fn region_bounds(rg: *const t_region) -> t_rect {
    let mut b = t_rect {
        x: 0,
        y: 0,
        w: 0,
        h: 0,
    };
    if rg.is_null() {
        return b;
    }
    // SAFETY: rg n'est pas nul.
    let rg = unsafe { &*rg };
    let mut i = 0usize;
    let n = rg.n as usize;
    while i < n && i < GFX_REGION_MAX {
        b = rect_union(b, rg.r[i]);
        i += 1;
    }
    b
}

pub(crate) fn region_add(rg: *mut t_region, r: t_rect) {
    if rg.is_null() {
        return;
    }
    // SAFETY: rg n'est pas nul.
    let rg = unsafe { &mut *rg };
    if rg.n > GFX_REGION_MAX as u32 {
        rg.n = GFX_REGION_MAX as u32;
    }
    let limit = t_rect {
        x: -REGION_LIMIT,
        y: -REGION_LIMIT,
        w: REGION_LIMIT.saturating_mul(2),
        h: REGION_LIMIT.saturating_mul(2),
    };
    let r = rect_intersect(r, limit);
    if r.w <= 0 || r.h <= 0 {
        return;
    }
    if rg.n >= GFX_REGION_MAX as u32 {
        rg.r[0] = rect_union(region_bounds(rg), r);
        rg.n = 1;
        return;
    }
    rg.r[rg.n as usize] = r;
    rg.n += 1;
}

pub(crate) fn gfx_set_clip(s: *mut t_surface, r: t_rect) {
    if s.is_null() {
        return;
    }
    // SAFETY: s est une surface du test.
    let s = unsafe { &mut *s };
    s.clip = rect_intersect(
        r,
        t_rect {
            x: 0,
            y: 0,
            w: s.w,
            h: s.h,
        },
    );
}

struct Fonts {
    ui: t_font,
    bold: t_font,
    big: t_font,
}

// SAFETY: glyphes et bits sont nuls. La table ne change pas et n'est pas libérée.
unsafe impl Sync for Fonts {}

static FONTS: Fonts = Fonts {
    ui: t_font {
        ascent: 9,
        descent: 2,
        height: 11,
        nglyphs: 0,
        glyphs: core::ptr::null(),
        bits: core::ptr::null(),
        nbits: 0,
    },
    bold: t_font {
        ascent: 9,
        descent: 2,
        height: 11,
        nglyphs: 1,
        glyphs: core::ptr::null(),
        bits: core::ptr::null(),
        nbits: 0,
    },
    big: t_font {
        ascent: 19,
        descent: 5,
        height: 24,
        nglyphs: 0,
        glyphs: core::ptr::null(),
        bits: core::ptr::null(),
        nbits: 0,
    },
};

pub(crate) fn font_get(id: i32) -> *const t_font {
    if id == FONT_UI_BOLD {
        return &FONTS.bold;
    }
    if id == FONT_INS_BOLD_24 {
        return &FONTS.big;
    }
    &FONTS.ui
}

fn char_w(f: &t_font) -> i32 {
    if f.nglyphs == 1 { 7 } else { 6 }
}

pub(crate) fn font_text_width(f: *const t_font, text: *const c_char, len: i32) -> i32 {
    if f.is_null() || text.is_null() || len <= 0 {
        return 0;
    }
    // SAFETY: f n'est pas nul. text est lu jusqu'au NUL ou à len octets, comme font_text_width.
    let w = char_w(unsafe { &*f });
    let mut i = 0i32;
    let mut n = 0i32;
    while i < len {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let b = unsafe { text.add(i as usize).read() } as u8;
        if b == 0 {
            break;
        }
        if b & 0xc0 != 0x80 {
            n += 1;
        }
        i += 1;
    }
    n * w
}

pub(crate) fn font_fit(f: *const t_font, text: *const c_char, max_w: i32) -> i32 {
    if f.is_null() || text.is_null() {
        return 0;
    }
    // SAFETY: f n'est pas nul.
    let w = char_w(unsafe { &*f });
    let mut i = 0i32;
    let mut n = 0i32;
    loop {
        // SAFETY: text est une chaîne C terminée.
        let b = unsafe { text.add(i as usize).read() } as u8;
        if b == 0 {
            break;
        }
        if b & 0xc0 != 0x80 {
            if (n + 1) * w > max_w {
                break;
            }
            n += 1;
        }
        i += 1;
    }
    i
}

pub(crate) fn luna_metrics(out: *mut t_lunametrics) {
    if out.is_null() {
        return;
    }
    // SAFETY: out est inscriptible.
    unsafe {
        *out = t_lunametrics {
            caption_h: 30,
            frame_w: 4,
            frame_bottom: 4,
            btn_w: 21,
            btn_h: 21,
            taskbar_h: 30,
            start_w: 100,
            tray_w: 80,
            scroll_w: 17,
            menu_item_h: 22,
            icon_small: 16,
            icon_large: 32,
        };
    }
}
