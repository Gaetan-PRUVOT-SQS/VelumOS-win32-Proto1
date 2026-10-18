use crate::geom::{box_clip, box_empty, box_of, max64, min64};
use crate::px::{px_at, span_color, store};
use crate::surface::clipbox;
use crate::{Color, Rect, Rrect, Surface};

const RADIUS_MAX: i64 = 1_048_576;
const COVER_SIDE: i64 = 8;

struct RrCtx {
    s: *mut Surface,
    r: crate::geom::PxBox,
    clip: crate::geom::PxBox,
    color: Color,
    y: i64,
    rad: [i64; 4],
}

fn rr_radius(radius: i32, r: Rect) -> i64 {
    let cap = min64(min64(i64::from(r.w), i64::from(r.h)) / 2, RADIUS_MAX);
    max64(0, min64(i64::from(radius), cap))
}

fn scale_alpha(color: Color, cover: u32) -> Color {
    let mut t = (color >> 24).wrapping_mul(cover).wrapping_add(128);
    t = t.wrapping_add(t >> 8) >> 8;
    (color & 0x00ff_ffff) | (t << 24)
}

fn sqsum(a: i64, b: i64) -> i128 {
    i128::from(a) * i128::from(a) + i128::from(b) * i128::from(b)
}

fn hard_cover(radius: i64, u: i64, v: i64) -> u32 {
    let dx = 2 * (radius - u) - 1;
    let dy = 2 * (radius - v) - 1;
    let limit = 4 * i128::from(radius) * i128::from(radius);
    if sqsum(dx, dy) <= limit { 255 } else { 0 }
}

fn sample_inside(radius: i64, u: i64, v: i64) -> u32 {
    let mut n = 0u32;
    let mut k = 0i64;
    let limit = 4
        * i128::from(COVER_SIDE)
        * i128::from(COVER_SIDE)
        * i128::from(radius)
        * i128::from(radius);
    while k < COVER_SIDE {
        let dx = 2 * COVER_SIDE * (u - radius) + 2 * k + 1;
        let mut l = 0i64;
        while l < COVER_SIDE {
            let dy = 2 * COVER_SIDE * (v - radius) + 2 * l + 1;
            if sqsum(dx, dy) <= limit {
                n += 1;
            }
            l += 1;
        }
        k += 1;
    }
    n
}

fn corner_cover(radius: i64, u: i64, v: i64) -> u32 {
    if radius <= 3 {
        return hard_cover(radius, u, v);
    }
    let r2 = i128::from(radius) * i128::from(radius);
    if sqsum(radius - u, radius - v) <= r2 {
        return 255;
    }
    if sqsum(radius - u - 1, radius - v - 1) >= r2 {
        return 0;
    }
    let n = sample_inside(radius, u, v);
    let denom = (COVER_SIDE * COVER_SIDE) as u32;
    (n * 255 + denom / 2) / denom
}

fn rr_x(c: &RrCtx, side: i32, u: i64) -> i64 {
    if side == 0 {
        c.r.x0 + u
    } else {
        c.r.x1 - 1 - u
    }
}

fn rr_paint(c: &RrCtx, side: i32, rad: i64, v: i64) {
    let (mut u, end) = if side == 1 {
        (max64(0, c.r.x1 - c.clip.x1), min64(rad, c.r.x1 - c.clip.x0))
    } else {
        (max64(0, c.clip.x0 - c.r.x0), min64(rad, c.clip.x1 - c.r.x0))
    };
    while u < end {
        let x = rr_x(c, side, u);
        store(
            px_at(c.s, x, c.y),
            scale_alpha(c.color, corner_cover(rad, u, v)),
        );
        u += 1;
    }
}

fn rr_side(c: &RrCtx, side: i32) -> i64 {
    let top = c.y - c.r.y0;
    let bottom = c.r.y1 - 1 - c.y;
    let idx = side as usize;
    if top < c.rad[idx] {
        rr_paint(c, side, c.rad[idx], top);
        return c.rad[idx];
    }
    let bidx = (3 - side) as usize;
    if bottom < c.rad[bidx] {
        rr_paint(c, side, c.rad[bidx], bottom);
        return c.rad[bidx];
    }
    0
}

fn rr_row(c: &RrCtx) {
    let left = rr_side(c, 0);
    let right = rr_side(c, 1);
    let x0 = max64(c.clip.x0, c.r.x0 + left);
    let x1 = min64(c.clip.x1, c.r.x1 - right);
    if x1 > x0 {
        span_color(px_at(c.s, x0, c.y), (x1 - x0) as usize, c.color);
    }
}

pub(crate) fn fill(s: *mut Surface, rr: *const Rrect) {
    if rr.is_null() {
        return;
    }
    // SAFETY: rr non nul, le rectangle arrondi reste valide le temps de l'appel.
    let rr = unsafe { &*rr };
    if rr.color >> 24 == 0 {
        return;
    }
    let shape = box_of(rr.r);
    let mut c = RrCtx {
        s,
        r: shape,
        clip: box_clip(shape, clipbox(s)),
        color: rr.color,
        y: 0,
        rad: [
            rr_radius(rr.tl, rr.r),
            rr_radius(rr.tr, rr.r),
            rr_radius(rr.br, rr.r),
            rr_radius(rr.bl, rr.r),
        ],
    };
    if box_empty(c.clip) {
        return;
    }
    c.y = c.clip.y0;
    while c.y < c.clip.y1 {
        rr_row(&c);
        c.y += 1;
    }
}
