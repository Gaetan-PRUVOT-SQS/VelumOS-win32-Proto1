use crate::abi::{
    LP_PMAX, LP_RMAX, t_color, t_lbox, t_lcring, t_ldisc, t_ledge, t_lgrad, t_llayers, t_lline,
    t_lpe, t_lpoly, t_lrr, t_lspan, t_lstop, t_point, t_rect, t_surface,
};
use crate::ink::{lp_fill, lp_flat, lp_grad_at, lp_plot, lp_visible};
use crate::num::{lp_clamp, lp_floor16, lp_isqrt, lp_max, lp_min, lp_ok, lp_pt, lp_rect};

const CORNER_RMAX: i32 = 16_777_216;

fn corner_in(r8: i64, x: i64, y: i64) -> i32 {
    let dx = r8.wrapping_sub(x);
    let dy = r8.wrapping_sub(y);
    if dx.wrapping_mul(dx).wrapping_add(dy.wrapping_mul(dy)) <= r8.wrapping_mul(r8) {
        1
    } else {
        0
    }
}

pub(crate) fn lp_corner_cov(mut r: i32, mut ix: i32, mut iy: i32) -> i32 {
    r = lp_clamp(r, 0, CORNER_RMAX);
    ix = lp_clamp(ix, -1, r.wrapping_mul(2).wrapping_add(1));
    iy = lp_clamp(iy, -1, r.wrapping_mul(2).wrapping_add(1));
    let x = i64::from(ix).wrapping_mul(8).wrapping_add(1);
    let y = i64::from(iy).wrapping_mul(8).wrapping_add(1);
    let r8 = i64::from(r).wrapping_mul(8);
    let mut n = 0;
    let mut sy = 0;
    while sy < 4 {
        let mut sx = 0;
        while sx < 4 {
            n += corner_in(r8, x + i64::from(sx) * 2, y + i64::from(sy) * 2);
            sx += 1;
        }
        sy += 1;
    }
    n
}

struct Cspan {
    lo: i32,
    hi: i32,
}

fn ring_span(origin: i32, dir: i32, extent: i32, rad: i32) -> Cspan {
    let mut lo = -i64::from(origin);
    let mut hi = i64::from(extent) - 1 - i64::from(origin);
    if dir < 0 {
        lo = i64::from(origin) - i64::from(extent) + 1;
        hi = i64::from(origin);
    }
    if lo < 0 {
        lo = 0;
    }
    if hi > i64::from(rad) - 1 {
        hi = i64::from(rad) - 1;
    }
    let mut sp = Cspan {
        lo: lo as i32,
        hi: hi as i32,
    };
    if lo > hi {
        sp.lo = 0;
        sp.hi = -1;
    }
    sp
}

fn ring_cov(k: &t_lcring, ix: i32, iy: i32) -> i32 {
    let mut cov = lp_corner_cov(k.rad, ix, iy);
    if ix > 0 && iy > 0 {
        cov -= lp_corner_cov(k.rad - 1, ix - 1, iy - 1);
    }
    lp_clamp(cov, 0, 16)
}

pub(crate) fn lp_corner_ring(s: *mut t_surface, k: *const t_lcring) {
    if !lp_ok(s) || k.is_null() {
        return;
    }
    // SAFETY: k non nul.
    let k = unsafe { &*k };
    // SAFETY: lp_ok a validé s.
    let sr = unsafe { &*s };
    let sx = ring_span(k.x, k.dx, sr.w, k.rad);
    let sy = ring_span(k.y, k.dy, sr.h, k.rad);
    let mut iy = sy.lo;
    while iy <= sy.hi {
        let mut ix = sx.lo;
        while ix <= sx.hi {
            lp_plot(
                s,
                lp_pt(k.x + k.dx * ix, k.y + k.dy * iy),
                k.c,
                ring_cov(k, ix, iy) as u32,
            );
            ix += 1;
        }
        iy += 1;
    }
}

