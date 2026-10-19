use crate::abi::{
    ICON_COMPUTER, ICON_DOCUMENT, ICON_DRIVE, ICON_FOLDER, ICON_HELP, ICON_IDS, ICON_LOGO,
    ICON_NONE, ICON_POWER, ICON_PROGRAM, ICON_RECYCLE, ICON_RUN, ICON_SEARCH, ICON_SETTINGS,
    ICON_USER, LOP_DISC, LOP_LINE, LOP_POLY, LOP_RECT, LOP_RING, LOP_RRECT, LP_ICON_MAX,
    LP_OPS_ARGS, LP_PMAX, t_ldisc, t_lg2, t_lgrad, t_liconent, t_lline, t_lop, t_loplist, t_lpoly,
    t_lrr, t_lscale, t_lstop, t_point, t_rect, t_surface,
};
use crate::ink::{lp_flat, lp_vfill};
use crate::num::{lp_floor16, lp_max, lp_min, lp_ok, lp_pt, lp_pt_ok, lp_rect, lp_round16};
use crate::shape::{lp_disc, lp_line, lp_poly, lp_poly_box, lp_rr_paint};

pub(crate) const fn op<const N: usize>(kind: u8, n: u8, a: [u8; N], c0: u32, c1: u32) -> t_lop {
    let mut aa = [0u8; LP_OPS_ARGS];
    let mut i = 0;
    while i < N {
        aa[i] = a[i];
        i += 1;
    }
    t_lop {
        kind,
        n,
        a: aa,
        c0,
        c1,
    }
}

fn sc_of(sc: *const t_lscale) -> t_lscale {
    // SAFETY: sc non nul, comme les lp_op_* en C.
    unsafe { *sc }
}

fn term(v: i32, num: i32, den: i32) -> i32 {
    v.wrapping_mul(num).wrapping_mul(16).wrapping_div(den)
}

pub(crate) fn lp_op_x(sc: *const t_lscale, v: i32) -> i32 {
    let sc = sc_of(sc);
    sc.ox.wrapping_mul(16).wrapping_add(term(v, sc.num, sc.den))
}

pub(crate) fn lp_op_y(sc: *const t_lscale, v: i32) -> i32 {
    let sc = sc_of(sc);
    sc.oy.wrapping_mul(16).wrapping_add(term(v, sc.num, sc.den))
}

pub(crate) fn lp_op_len(sc: *const t_lscale, v: i32) -> i32 {
    let sc = sc_of(sc);
    term(v, sc.num, sc.den)
}

pub(crate) fn lp_op_grad(op: *const t_lop, at0: i32, at1: i32, o: *mut t_lg2) {
    // SAFETY: op et o non nuls, o->g.st pointe dans o->st.
    unsafe {
        let op = &*op;
        let o = &mut *o;
        o.st[0] = t_lstop { at: at0, c: op.c0 };
        o.st[1] = t_lstop { at: at1, c: op.c1 };
        o.g.n = 2;
        o.g.tint = 0;
        o.g.tint_t = 0;
        o.g.st = o.st.as_ptr();
    }
}

fn arg(op: &t_lop, i: usize) -> i32 {
    i32::from(op.a[i])
}

fn op_box(sc: &t_lscale, op: &t_lop) -> t_rect {
    let x0 = lp_round16(lp_op_x(sc, arg(op, 0)));
    let y0 = lp_round16(lp_op_y(sc, arg(op, 1)));
    let mut x1 = lp_round16(lp_op_x(sc, arg(op, 2)));
    let mut y1 = lp_round16(lp_op_y(sc, arg(op, 3)));
    x1 = lp_max(x1, x0 + 1);
    y1 = lp_max(y1, y0 + 1);
    lp_rect(x0, y0, x1 - x0, y1 - y0)
}

pub(crate) fn lp_op_rect(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) {
    let sc = sc_of(sc);
    // SAFETY: op non nul.
    let op = unsafe { &*op };
    let r = op_box(&sc, op);
    let mut o = blank_g2();
    lp_op_grad(op, 0, lp_max(r.h - 1, 1), &mut o);
    lp_vfill(s, r, &o.g);
}

