use crate::geom::{PxBox, abs64, box_clip, box_empty, box_make, box_of};
use crate::px::{px_at, span_color, store};
use crate::surface::clipbox;
use crate::{Color, Point, Rect, Surface};

pub(crate) fn fill_box(s: *mut Surface, mut b: PxBox, c: Color) {
    b = box_clip(b, clipbox(s));
    if box_empty(b) || c >> 24 == 0 {
        return;
    }
    let n = (b.x1 - b.x0) as usize;
    let mut y = b.y0;
    while y < b.y1 {
        span_color(px_at(s, b.x0, y), n, c);
        y += 1;
    }
}

pub(crate) fn fill(s: *mut Surface, r: Rect, c: Color) {
    fill_box(s, box_of(r), c);
}

pub(crate) fn hline(s: *mut Surface, p: Point, len: i32, c: Color) {
    fill(
        s,
        Rect {
            x: p.x,
            y: p.y,
            w: len,
            h: 1,
        },
        c,
    );
}

pub(crate) fn vline(s: *mut Surface, p: Point, len: i32, c: Color) {
    fill(
        s,
        Rect {
            x: p.x,
            y: p.y,
            w: 1,
            h: len,
        },
        c,
    );
}

fn frame_side(b: PxBox, side: i32) -> PxBox {
    if side == 0 {
        return box_make(b.x0, b.y0, b.x1, b.y0 + 1);
    }
    if side == 1 && b.y1 - b.y0 > 1 {
        return box_make(b.x0, b.y1 - 1, b.x1, b.y1);
    }
    if side == 2 {
        return box_make(b.x0, b.y0 + 1, b.x0 + 1, b.y1 - 1);
    }
    if side == 3 && b.x1 - b.x0 > 1 {
        return box_make(b.x1 - 1, b.y0 + 1, b.x1, b.y1 - 1);
    }
    box_make(0, 0, 0, 0)
}

pub(crate) fn frame(s: *mut Surface, r: Rect, c: Color) {
    let b = box_of(r);
    if box_empty(b) {
        return;
    }
    let mut side = 0;
    while side < 4 {
        fill_box(s, frame_side(b, side), c);
        side += 1;
    }
}

struct Line {
    maj0: i64,
    min0: i64,
    len: i64,
    dmin: u64,
    sgn: i64,
    steep: bool,
    clip: PxBox,
    i: i64,
    hi: i64,
    q: u64,
    r: u64,
}

fn line_axes(c: &mut Line, a: Point, b: Point) {
    let (maj_a, min_a, maj_b, min_b) = if c.steep {
        (a.y, a.x, b.y, b.x)
    } else {
        (a.x, a.y, b.x, b.y)
    };
    c.maj0 = i64::from(maj_a);
    c.min0 = i64::from(min_a);
    let dmaj = i64::from(maj_b) - c.maj0;
    let dmin = i64::from(min_b) - c.min0;
    c.len = dmaj;
    c.dmin = abs64(dmin) as u64;
    c.sgn = if dmin < 0 { -1 } else { 1 };
}

fn line_setup(c: &mut Line, mut a: Point, mut b: Point) {
    let dx = i64::from(b.x) - i64::from(a.x);
    let dy = i64::from(b.y) - i64::from(a.y);
    c.steep = abs64(dy) > abs64(dx);
    if (c.steep && b.y < a.y) || (!c.steep && b.x < a.x) {
        core::mem::swap(&mut a, &mut b);
    }
    line_axes(c, a, b);
}

fn line_range(c: &mut Line) -> bool {
    let (mlo, mhi) = if c.steep {
        (c.clip.y0, c.clip.y1)
    } else {
        (c.clip.x0, c.clip.x1)
    };
    let lo = crate::geom::max64(0, mlo - c.maj0);
    c.hi = crate::geom::min64(c.len, mhi - 1 - c.maj0);
    c.i = lo;
    c.q = 0;
    c.r = 0;
    if lo > c.hi {
        return false;
    }
    if c.len > 0 {
        let len = c.len as u64;
        let prod = c.dmin.wrapping_mul(lo as u64);
        c.q = prod / len;
        c.r = prod % len;
    }
    true
}

fn line_step(c: &mut Line) {
    c.i += 1;
    c.r = c.r.wrapping_add(c.dmin);
    let len = c.len as u64;
    if c.r >= len {
        c.r = c.r.wrapping_sub(len);
        c.q = c.q.wrapping_add(1);
    }
}

fn line_pixel(s: *mut Surface, c: &Line, col: Color) -> i32 {
    let round = i64::from(c.r.wrapping_mul(2) > c.len as u64);
    let minor = c.min0 + c.sgn * (c.q as i64 + round);
    let (lo, hi) = if c.steep {
        (c.clip.x0, c.clip.x1)
    } else {
        (c.clip.y0, c.clip.y1)
    };
    if (c.sgn > 0 && minor >= hi) || (c.sgn < 0 && minor < lo) {
        return -1;
    }
    if minor < lo || minor >= hi {
        if c.dmin == 0 {
            return -1;
        }
        return 0;
    }
    if c.steep {
        store(px_at(s, minor, c.maj0 + c.i), col);
    } else {
        store(px_at(s, c.maj0 + c.i, minor), col);
    }
    1
}

fn line_walk(s: *mut Surface, c: &mut Line, col: Color) {
    if !line_range(c) {
        return;
    }
    while c.i <= c.hi {
        if line_pixel(s, c, col) < 0 {
            return;
        }
        line_step(c);
    }
}

pub(crate) fn line(s: *mut Surface, a: Point, b: Point, col: Color) {
    if col >> 24 == 0 {
        return;
    }
    let clip = clipbox(s);
    if box_empty(clip) {
        return;
    }
    let mut ln = Line {
        maj0: 0,
        min0: 0,
        len: 0,
        dmin: 0,
        sgn: 1,
        steep: false,
        clip,
        i: 0,
        hi: 0,
        q: 0,
        r: 0,
    };
    line_setup(&mut ln, a, b);
    line_walk(s, &mut ln, col);
}