pub(crate) fn lp_rr_span(s: *mut t_surface, rr: *const t_lrr, sp: *mut t_lspan) {
    // SAFETY: rr non nul, comme le C.
    let rr = unsafe { &*rr };
    // SAFETY: sp non nul, distinct de rr.
    let sp = unsafe { &mut *sp };
    sp.x0 = lp_max(sp.x0, rr.vis.x);
    sp.x1 = lp_min(sp.x1, rr.vis.x + rr.vis.w);
    if sp.y < rr.hole.y || sp.y >= rr.hole.y + rr.hole.h || rr.hole.w <= 0 {
        lp_fill(s, lp_rect(sp.x0, sp.y, sp.x1 - sp.x0, 1), sp.c);
        return;
    }
    let cut = lp_min(sp.x1, rr.hole.x);
    lp_fill(s, lp_rect(sp.x0, sp.y, cut - sp.x0, 1), sp.c);
    let cut = lp_max(sp.x0, rr.hole.x + rr.hole.w);
    lp_fill(s, lp_rect(cut, sp.y, sp.x1 - cut, 1), sp.c);
}

pub(crate) fn lp_rr_edge(s: *mut t_surface, rr: *const t_lrr, e: *const t_ledge) {
    // SAFETY: rr non nul.
    let rr = unsafe { &*rr };
    // SAFETY: e non nul.
    let e = unsafe { &*e };
    let mut ix = 0;
    while ix < e.rad {
        let px = e.x + e.dir * ix;
        if px >= rr.vis.x && px < rr.vis.x + rr.vis.w {
            lp_plot(
                s,
                lp_pt(px, e.y),
                e.c,
                lp_corner_cov(e.rad, ix, e.iy) as u32,
            );
        }
        ix += 1;
    }
}

fn rr_clamp(rr: &t_lrr) -> t_lrr {
    let mut c = *rr;
    let lim = lp_min(lp_min(rr.r.w, rr.r.h) / 2, LP_RMAX);
    let mut k = 0;
    while k < 4 {
        c.rad[k] = lp_clamp(rr.rad[k], 0, lim);
        k += 1;
    }
    c
}

fn rr_rad(top: i32, bot: i32, dt: i32, db: i32) -> i32 {
    if dt < top {
        return top;
    }
    if db < bot {
        return bot;
    }
    0
}

fn rr_side(s: *mut t_surface, c: &t_lrr, y: i32, right: i32) -> i32 {
    let dt = y - c.r.y;
    let db = c.r.y + c.r.h - 1 - y;
    let e = t_ledge {
        rad: rr_rad(c.rad[right as usize], c.rad[(3 - right) as usize], dt, db),
        x: c.r.x + right * (c.r.w - 1),
        y,
        dir: 1 - 2 * right,
        iy: lp_min(dt, db),
        c: lp_grad_at(c.g, y - c.y0),
    };
    lp_rr_edge(s, c, &e);
    e.rad
}

pub(crate) fn lp_rr_paint(s: *mut t_surface, rr: *const t_lrr) {
    if s.is_null() || rr.is_null() {
        return;
    }
    // SAFETY: rr non nul.
    let rr = unsafe { &*rr };
    if rr.g.is_null() {
        return;
    }
    let mut c = rr_clamp(rr);
    c.vis = lp_visible(s, rr.r);
    let mut sp = t_lspan {
        y: c.vis.y,
        x0: 0,
        x1: 0,
        c: 0,
    };
    while sp.y < c.vis.y + c.vis.h {
        sp.c = lp_grad_at(c.g, sp.y - c.y0);
        sp.x0 = c.r.x + rr_side(s, &c, sp.y, 0);
        sp.x1 = c.r.x + c.r.w - rr_side(s, &c, sp.y, 1);
        lp_rr_span(s, &c, &mut sp);
        sp.y += 1;
    }
}

fn layer_shape(l: &t_llayers, i: i32) -> t_lrr {
    let mut rr = t_lrr {
        r: crate::num::lp_inset(l.r, i),
        rad: [0; 4],
        g: core::ptr::null(),
        y0: 0,
        hole: l.hole,
        vis: t_rect {
            x: 0,
            y: 0,
            w: 0,
            h: 0,
        },
    };
    let mut k = 0;
    while k < 4 {
        rr.rad[k] = lp_max(l.rad[k] - i, 0);
        k += 1;
    }
    rr
}