pub(crate) fn lp_op_rrect(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) {
    let sc = sc_of(sc);
    // SAFETY: op non nul.
    let op = unsafe { &*op };
    let mut rr = t_lrr {
        r: op_box(&sc, op),
        rad: [0; 4],
        g: core::ptr::null(),
        y0: 0,
        hole: lp_rect(0, 0, 0, 0),
        vis: lp_rect(0, 0, 0, 0),
    };
    let r = lp_round16(lp_op_len(&sc, arg(op, 4)));
    let mut k = 0;
    while k < 4 {
        if op.n == 0 || ((op.n >> k) & 1) != 0 {
            rr.rad[k] = r;
        }
        k += 1;
    }
    let mut o = blank_g2();
    lp_op_grad(op, 0, lp_max(rr.r.h - 1, 1), &mut o);
    rr.g = &o.g;
    rr.y0 = rr.r.y;
    lp_rr_paint(s, &rr);
}

pub(crate) fn lp_op_disc(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) {
    // SAFETY: op non nul.
    let op = unsafe { &*op };
    let d = t_ldisc {
        cx: lp_op_x(sc, arg(op, 0)),
        cy: lp_op_y(sc, arg(op, 1)),
        r: lp_op_len(sc, arg(op, 2)),
        r_in: 0,
    };
    let mut o = blank_g2();
    lp_op_grad(op, 1, lp_floor16(2 * d.r) + 2, &mut o);
    lp_disc(s, &d, &o.g);
}

pub(crate) fn lp_op_ring(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) {
    // SAFETY: op non nul.
    let op = unsafe { &*op };
    let d = t_ldisc {
        cx: lp_op_x(sc, arg(op, 0)),
        cy: lp_op_y(sc, arg(op, 1)),
        r: lp_op_len(sc, arg(op, 2)),
        r_in: lp_op_len(sc, arg(op, 3)),
    };
    let mut g = t_lgrad {
        st: core::ptr::null(),
        n: 0,
        tint: 0,
        tint_t: 0,
    };
    let mut st = t_lstop { at: 0, c: 0 };
    lp_flat(&mut g, &mut st, op.c0);
    lp_disc(s, &d, &g);
}

pub(crate) fn lp_op_poly(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) {
    // SAFETY: op non nul.
    let op = unsafe { &*op };
    let mut q = t_lpoly {
        p: [t_point { x: 0, y: 0 }; LP_PMAX],
        n: i32::from(op.n),
    };
    let mut i = 0;
    while i < q.n && i < LP_PMAX as i32 {
        q.p[i as usize].x = lp_op_x(sc, arg(op, (2 * i) as usize));
        q.p[i as usize].y = lp_op_y(sc, arg(op, (2 * i + 1) as usize));
        i += 1;
    }
    if q.n < 3 || q.n > LP_PMAX as i32 {
        return;
    }
    let mut o = blank_g2();
    lp_op_grad(op, 1, lp_poly_box(&q).h - 1, &mut o);
    lp_poly(s, &q, &o.g);
}

pub(crate) fn lp_op_line(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) {
    // SAFETY: op non nul.
    let op = unsafe { &*op };
    let l = t_lline {
        a: lp_pt(lp_op_x(sc, arg(op, 0)), lp_op_y(sc, arg(op, 1))),
        b: lp_pt(lp_op_x(sc, arg(op, 2)), lp_op_y(sc, arg(op, 3))),
        w: lp_op_len(sc, arg(op, 4)) / 2,
        round: op.n != 0,
    };
    lp_line(s, &l, op.c0);
}

fn blank_g2() -> t_lg2 {
    t_lg2 {
        g: t_lgrad {
            st: core::ptr::null(),
            n: 0,
            tint: 0,
            tint_t: 0,
        },
        st: [t_lstop { at: 0, c: 0 }, t_lstop { at: 0, c: 0 }],
    }
}

pub(crate) fn lp_ops(s: *mut t_surface, sc: *const t_lscale, ops: *const t_lop, n: i32) {
    if s.is_null() || sc.is_null() || ops.is_null() {
        return;
    }
    if sc_of(sc).den <= 0 {
        return;
    }
    let mut i = 0;
    while i < n {
        // SAFETY: i < n, ops[i] est vivant et aligné.
        let op = unsafe { &*ops.add(i as usize) };
        let kind = op.kind;
        if kind == LOP_RECT {
            lp_op_rect(s, sc, op);
        } else if kind == LOP_RRECT {
            lp_op_rrect(s, sc, op);
        } else if kind == LOP_DISC {
            lp_op_disc(s, sc, op);
        } else if kind == LOP_POLY {
            lp_op_poly(s, sc, op);
        } else if kind == LOP_LINE {
            lp_op_line(s, sc, op);
        } else if kind == LOP_RING {
            lp_op_ring(s, sc, op);
        }
        i += 1;
    }
}

