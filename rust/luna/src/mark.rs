use crate::abi::{
    LP_ICON_MAX, LP_LETTER_GAP, LP_LETTERS, t_color, t_lglyph, t_lline, t_lmk, t_point, t_rect,
    t_surface,
};
use crate::num::{lp_clamp, lp_max, lp_ok, lp_pt, lp_pt_ok};
use crate::shape::lp_line;

static G_V: [u8; 7] = [0, 0, 6, 16, 12, 0, 254];
static G_E: [u8; 29] = [
    0, 11, 10, 11, 10, 9, 9, 7, 7, 6, 3, 6, 1, 7, 0, 9, 0, 13, 1, 15, 3, 16, 7, 16, 9, 15, 10, 14,
    254,
];
static G_L: [u8; 5] = [0, 0, 0, 16, 254];
static G_U: [u8; 20] = [
    0, 6, 0, 13, 1, 15, 3, 16, 7, 16, 9, 15, 10, 13, 255, 10, 6, 10, 16, 254,
];
static G_M: [u8; 35] = [
    0, 6, 0, 16, 255, 0, 9, 1, 7, 3, 6, 4, 6, 5, 7, 6, 9, 6, 16, 255, 6, 9, 7, 7, 9, 6, 10, 6, 11,
    7, 12, 9, 12, 16, 254,
];
static G_O: [u8; 27] = [
    4, 0, 8, 0, 11, 2, 12, 5, 12, 11, 11, 14, 8, 16, 4, 16, 1, 14, 0, 11, 0, 5, 1, 2, 4, 0, 254,
];
static G_S: [u8; 33] = [
    12, 3, 10, 1, 8, 0, 4, 0, 1, 2, 0, 4, 1, 6, 4, 8, 8, 8, 11, 10, 12, 12, 11, 14, 8, 16, 4, 16,
    2, 15, 0, 13, 254,
];

fn letters() -> [t_lglyph; LP_LETTERS] {
    [
        t_lglyph {
            pts: G_V.as_ptr(),
            w: 12,
        },
        t_lglyph {
            pts: G_E.as_ptr(),
            w: 10,
        },
        t_lglyph {
            pts: G_L.as_ptr(),
            w: 0,
        },
        t_lglyph {
            pts: G_U.as_ptr(),
            w: 10,
        },
        t_lglyph {
            pts: G_M.as_ptr(),
            w: 12,
        },
        t_lglyph {
            pts: G_O.as_ptr(),
            w: 12,
        },
        t_lglyph {
            pts: G_S.as_ptr(),
            w: 12,
        },
    ]
}

fn mk_span(cells: i32, h: i32) -> i32 {
    cells * h * 55 / 1024
}

pub(crate) fn lp_wordmark_w(h: i32) -> i32 {
    let h = lp_clamp(h, 0, LP_ICON_MAX);
    let g = letters();
    let mut cells = LP_LETTER_GAP * (LP_LETTERS as i32 - 1);
    let mut i = 0;
    while i < LP_LETTERS {
        cells += i32::from(g[i].w);
        i += 1;
    }
    mk_span(cells, h) + 9 * h / 64 + 1
}

fn mk_scale(unit: i32) -> i32 {
    lp_max(unit * 55 / 64 - 2, 1)
}

fn mk_pt(m: &t_lmk, p: *const u8) -> t_point {
    let scale = mk_scale(m.unit);
    // SAFETY: p vise une paire du contour, pas la sentinelle 254.
    let (a, b) = unsafe { (*p, *p.add(1)) };
    lp_pt(
        m.org.x + i32::from(a) * scale,
        m.org.y + i32::from(b) * scale,
    )
}

fn letter_draw(s: *mut t_surface, g: &t_lglyph, m: &t_lmk) {
    let mut ln = t_lline {
        a: lp_pt(0, 0),
        b: lp_pt(0, 0),
        w: 9 * m.unit / 4,
        round: true,
    };
    let mut have = false;
    let mut p = g.pts;
    loop {
        // SAFETY: le contour se termine par 254, lu avant tout décalage.
        let b0 = unsafe { *p };
        if b0 == 254 {
            break;
        }
        if b0 == 255 {
            have = false;
        } else {
            ln.b = mk_pt(m, p);
            if have {
                lp_line(s, &ln, m.c);
            }
            ln.a = ln.b;
            have = true;
        }
        let step = 2 - usize::from(b0 == 255);
        // SAFETY: le pas 1 ou 2 reste dans le tableau, qui finit par 254.
        p = unsafe { p.add(step) };
    }
}

pub(crate) fn lp_wordmark(s: *mut t_surface, r: t_rect, c: t_color) {
    if !lp_ok(s) || r.h < 4 || r.h > LP_ICON_MAX || !lp_pt_ok(lp_pt(r.x, r.y)) {
        return;
    }
    let g = letters();
    let mut m = t_lmk {
        unit: r.h,
        c,
        org: lp_pt(r.x * 16 + 9 * r.h / 8 + 16, r.y * 16 + 9 * r.h / 8 + 16),
    };
    let mut i = 0;
    while i < LP_LETTERS {
        letter_draw(s, &g[i], &m);
        m.org.x += (i32::from(g[i].w) + LP_LETTER_GAP) * r.h * 55 / 64;
        i += 1;
    }
}

pub(crate) mod ffi {
    use crate::abi::{t_color, t_rect, t_surface};

    c_fn!(lp_wordmark_w(h: i32) -> i32 { super::lp_wordmark_w(h) });
    c_fn!(lp_wordmark(s: *mut t_surface, r: t_rect, c: t_color) { super::lp_wordmark(s, r, c) });
}