pub(crate) fn lp_layers(s: *mut t_surface, l: *const t_llayers) {
    if s.is_null() || l.is_null() {
        return;
    }
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    if l.fill.is_null() {
        return;
    }
    let mut i = 0;
    while i <= l.nring {
        let mut rr = layer_shape(l, i);
        if rr.r.w <= 0 || rr.r.h <= 0 {
            return;
        }
        rr.g = l.fill;
        let mut flat = t_lgrad {
            st: core::ptr::null(),
            n: 0,
            tint: 0,
            tint_t: 0,
        };
        let mut one = t_lstop { at: 0, c: 0 };
        if i < l.nring {
            // SAFETY: ring[i] est un t_color, nring le borne comme en C.
            let col = unsafe { *l.ring.offset(i as isize) };
            lp_flat(&mut flat, &mut one, col);
            rr.g = &flat;
        }
        rr.y0 = rr.r.y - l.skip * i32::from(i == l.nring);
        lp_rr_paint(s, &rr);
        i += 1;
    }
}

pub(crate) fn lp_box(s: *mut t_surface, b: *const t_lbox) {
    if b.is_null() {
        return;
    }
    // SAFETY: b non nul.
    let b = unsafe { &*b };
    let l = t_llayers {
        r: b.r,
        rad: [b.rad, b.rad, b.rad, b.rad],
        ring: b.ring,
        nring: b.nring,
        fill: b.fill,
        hole: lp_rect(0, 0, 0, 0),
        skip: 0,
    };
    lp_layers(s, &l);
}

fn edge_cov(r: i32, d2: i64) -> i32 {
    let rm = r.wrapping_sub(8);
    let rp = r.wrapping_add(8);
    let lo = i64::from(rm).wrapping_mul(i64::from(rm));
    let hi = i64::from(rp).wrapping_mul(i64::from(rp));
    if r >= 8 && d2 <= lo {
        return 16;
    }
    if d2 >= hi {
        return 0;
    }
    lp_clamp(r - (lp_isqrt(d2 as u64) as i32) + 8, 0, 16)
}

fn disc_cov(d: &t_ldisc, x: i32, y: i32) -> i32 {
    let dx = i64::from(x.wrapping_sub(d.cx));
    let dy = i64::from(y.wrapping_sub(d.cy));
    let d2 = dx.wrapping_mul(dx).wrapping_add(dy.wrapping_mul(dy));
    let mut cov = edge_cov(d.r, d2);
    if d.r_in > 0 && cov > 0 {
        cov = lp_min(cov, 16 - edge_cov(d.r_in, d2));
    }
    cov
}

pub(crate) fn lp_disc(s: *mut t_surface, d: *const t_ldisc, g: *const t_lgrad) {
    if s.is_null() || d.is_null() || g.is_null() {
        return;
    }
    // SAFETY: d non nul.
    let d = unsafe { &*d };
    if d.r <= 0 {
        return;
    }
    let top = lp_floor16(d.cy - d.r - 8);
    let mut vis = lp_rect(
        lp_floor16(d.cx - d.r - 8),
        top,
        lp_floor16(2 * d.r + 16) + 2,
        lp_floor16(2 * d.r + 16) + 2,
    );
    vis = lp_visible(s, vis);
    let mut p = t_point { x: 0, y: vis.y };
    while p.y < vis.y + vis.h {
        p.x = vis.x;
        while p.x < vis.x + vis.w {
            let cov = disc_cov(d, p.x * 16 + 8, p.y * 16 + 8);
            if cov > 0 {
                lp_plot(s, p, lp_grad_at(g, p.y - top), cov as u32);
            }
            p.x += 1;
        }
        p.y += 1;
    }
}

pub(crate) fn lp_poly_area2(p: *const t_lpoly) -> i64 {
    // SAFETY: p non nul, comme le C.
    let p = unsafe { &*p };
    let mut a = 0i64;
    let mut i = 0;
    let n = p.n;
    if n <= 0 {
        return 0;
    }
    while i < n && (i as usize) < LP_PMAX {
        let j = (i + 1) % n;
        if (j as usize) >= LP_PMAX {
            break;
        }
        a = a.wrapping_add(i64::from(p.p[i as usize].x).wrapping_mul(i64::from(p.p[j as usize].y)));
        a = a.wrapping_sub(i64::from(p.p[j as usize].x).wrapping_mul(i64::from(p.p[i as usize].y)));
        i += 1;
    }
    a
}

