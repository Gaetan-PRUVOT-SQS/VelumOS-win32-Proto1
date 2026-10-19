use crate::abi::{
    gfx_fill, gfx_get, gfx_put, t_color, t_lgrad, t_lstop, t_point, t_rect, t_surface,
};
use crate::num::{lp_isect, lp_rect};

pub(crate) fn lp_mix(a: t_color, b: t_color, mut t: u32) -> t_color {
    if t > 255 {
        t = 255;
    }
    let mut out = 0u32;
    let mut sh = 0u32;
    while sh < 32 {
        let ca = (a >> sh) & 0xff;
        let cb = (b >> sh) & 0xff;
        let v = ca
            .wrapping_mul(255u32.wrapping_sub(t))
            .wrapping_add(cb.wrapping_mul(t))
            .wrapping_add(127)
            / 255;
        out |= (v & 0xff) << sh;
        sh += 8;
    }
    out
}

pub(crate) fn lp_plot(s: *mut t_surface, p: t_point, c: t_color, cov: u32) {
    let a = ((c >> 24).wrapping_mul(cov)) / 16;
    if a == 0 {
        return;
    }
    if a >= 255 {
        gfx_put(s, p, c | 0xff00_0000);
        return;
    }
    let dst = gfx_get(s, p);
    gfx_put(s, p, lp_mix(dst, c, a) | 0xff00_0000);
}

pub(crate) fn lp_visible(s: *const t_surface, r: t_rect) -> t_rect {
    // SAFETY: le C lit s sans test dans lp_visible. L'appelant a déjà lp_ok, ou accepte le même crash.
    let sr = unsafe { &*s };
    let bounds = lp_rect(0, 0, sr.w, sr.h);
    lp_isect(lp_isect(r, bounds), sr.clip)
}

pub(crate) fn lp_fill(s: *mut t_surface, r: t_rect, c: t_color) {
    if r.w <= 0 || r.h <= 0 || (c >> 24) == 0 {
        return;
    }
    if (c >> 24) == 255 {
        gfx_fill(s, r, c);
        return;
    }
    let vis = lp_visible(s, r);
    let mut y = vis.y;
    while y < vis.y.wrapping_add(vis.h) {
        let mut x = vis.x;
        while x < vis.x.wrapping_add(vis.w) {
            lp_plot(s, t_point { x, y }, c, 16);
            x = x.wrapping_add(1);
        }
        y = y.wrapping_add(1);
    }
}

pub(crate) fn lp_flat(g: *mut t_lgrad, st: *mut t_lstop, c: t_color) {
    // SAFETY: g et st non nuls, exclusifs, comme lp_flat en C.
    unsafe {
        (*st).at = 0;
        (*st).c = c;
        (*g).st = st;
        (*g).n = 1;
        (*g).tint = 0;
        (*g).tint_t = 0;
    }
}

fn stop(g: &t_lgrad, i: i32) -> t_lstop {
    // SAFETY: g->st[i] est un arrêt vivant, comme l'indexation C.
    unsafe { g.st.offset(i as isize).read() }
}

fn grad_ratio(g: &t_lgrad, i: i32, pos: i32) -> u32 {
    let span = stop(g, i).at.wrapping_sub(stop(g, i - 1).at);
    if span <= 0 {
        return 255;
    }
    let num = pos.wrapping_sub(stop(g, i - 1).at).wrapping_mul(255);
    (num / span) as u32
}

pub(crate) fn lp_grad_at(g: *const t_lgrad, pos: i32) -> t_color {
    if g.is_null() {
        return 0;
    }
    // SAFETY: g non nul, tableau d'arrêts vivant comme en C.
    let g = unsafe { &*g };
    if g.n < 1 {
        return 0;
    }
    let mut i = 1;
    while i < g.n && pos > stop(g, i).at {
        i += 1;
    }
    let mut c = if g.n == 1 || pos <= stop(g, 0).at {
        stop(g, 0).c
    } else if i >= g.n {
        stop(g, g.n - 1).c
    } else {
        lp_mix(stop(g, i - 1).c, stop(g, i).c, grad_ratio(g, i, pos))
    };
    if g.tint_t != 0 {
        c = lp_mix(c, g.tint, g.tint_t);
    }
    c
}

pub(crate) fn lp_vfill(s: *mut t_surface, r: t_rect, g: *const t_lgrad) {
    let vis = lp_visible(s, r);
    let mut y = vis.y;
    while y < vis.y.wrapping_add(vis.h) {
        lp_fill(
            s,
            lp_rect(vis.x, y, vis.w, 1),
            lp_grad_at(g, y.wrapping_sub(r.y)),
        );
        y = y.wrapping_add(1);
    }
}

pub(crate) fn lp_hfill(s: *mut t_surface, r: t_rect, g: *const t_lgrad) {
    let vis = lp_visible(s, r);
    let mut x = vis.x;
    while x < vis.x.wrapping_add(vis.w) {
        lp_fill(
            s,
            lp_rect(x, vis.y, 1, vis.h),
            lp_grad_at(g, x.wrapping_sub(r.x)),
        );
        x = x.wrapping_add(1);
    }
}

pub(crate) mod ffi {
    use crate::abi::{t_color, t_lgrad, t_lstop, t_point, t_rect, t_surface};

    c_fn!(lp_mix(a: t_color, b: t_color, t: u32) -> t_color { super::lp_mix(a, b, t) });
    c_fn!(lp_plot(s: *mut t_surface, p: t_point, c: t_color, cov: u32) { super::lp_plot(s, p, c, cov) });
    c_fn!(lp_fill(s: *mut t_surface, r: t_rect, c: t_color) { super::lp_fill(s, r, c) });
    c_fn!(lp_flat(g: *mut t_lgrad, st: *mut t_lstop, c: t_color) { super::lp_flat(g, st, c) });
    c_fn!(lp_visible(s: *const t_surface, r: t_rect) -> t_rect { super::lp_visible(s, r) });
    c_fn!(lp_grad_at(g: *const t_lgrad, pos: i32) -> t_color { super::lp_grad_at(g, pos) });
    c_fn!(lp_vfill(s: *mut t_surface, r: t_rect, g: *const t_lgrad) { super::lp_vfill(s, r, g) });
    c_fn!(lp_hfill(s: *mut t_surface, r: t_rect, g: *const t_lgrad) { super::lp_hfill(s, r, g) });
}