pub(crate) fn lp_ops_flat(s: *mut t_surface, sc: *const t_lscale, l: *const t_loplist) {
    if l.is_null() {
        return;
    }
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    let mut i = 0;
    while !l.ops.is_null() && i < l.n {
        // SAFETY: i < n, ops[i] est vivant.
        let mut one = unsafe { *l.ops.add(i as usize) };
        one.c0 = l.flat;
        one.c1 = l.flat;
        lp_ops(s, sc, &one, 1);
        i += 1;
    }
}

pub(crate) fn lp_ops_list(s: *mut t_surface, sc: *const t_lscale, l: *const t_loplist) {
    if l.is_null() {
        return;
    }
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    if !l.ops.is_null() {
        lp_ops(s, sc, l.ops, l.n);
    }
}

fn icon_find(mut tab: *const t_liconent, id: i32) -> t_loplist {
    let mut l = t_loplist {
        ops: core::ptr::null(),
        n: 0,
        flat: 0,
    };
    // SAFETY: tab est une table terminée par ICON_NONE.
    unsafe {
        while (*tab).id != ICON_NONE && (*tab).id != id {
            tab = tab.add(1);
        }
        if (*tab).id == id {
            l.ops = (*tab).ops;
            l.n = (*tab).n;
        }
    }
    l
}

pub(crate) fn lp_icon_list(id: i32) -> t_loplist {
    let mut l = icon_find(lp_icons_a(), id);
    if l.ops.is_null() {
        l = icon_find(lp_icons_b(), id);
    }
    if l.ops.is_null() {
        l = icon_find(lp_icons_c(), id);
    }
    l
}

pub(crate) fn luna_icon(s: *mut t_surface, p: t_point, id: i32, size: i32) {
    if !lp_ok(s)
        || !lp_pt_ok(p)
        || !(4..=LP_ICON_MAX).contains(&size)
        || id <= ICON_NONE
        || id >= ICON_IDS
    {
        return;
    }
    let l = lp_icon_list(id);
    let sc = t_lscale {
        ox: p.x,
        oy: p.y,
        num: size,
        den: 64,
    };
    lp_ops_list(s, &sc, &l);
}

pub(crate) fn lp_emblem(s: *mut t_surface, r: t_rect) {
    let size = lp_min(r.w, r.h);
    if s.is_null() || size < 4 {
        return;
    }
    let l = lp_icon_list(ICON_LOGO);
    let sc = t_lscale {
        ox: r.x + (r.w - size) / 2,
        oy: r.y + (r.h - size) / 2,
        num: size,
        den: 64,
    };
    lp_ops_list(s, &sc, &l);
}

const fn ent(id: i32, ops: &'static [t_lop]) -> t_liconent {
    t_liconent {
        id,
        ops: ops.as_ptr(),
        n: ops.len() as i32,
    }
}

// SAFETY: ops est nul ou vise une table t_lop immuable. Le partage ne fait que lire ce pointeur.
unsafe impl Sync for t_liconent {}