pub(crate) fn lp_poly_box(p: *const t_lpoly) -> t_rect {
    // SAFETY: p non nul.
    let p = unsafe { &*p };
    let mut lo_x = p.p[0].x;
    let mut hi_x = lo_x;
    let mut lo_y = p.p[0].y;
    let mut hi_y = lo_y;
    let mut i = 1;
    while i < p.n && (i as usize) < LP_PMAX {
        lo_x = lp_min(lo_x, p.p[i as usize].x);
        hi_x = lp_max(hi_x, p.p[i as usize].x);
        lo_y = lp_min(lo_y, p.p[i as usize].y);
        hi_y = lp_max(hi_y, p.p[i as usize].y);
        i += 1;
    }
    lp_rect(
        lp_floor16(lo_x) - 1,
        lp_floor16(lo_y) - 1,
        lp_floor16(hi_x) - lp_floor16(lo_x) + 3,
        lp_floor16(hi_y) - lp_floor16(lo_y) + 3,
    )
}

fn poly_prep(p: &t_lpoly, e: &mut [t_lpe], sgn: i64) {
    let mut i = 0;
    while i < p.n && (i as usize) < LP_PMAX {
        let j = (i + 1) % p.n;
        if (j as usize) >= LP_PMAX {
            break;
        }
        let dx = i64::from(p.p[j as usize].x.wrapping_sub(p.p[i as usize].x));
        let dy = i64::from(p.p[j as usize].y.wrapping_sub(p.p[i as usize].y));
        let len = i64::from(lp_isqrt(
            dx.wrapping_mul(dx).wrapping_add(dy.wrapping_mul(dy)) as u64,
        ));
        e[i as usize].a = dy.wrapping_neg().wrapping_mul(sgn);
        e[i as usize].b = dx.wrapping_mul(sgn);
        e[i as usize].c = dy
            .wrapping_mul(i64::from(p.p[i as usize].x))
            .wrapping_sub(dx.wrapping_mul(i64::from(p.p[i as usize].y)))
            .wrapping_mul(sgn);
        e[i as usize].inv = 0;
        if len > 0 {
            e[i as usize].inv = 4_294_967_296i64.wrapping_div(len);
        }
        i += 1;
    }
}

fn poly_cov(e: &[t_lpe], n: i32, x: i32, y: i32) -> i32 {
    let mut best = 8i64;
    let mut i = 0;
    while i < n && (i as usize) < e.len() {
        if e[i as usize].inv != 0 {
            let mut dist = e[i as usize]
                .a
                .wrapping_mul(i64::from(x))
                .wrapping_add(e[i as usize].b.wrapping_mul(i64::from(y)))
                .wrapping_add(e[i as usize].c);
            dist = dist.wrapping_mul(e[i as usize].inv) / 4_294_967_296;
            if dist < best {
                best = dist;
            }
            if best <= -8 {
                return 0;
            }
        }
        i += 1;
    }
    (best as i32).wrapping_add(8)
}

pub(crate) fn lp_poly(s: *mut t_surface, poly: *const t_lpoly, g: *const t_lgrad) {
    if s.is_null() || poly.is_null() || g.is_null() {
        return;
    }
    // SAFETY: poly non nul.
    let poly = unsafe { &*poly };
    if poly.n < 3 || poly.n > LP_PMAX as i32 || lp_poly_area2(poly) == 0 {
        return;
    }
    let mut e = [t_lpe {
        a: 0,
        b: 0,
        c: 0,
        inv: 0,
    }; LP_PMAX];
    let sgn = 1 - 2 * i64::from(lp_poly_area2(poly) < 0);
    poly_prep(poly, &mut e, sgn);
    let mut vis = lp_poly_box(poly);
    let top = vis.y;
    vis = lp_visible(s, vis);
    let mut y = vis.y;
    while y < vis.y + vis.h {
        let mut p = t_point { x: vis.x, y };
        while p.x < vis.x + vis.w {
            let cov = poly_cov(&e, poly.n, p.x * 16 + 8, y * 16 + 8);
            if cov > 0 {
                lp_plot(s, p, lp_grad_at(g, y - top), cov as u32);
            }
            p.x += 1;
        }
        y += 1;
    }
}

