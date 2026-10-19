use crate::abi::{gfx_put, t_color, t_rect, t_surface};
use crate::ink::lp_fill;
use crate::num::{lp_pt, lp_rect};

pub(crate) fn lp_dotted(s: *mut t_surface, r: t_rect, c: t_color) {
    if s.is_null() || r.w <= 0 || r.h <= 0 {
        return;
    }
    let mut i = 0;
    while i < r.w {
        if i % 2 == 0 {
            gfx_put(s, lp_pt(r.x + i, r.y), c);
            gfx_put(s, lp_pt(r.x + i, r.y + r.h - 1), c);
        }
        i += 1;
    }
    i = 0;
    while i < r.h {
        if i % 2 == 0 {
            gfx_put(s, lp_pt(r.x, r.y + i), c);
            gfx_put(s, lp_pt(r.x + r.w - 1, r.y + i), c);
        }
        i += 1;
    }
}

fn arrow_row(r: t_rect, dir: i32, i: i32) -> t_rect {
    let cx = r.x + r.w / 2;
    let cy = r.y + r.h / 2;
    if dir == 0 || dir == 1 {
        let len = if dir == 1 { 7 - 2 * i } else { 1 + 2 * i };
        return lp_rect(cx - len / 2, cy - 2 + i, len, 1);
    }
    let len = if dir == 3 { 7 - 2 * i } else { 1 + 2 * i };
    lp_rect(cx - 2 + i, cy - len / 2, 1, len)
}

pub(crate) fn lp_arrow(s: *mut t_surface, r: t_rect, dir: i32, c: t_color) {
    if s.is_null() {
        return;
    }
    let mut i = 0;
    while i < 4 {
        lp_fill(s, arrow_row(r, dir, i), c);
        i += 1;
    }
}

pub(crate) mod ffi {
    use crate::abi::{t_color, t_rect, t_surface};

    c_fn!(lp_dotted(s: *mut t_surface, r: t_rect, c: t_color) { super::lp_dotted(s, r, c) });
    c_fn!(lp_arrow(s: *mut t_surface, r: t_rect, dir: i32, c: t_color) {
        super::lp_arrow(s, r, dir, c)
    });
}