static G_LOGO: [t_lop; 5] = [
    op(LOP_DISC, 0, [32, 32, 31], 0xff9e_d4ff, 0xff0b_3bb0),
    op(LOP_DISC, 0, [32, 32, 28], 0xff4c_9bff, 0xff0b_4bd0),
    op(LOP_DISC, 0, [32, 20, 16], 0x50ff_ffff, 0x08ff_ffff),
    op(LOP_LINE, 1, [46, 18, 32, 43, 22], 0xffbf_e6ff, 0),
    op(LOP_LINE, 1, [18, 18, 32, 43, 22], 0xffff_ffff, 0),
];
static G_COMPUTER: [t_lop; 6] = [
    op(LOP_RRECT, 0, [6, 6, 58, 46, 5], 0xffed_eadf, 0xffb9_b5a3),
    op(LOP_RRECT, 0, [9, 9, 55, 40, 2], 0xff6e_6a5c, 0xff8c_8878),
    op(LOP_RRECT, 0, [11, 11, 53, 38, 1], 0xff5e_b0ff, 0xff1c_5ed6),
    op(
        LOP_POLY,
        3,
        [11, 11, 40, 11, 11, 30],
        0x50ff_ffff,
        0x10ff_ffff,
    ),
    op(LOP_RECT, 0, [26, 46, 38, 51], 0xffb8_b4a2, 0xff8f_8b79),
    op(LOP_RRECT, 0, [14, 51, 50, 57, 3], 0xffda_d6c8, 0xffa3_9f8d),
];
static G_FOLDER: [t_lop; 5] = [
    op(LOP_RRECT, 3, [5, 12, 27, 24, 3], 0xffe2_a83a, 0xffcc_8f22),
    op(LOP_RRECT, 0, [5, 18, 59, 52, 3], 0xffe8_b546, 0xffd6_9a28),
    op(LOP_RECT, 0, [10, 16, 54, 30], 0xfff6_f8ff, 0xffdd_e3f5),
    op(LOP_RRECT, 0, [5, 26, 59, 53, 3], 0xffff_e488, 0xfff0_b93a),
    op(LOP_RECT, 0, [7, 27, 57, 29], 0x90ff_ffff, 0x30ff_ffff),
];
static G_DOCUMENT: [t_lop; 8] = [
    op(
        LOP_POLY,
        5,
        [13, 4, 41, 4, 53, 16, 53, 60, 13, 60],
        0xff7a_88a8,
        0xff5a_6a90,
    ),
    op(
        LOP_POLY,
        5,
        [14, 5, 40, 5, 52, 17, 52, 59, 14, 59],
        0xffff_ffff,
        0xffe4_eaf8,
    ),
    op(
        LOP_POLY,
        3,
        [40, 5, 40, 17, 52, 17],
        0xffc8_d3ec,
        0xff9e_afd6,
    ),
    op(LOP_RECT, 0, [19, 12, 33, 16], 0xff5a_86e0, 0xff5a_86e0),
    op(LOP_RECT, 0, [19, 24, 47, 26], 0xff8f_a4d6, 0xff8f_a4d6),
    op(LOP_RECT, 0, [19, 31, 47, 33], 0xff8f_a4d6, 0xff8f_a4d6),
    op(LOP_RECT, 0, [19, 38, 47, 40], 0xff8f_a4d6, 0xff8f_a4d6),
    op(LOP_RECT, 0, [19, 45, 39, 47], 0xffb3_c1e4, 0xffb3_c1e4),
];
static G_A: [t_liconent; 5] = [
    ent(ICON_LOGO, &G_LOGO),
    ent(ICON_COMPUTER, &G_COMPUTER),
    ent(ICON_FOLDER, &G_FOLDER),
    ent(ICON_DOCUMENT, &G_DOCUMENT),
    t_liconent {
        id: ICON_NONE,
        ops: core::ptr::null(),
        n: 0,
    },
];