fn line_quad(l: &t_lline, q: &mut t_lpoly) -> bool {
    let dx = i64::from(l.b.x.wrapping_sub(l.a.x));
    let dy = i64::from(l.b.y.wrapping_sub(l.a.y));
    let len = i64::from(lp_isqrt(
        dx.wrapping_mul(dx).wrapping_add(dy.wrapping_mul(dy)) as u64,
    ));
    if len == 0 {
        return false;
    }
    let nx = ((-dy).wrapping_mul(i64::from(l.w)) / (2 * len)) as i32;
    let ny = (dx.wrapping_mul(i64::from(l.w)) / (2 * len)) as i32;
    q.n = 4;
    q.p[0] = lp_pt(l.a.x + nx, l.a.y + ny);
    q.p[1] = lp_pt(l.b.x + nx, l.b.y + ny);
    q.p[2] = lp_pt(l.b.x - nx, l.b.y - ny);
    q.p[3] = lp_pt(l.a.x - nx, l.a.y - ny);
    true
}

pub(crate) fn lp_line(s: *mut t_surface, l: *const t_lline, c: t_color) {
    if s.is_null() || l.is_null() {
        return;
    }
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    if l.w <= 0 {
        return;
    }
    let mut g = t_lgrad {
        st: core::ptr::null(),
        n: 0,
        tint: 0,
        tint_t: 0,
    };
    let mut st = t_lstop { at: 0, c: 0 };
    lp_flat(&mut g, &mut st, c);
    let mut q = t_lpoly {
        p: [t_point { x: 0, y: 0 }; LP_PMAX],
        n: 0,
    };
    if line_quad(l, &mut q) {
        lp_poly(s, &q, &g);
    }
    if !l.round {
        return;
    }
    let mut d = t_ldisc {
        r: l.w / 2,
        r_in: 0,
        cx: l.a.x,
        cy: l.a.y,
    };
    lp_disc(s, &d, &g);
    d.cx = l.b.x;
    d.cy = l.b.y;
    lp_disc(s, &d, &g);
}

pub(crate) mod ffi {
    use crate::abi::{
        t_color, t_lbox, t_lcring, t_ldisc, t_ledge, t_lgrad, t_llayers, t_lline, t_lpoly, t_lrr,
        t_lspan, t_rect, t_surface,
    };

    c_fn!(lp_corner_cov(r: i32, ix: i32, iy: i32) -> i32 { super::lp_corner_cov(r, ix, iy) });
    c_fn!(lp_corner_ring(s: *mut t_surface, k: *const t_lcring) { super::lp_corner_ring(s, k) });
    c_fn!(lp_rr_span(s: *mut t_surface, rr: *const t_lrr, sp: *mut t_lspan) { super::lp_rr_span(s, rr, sp) });
    c_fn!(lp_rr_edge(s: *mut t_surface, rr: *const t_lrr, e: *const t_ledge) { super::lp_rr_edge(s, rr, e) });
    c_fn!(lp_rr_paint(s: *mut t_surface, rr: *const t_lrr) { super::lp_rr_paint(s, rr) });
    c_fn!(lp_layers(s: *mut t_surface, l: *const t_llayers) { super::lp_layers(s, l) });
    c_fn!(lp_box(s: *mut t_surface, b: *const t_lbox) { super::lp_box(s, b) });
    c_fn!(lp_disc(s: *mut t_surface, d: *const t_ldisc, g: *const t_lgrad) { super::lp_disc(s, d, g) });
    c_fn!(lp_poly_area2(p: *const t_lpoly) -> i64 { super::lp_poly_area2(p) });
    c_fn!(lp_poly_box(p: *const t_lpoly) -> t_rect { super::lp_poly_box(p) });
    c_fn!(lp_poly(s: *mut t_surface, p: *const t_lpoly, g: *const t_lgrad) { super::lp_poly(s, p, g) });
    c_fn!(lp_line(s: *mut t_surface, l: *const t_lline, c: t_color) { super::lp_line(s, l, c) });
}
