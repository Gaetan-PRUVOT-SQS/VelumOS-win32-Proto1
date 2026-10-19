#![cfg_attr(not(test), allow(dead_code))]

use crate::abi::{t_color, t_font, t_point, t_rect, t_surface, t_textreq};
use core::ffi::c_char;

#[cfg(not(test))]
pub(crate) fn put(_: *mut t_surface, _: t_point, _: t_color) {}

#[cfg(not(test))]
pub(crate) fn get(_: *const t_surface, _: t_point) -> t_color {
    0
}

#[cfg(not(test))]
pub(crate) fn fill(_: *mut t_surface, _: t_rect, _: t_color) {}

#[cfg(not(test))]
pub(crate) fn font_get(_: i32) -> *const t_font {
    core::ptr::null()
}

#[cfg(not(test))]
pub(crate) fn font_text_width(_: *const t_font, _: *const c_char, _: i32) -> i32 {
    0
}

#[cfg(not(test))]
pub(crate) fn font_fit(_: *const t_font, _: *const c_char, _: i32) -> i32 {
    0
}

#[cfg(not(test))]
pub(crate) fn font_draw(_: *mut t_surface, _: *const t_textreq) {}

#[cfg(test)]
fn cut(a: t_rect, b: t_rect) -> t_rect {
    let mut x0 = i64::from(a.x);
    let mut y0 = i64::from(a.y);
    if i64::from(b.x) > x0 {
        x0 = i64::from(b.x);
    }
    if i64::from(b.y) > y0 {
        y0 = i64::from(b.y);
    }
    let mut x1 = i64::from(a.x) + i64::from(a.w);
    let mut y1 = i64::from(a.y) + i64::from(a.h);
    if i64::from(b.x) + i64::from(b.w) < x1 {
        x1 = i64::from(b.x) + i64::from(b.w);
    }
    if i64::from(b.y) + i64::from(b.h) < y1 {
        y1 = i64::from(b.y) + i64::from(b.h);
    }
    let mut r = t_rect {
        x: x0 as i32,
        y: y0 as i32,
        w: 0,
        h: 0,
    };
    if x1 > x0 && y1 > y0 {
        r.w = (x1 - x0) as i32;
        r.h = (y1 - y0) as i32;
    }
    r
}

#[cfg(test)]
fn visible(s: &t_surface, r: t_rect) -> t_rect {
    let bounds = t_rect {
        x: 0,
        y: 0,
        w: s.w,
        h: s.h,
    };
    cut(cut(r, bounds), s.clip)
}

#[cfg(test)]
fn px(s: &t_surface, x: i32, y: i32) -> *mut u32 {
    let i = (y as usize)
        .wrapping_mul(s.stride as usize)
        .wrapping_add(x as usize);
    s.px.wrapping_add(i)
}

#[cfg(test)]
pub(crate) fn put(s: *mut t_surface, p: t_point, c: t_color) {
    let one = t_rect {
        x: p.x,
        y: p.y,
        w: 1,
        h: 1,
    };
    // SAFETY: le test fournit une surface vivante, comme fake_gfx.c.
    let sr = unsafe { &*s };
    let v = visible(sr, one);
    if v.w == 1 && v.h == 1 {
        // SAFETY: v est dans le tampon de la surface de test.
        unsafe { *px(sr, v.x, v.y) = c };
    }
}

#[cfg(test)]
pub(crate) fn get(s: *const t_surface, p: t_point) -> t_color {
    // SAFETY: le test fournit une surface vivante, comme fake_gfx.c.
    let sr = unsafe { &*s };
    if p.x < 0 || p.y < 0 || p.x >= sr.w || p.y >= sr.h {
        return 0;
    }
    // SAFETY: p est dans le tampon.
    unsafe { *px(sr, p.x, p.y) }
}

#[cfg(test)]
pub(crate) fn fill(s: *mut t_surface, r: t_rect, c: t_color) {
    // SAFETY: le test fournit une surface vivante, comme fake_gfx.c.
    let sr = unsafe { &*s };
    let v = visible(sr, r);
    let mut y = v.y;
    while y < v.y + v.h {
        let mut x = v.x;
        while x < v.x + v.w {
            // SAFETY: (x, y) est dans la zone visible du tampon.
            unsafe { *px(sr, x, y) = c };
            x += 1;
        }
        y += 1;
    }
}

#[cfg(test)]
pub(crate) fn font_get(_: i32) -> *const t_font {
    core::ptr::null()
}

#[cfg(test)]
pub(crate) fn font_text_width(_: *const t_font, _: *const c_char, _: i32) -> i32 {
    0
}

#[cfg(test)]
pub(crate) fn font_fit(_: *const t_font, _: *const c_char, _: i32) -> i32 {
    0
}

#[cfg(test)]
pub(crate) fn font_draw(_: *mut t_surface, _: *const t_textreq) {}