static G_RECYCLE: [t_lop; 9] = [
    op(
        LOP_POLY,
        4,
        [14, 20, 50, 20, 46, 58, 18, 58],
        0xffd5_dce8,
        0xff87_94aa,
    ),
    op(LOP_RRECT, 0, [10, 12, 54, 21, 2], 0xffbc_c6d8, 0xff87_94aa),
    op(LOP_RRECT, 0, [25, 7, 39, 13, 2], 0xff9a_a6bc, 0xff7c_899f),
    op(LOP_RECT, 0, [23, 26, 25, 52], 0x907f_8ca3, 0x907f_8ca3),
    op(LOP_RECT, 0, [31, 26, 33, 52], 0x907f_8ca3, 0x907f_8ca3),
    op(LOP_RECT, 0, [39, 26, 41, 52], 0x907f_8ca3, 0x907f_8ca3),
    op(LOP_LINE, 1, [32, 28, 41, 44, 6], 0xff2f_b344, 0),
    op(LOP_LINE, 1, [41, 44, 23, 44, 6], 0xff2f_b344, 0),
    op(LOP_LINE, 1, [23, 44, 32, 28, 6], 0xff2f_b344, 0),
];
static G_DRIVE: [t_lop; 4] = [
    op(LOP_RRECT, 0, [5, 22, 59, 46, 4], 0xffe4_e8f0, 0xff9a_a3b8),
    op(LOP_RECT, 0, [7, 24, 57, 29], 0xe0ff_ffff, 0x60ff_ffff),
    op(LOP_RECT, 0, [11, 36, 37, 40], 0xff59_637a, 0xff6b_758c),
    op(LOP_RECT, 0, [46, 36, 53, 40], 0xff7b_ea6b, 0xff2f_b82f),
];
static G_SETTINGS: [t_lop; 6] = [
    op(LOP_RECT, 0, [27, 5, 37, 59], 0xff89_96b2, 0xff89_96b2),
    op(LOP_RECT, 0, [5, 27, 59, 37], 0xff89_96b2, 0xff89_96b2),
    op(LOP_LINE, 0, [14, 14, 50, 50, 20], 0xff89_96b2, 0),
    op(LOP_LINE, 0, [50, 14, 14, 50, 20], 0xff89_96b2, 0),
    op(LOP_DISC, 0, [32, 32, 20], 0xffdd_e4f1, 0xff8c_99b4),
    op(LOP_DISC, 0, [32, 32, 8], 0xff3f_4b66, 0xff5b_6784),
];
static G_RUN: [t_lop; 7] = [
    op(LOP_RRECT, 0, [4, 9, 48, 49, 4], 0xff5b_7bc9, 0xff5b_7bc9),
    op(LOP_RRECT, 0, [5, 10, 47, 48, 3], 0xffff_ffff, 0xffe6_ebf8),
    op(LOP_RRECT, 3, [5, 10, 47, 19, 3], 0xff3f_83f4, 0xff1a_5cd8),
    op(LOP_RECT, 0, [10, 25, 34, 27], 0xff93_a5d3, 0xff93_a5d3),
    op(LOP_RECT, 0, [10, 31, 40, 33], 0xff93_a5d3, 0xff93_a5d3),
    op(LOP_RECT, 0, [28, 38, 42, 46], 0xff6f_dc66, 0xff2d_b23a),
    op(
        LOP_POLY,
        3,
        [40, 31, 58, 42, 40, 53],
        0xff7d_e673,
        0xff25_a834,
    ),
];
static G_B: [t_liconent; 5] = [
    ent(ICON_RECYCLE, &G_RECYCLE),
    ent(ICON_DRIVE, &G_DRIVE),
    ent(ICON_SETTINGS, &G_SETTINGS),
    ent(ICON_RUN, &G_RUN),
    t_liconent {
        id: ICON_NONE,
        ops: core::ptr::null(),
        n: 0,
    },
];

static G_SEARCH: [t_lop; 4] = [
    op(LOP_LINE, 1, [40, 40, 56, 56, 14], 0xff8b_5a2b, 0),
    op(LOP_DISC, 0, [26, 26, 12], 0xc0f4_faff, 0xc0a7_d3f7),
    op(LOP_RING, 0, [26, 26, 19, 12], 0xff8e_9cbb, 0),
    op(LOP_DISC, 0, [21, 20, 3], 0xa0ff_ffff, 0xa0ff_ffff),
];
static G_HELP: [t_lop; 9] = [
    op(LOP_DISC, 0, [32, 32, 29], 0xff74_b4ff, 0xff17_52cc),
    op(LOP_RING, 0, [32, 32, 29, 26], 0xffe8_f2ff, 0),
    op(LOP_LINE, 1, [22, 24, 27, 16, 9], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [27, 16, 35, 16, 9], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [35, 16, 41, 22, 9], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [41, 22, 39, 29, 9], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [39, 29, 32, 34, 9], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [32, 34, 32, 39, 9], 0xffff_ffff, 0),
    op(LOP_DISC, 0, [32, 48, 4], 0xffff_ffff, 0xffff_ffff),
];
static G_POWER: [t_lop; 9] = [
    op(LOP_DISC, 0, [32, 32, 29], 0xffff_9a7a, 0xffc0_2b0c),
    op(LOP_RING, 0, [32, 32, 29, 27], 0x80ff_ffff, 0),
    op(LOP_LINE, 1, [40, 23, 46, 34, 8], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [46, 34, 42, 44, 8], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [42, 44, 32, 48, 8], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [32, 48, 22, 44, 8], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [22, 44, 18, 34, 8], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [18, 34, 24, 23, 8], 0xffff_ffff, 0),
    op(LOP_LINE, 1, [32, 16, 32, 33, 8], 0xffff_ffff, 0),
];
static G_USER: [t_lop; 3] = [
    op(LOP_RRECT, 0, [4, 4, 60, 60, 8], 0xff9f_cbff, 0xff3f_7fe6),
    op(LOP_RRECT, 3, [12, 38, 52, 60, 14], 0xffff_ffff, 0xffd6_e2f8),
    op(LOP_DISC, 0, [32, 24, 11], 0xffff_f1dc, 0xfff5_c896),
];
static G_PROGRAM: [t_lop; 7] = [
    op(LOP_RRECT, 0, [4, 8, 60, 56, 5], 0xff1b_3a8f, 0xff1b_3a8f),
    op(LOP_RRECT, 0, [5, 9, 59, 55, 4], 0xff4d_86ee, 0xff26_56c8),
    op(LOP_RECT, 0, [8, 19, 56, 52], 0xffff_ffff, 0xffee_f2fb),
    op(LOP_RRECT, 0, [13, 24, 27, 36, 2], 0xff6f_d26c, 0xff2e_aa3a),
    op(LOP_RECT, 0, [31, 25, 51, 28], 0xff8f_a4d6, 0xff8f_a4d6),
    op(LOP_RECT, 0, [31, 32, 51, 35], 0xff8f_a4d6, 0xff8f_a4d6),
    op(LOP_RECT, 0, [13, 41, 51, 44], 0xffb3_c1e4, 0xffb3_c1e4),
];
static G_C: [t_liconent; 6] = [
    ent(ICON_SEARCH, &G_SEARCH),
    ent(ICON_HELP, &G_HELP),
    ent(ICON_POWER, &G_POWER),
    ent(ICON_USER, &G_USER),
    ent(ICON_PROGRAM, &G_PROGRAM),
    t_liconent {
        id: ICON_NONE,
        ops: core::ptr::null(),
        n: 0,
    },
];

