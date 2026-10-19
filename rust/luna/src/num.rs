use crate::abi::{LP_COORD_MAX, t_point, t_rect, t_surface};

pub(crate) fn lp_min(a: i32, b: i32) -> i32 {
    if a < b { a } else { b }
}

pub(crate) fn lp_max(a: i32, b: i32) -> i32 {
    if a > b { a } else { b }
}

pub(crate) fn lp_clamp(v: i32, lo: i32, hi: i32) -> i32 {
    if v < lo {
        return lo;
    }
    if v > hi {
        return hi;
    }
    v
}

pub(crate) fn lp_floor16(v: i32) -> i32 {
    let mut q = v.wrapping_div(16);
    if v.wrapping_rem(16) < 0 {
        q = q.wrapping_sub(1);
    }
    q
}

pub(crate) fn lp_round16(v: i32) -> i32 {
    lp_floor16(v.wrapping_add(8))
}

pub(crate) fn lp_isqrt(mut v: u64) -> u32 {
    let mut res = 0u64;
    let mut bit = 1u64 << 62;
    while bit > v {
        bit >>= 2;
    }
    while bit != 0 {
        if v >= res.wrapping_add(bit) {
            v = v.wrapping_sub(res.wrapping_add(bit));
            res = (res >> 1).wrapping_add(bit);
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    res as u32
}

pub(crate) fn lp_pt(x: i32, y: i32) -> t_point {
    t_point { x, y }
}

pub(crate) fn lp_rect(x: i32, y: i32, w: i32, h: i32) -> t_rect {
    t_rect { x, y, w, h }
}

pub(crate) fn lp_in(r: t_rect, p: t_point) -> bool {
    if r.w <= 0 || r.h <= 0 {
        return false;
    }
    if p.x < r.x || p.y < r.y {
        return false;
    }
    if i64::from(p.x) >= i64::from(r.x) + i64::from(r.w) {
        return false;
    }
    i64::from(p.y) < i64::from(r.y) + i64::from(r.h)
}

pub(crate) fn lp_isect(a: t_rect, b: t_rect) -> t_rect {
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
    let out = lp_rect(a.x, a.y, 0, 0);
    if a.w <= 0 || a.h <= 0 || b.w <= 0 || b.h <= 0 || x1 <= x0 || y1 <= y0 {
        return out;
    }
    lp_rect(x0 as i32, y0 as i32, (x1 - x0) as i32, (y1 - y0) as i32)
}

pub(crate) fn lp_inset(r: t_rect, d: i32) -> t_rect {
    let w = lp_max(r.w.wrapping_sub(d.wrapping_mul(2)), 0);
    let h = lp_max(r.h.wrapping_sub(d.wrapping_mul(2)), 0);
    lp_rect(r.x.wrapping_add(d), r.y.wrapping_add(d), w, h)
}

pub(crate) fn lp_ok(s: *const t_surface) -> bool {
    if s.is_null() {
        return false;
    }
    // SAFETY: s non nul. Lecture des champs, comme lp_ok en C.
    let s = unsafe { &*s };
    if s.px.is_null() || s.w <= 0 || s.h <= 0 {
        return false;
    }
    if s.w > LP_COORD_MAX || s.h > LP_COORD_MAX {
        return false;
    }
    s.stride >= s.w
}

pub(crate) fn lp_pt_ok(p: t_point) -> bool {
    p.x >= -LP_COORD_MAX && p.x <= LP_COORD_MAX && p.y >= -LP_COORD_MAX && p.y <= LP_COORD_MAX
}

pub(crate) fn lp_clean(r: t_rect) -> t_rect {
    if r.x < -LP_COORD_MAX
        || r.x > LP_COORD_MAX
        || r.y < -LP_COORD_MAX
        || r.y > LP_COORD_MAX
        || r.w <= 0
        || r.h <= 0
    {
        return lp_rect(0, 0, 0, 0);
    }
    lp_rect(
        r.x,
        r.y,
        lp_min(r.w, LP_COORD_MAX),
        lp_min(r.h, LP_COORD_MAX),
    )
}

pub(crate) mod ffi {
    use crate::abi::{t_point, t_rect, t_surface};

    c_fn!(lp_min(a: i32, b: i32) -> i32 { super::lp_min(a, b) });
    c_fn!(lp_max(a: i32, b: i32) -> i32 { super::lp_max(a, b) });
    c_fn!(lp_clamp(v: i32, lo: i32, hi: i32) -> i32 { super::lp_clamp(v, lo, hi) });
    c_fn!(lp_floor16(v: i32) -> i32 { super::lp_floor16(v) });
    c_fn!(lp_round16(v: i32) -> i32 { super::lp_round16(v) });
    c_fn!(lp_isqrt(v: u64) -> u32 { super::lp_isqrt(v) });
    c_fn!(lp_pt(x: i32, y: i32) -> t_point { super::lp_pt(x, y) });
    c_fn!(lp_rect(x: i32, y: i32, w: i32, h: i32) -> t_rect { super::lp_rect(x, y, w, h) });
    c_fn!(lp_in(r: t_rect, p: t_point) -> bool { super::lp_in(r, p) });
    c_fn!(lp_isect(a: t_rect, b: t_rect) -> t_rect { super::lp_isect(a, b) });
    c_fn!(lp_inset(r: t_rect, d: i32) -> t_rect { super::lp_inset(r, d) });
    c_fn!(lp_ok(s: *const t_surface) -> bool { super::lp_ok(s) });
    c_fn!(lp_pt_ok(p: t_point) -> bool { super::lp_pt_ok(p) });
    c_fn!(lp_clean(r: t_rect) -> t_rect { super::lp_clean(r) });
}