pub(crate) fn lp_icons_a() -> *const t_liconent {
    G_A.as_ptr()
}

pub(crate) fn lp_icons_b() -> *const t_liconent {
    G_B.as_ptr()
}

pub(crate) fn lp_icons_c() -> *const t_liconent {
    G_C.as_ptr()
}

pub(crate) mod ffi {
    use crate::abi::{t_lg2, t_liconent, t_lop, t_loplist, t_lscale, t_point, t_rect, t_surface};

    c_fn!(lp_op_x(sc: *const t_lscale, v: i32) -> i32 { super::lp_op_x(sc, v) });
    c_fn!(lp_op_y(sc: *const t_lscale, v: i32) -> i32 { super::lp_op_y(sc, v) });
    c_fn!(lp_op_len(sc: *const t_lscale, v: i32) -> i32 { super::lp_op_len(sc, v) });
    c_fn!(lp_op_grad(op: *const t_lop, at0: i32, at1: i32, o: *mut t_lg2) { super::lp_op_grad(op, at0, at1, o) });
    c_fn!(lp_ops(s: *mut t_surface, sc: *const t_lscale, ops: *const t_lop, n: i32) {
        super::lp_ops(s, sc, ops, n)
    });
    c_fn!(lp_op_rect(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) { super::lp_op_rect(s, sc, op) });
    c_fn!(lp_op_rrect(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) { super::lp_op_rrect(s, sc, op) });
    c_fn!(lp_ops_flat(s: *mut t_surface, sc: *const t_lscale, l: *const t_loplist) { super::lp_ops_flat(s, sc, l) });
    c_fn!(lp_op_disc(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) { super::lp_op_disc(s, sc, op) });
    c_fn!(lp_op_ring(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) { super::lp_op_ring(s, sc, op) });
    c_fn!(lp_op_poly(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) { super::lp_op_poly(s, sc, op) });
    c_fn!(lp_op_line(s: *mut t_surface, sc: *const t_lscale, op: *const t_lop) { super::lp_op_line(s, sc, op) });
    c_fn!(lp_ops_list(s: *mut t_surface, sc: *const t_lscale, l: *const t_loplist) { super::lp_ops_list(s, sc, l) });
    c_fn!(lp_icon_list(id: i32) -> t_loplist { super::lp_icon_list(id) });
    c_fn!(luna_icon(s: *mut t_surface, p: t_point, id: i32, size: i32) { super::luna_icon(s, p, id, size) });
    c_fn!(lp_emblem(s: *mut t_surface, r: t_rect) { super::lp_emblem(s, r) });
    c_fn!(lp_icons_a() -> *const t_liconent { super::lp_icons_a() });
    c_fn!(lp_icons_b() -> *const t_liconent { super::lp_icons_b() });
    c_fn!(lp_icons_c() -> *const t_liconent { super::lp_icons_c() });
}
