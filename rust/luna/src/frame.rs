use crate::abi::{
    FONT_TITLE, FONT_UI_BOLD, HT_BOTTOM, HT_BOTTOMLEFT, HT_BOTTOMRIGHT, HT_CAPTION, HT_CLIENT,
    HT_CLOSE, HT_LEFT, HT_MAXBUTTON, HT_MINBUTTON, HT_NOWHERE, HT_RIGHT, HT_SYSMENU, HT_TOP,
    HT_TOPLEFT, HT_TOPRIGHT, ICON_NONE, LA_LEFT, LG_DIM_MAX, LG_ENABLED, LG_FRAMED,
    LG_FRAMELESS_STYLES, LG_MAX, LG_NBTN, LG_POS_MAX, LG_SHOWN, LG_SIZABLE, LG_SYSMENU, LG_TOOL,
    LUNA_GLASS_WINDOW, WS_CAPTION, WS_MAXBOX, WS_MINBOX, WS_SIZEBOX, WS_SYSMENU, WS_TOOLWINDOW,
    gfx_get, gfx_put, t_color, t_lbox, t_lcb, t_lgeo, t_lglass, t_llayers, t_lline, t_ltext,
    t_lunawin, t_point, t_rect, t_surface,
};
use crate::glass::{lp_glass, lp_glass_make, lp_rmask};
use crate::ink::{lp_fill, lp_flat, lp_mix, lp_visible};
use crate::num::{lp_clamp, lp_in, lp_inset, lp_isect, lp_max, lp_min, lp_ok, lp_pt, lp_rect};
use crate::ops::luna_icon;
use crate::pal::{det_now, met_now, pal_now};
use crate::shape::{lp_box, lp_layers, lp_line};
use crate::text::lp_text;

const GLYPH_REACH: i32 = 16;

fn zrect() -> t_rect {
    lp_rect(0, 0, 0, 0)
}

fn lg_sane(r: t_rect) -> t_rect {
    if r.x < -LG_POS_MAX || r.x > LG_POS_MAX || r.y < -LG_POS_MAX || r.y > LG_POS_MAX {
        return zrect();
    }
    lp_rect(
        r.x,
        r.y,
        lp_clamp(r.w, 0, LG_DIM_MAX),
        lp_clamp(r.h, 0, LG_DIM_MAX),
    )
}

fn lg_flags(w: &t_lunawin) -> u32 {
    if (w.style & WS_CAPTION) == 0 || (w.style & LG_FRAMELESS_STYLES) != 0 {
        return 0;
    }
    let mut f = LG_FRAMED;
    if (w.style & WS_TOOLWINDOW) != 0 {
        f |= LG_TOOL;
    } else if w.maximized {
        f |= LG_MAX;
    }
    if (w.style & WS_SIZEBOX) != 0 && (f & LG_MAX) == 0 {
        f |= LG_SIZABLE;
    }
    if (w.style & WS_SYSMENU) != 0 {
        f |= LG_SYSMENU;
    }
    f
}

fn lg_dims(g: *mut t_lgeo) {
    // SAFETY: g non nul.
    let g = unsafe { &mut *g };
    let m = met_now();
    g.border = 0;
    g.top = 0;
    g.bottom = 0;
    g.cap_end = 0;
    if (g.flags & LG_FRAMED) == 0 {
        return;
    }
    g.cap_end = m.caption_h;
    if (g.flags & LG_TOOL) != 0 {
        g.cap_end = det_now().tool_caption_h;
    }
    if (g.flags & LG_MAX) != 0 {
        g.cap_end = m.caption_h - m.frame_w;
        return;
    }
    g.border = m.frame_w;
    g.top = m.frame_w;
    g.bottom = m.frame_bottom;
}

fn lg_caption(g: *mut t_lgeo) {
    // SAFETY: g non nul.
    let g = unsafe { &mut *g };
    g.caption = zrect();
    g.caption.x = g.outer.x;
    g.caption.y = g.outer.y;
    if (g.flags & LG_FRAMED) == 0 {
        return;
    }
    let nominal = lp_rect(
        g.outer.x + g.border,
        g.outer.y + g.top,
        g.outer.w - 2 * g.border,
        g.cap_end - g.top,
    );
    g.caption = lp_isect(nominal, g.outer);
}

pub(crate) fn lg_btn_ht(i: i32) -> u32 {
    if i == 0 {
        HT_MINBUTTON
    } else if i == 1 {
        HT_MAXBUTTON
    } else {
        HT_CLOSE
    }
}

pub(crate) fn lg_inside(a: t_rect, b: t_rect) -> bool {
    if a.w <= 0 || a.h <= 0 {
        return false;
    }
    if a.x < b.x || a.y < b.y {
        return false;
    }
    if i64::from(a.x) + i64::from(a.w) > i64::from(b.x) + i64::from(b.w) {
        return false;
    }
    i64::from(a.y) + i64::from(a.h) <= i64::from(b.y) + i64::from(b.h)
}

fn lg_btn_flags(w: &t_lunawin, g: &t_lgeo, i: i32) -> u32 {
    let boxes = w.style & (WS_MINBOX | WS_MAXBOX);
    if (w.style & WS_SYSMENU) == 0 {
        return 0;
    }
    if i == 2 {
        return (LG_SHOWN | LG_ENABLED) << i;
    }
    if (g.flags & LG_TOOL) != 0 || boxes == 0 {
        return 0;
    }
    let mut f = LG_SHOWN << i;
    if i == 0 && (w.style & WS_MINBOX) != 0 {
        f |= LG_ENABLED << i;
    }
    if i == 1 && (w.style & WS_MAXBOX) != 0 {
        f |= LG_ENABLED << i;
    }
    f
}

pub(crate) fn lg_buttons(w: *const t_lunawin, g: *mut t_lgeo) {
    // SAFETY: w non nul, comme lg_buttons en C.
    let w = unsafe { &*w };
    // SAFETY: g non nul et distinct de w le temps de l'appel.
    let g = unsafe { &mut *g };
    let d = det_now();
    let m = met_now();
    let mut size = m.btn_w;
    let mut height = m.btn_h;
    let mut top = d.btn_top;
    let mut right = d.btn_right;
    let mut left = g.caption.x;
    if (g.flags & LG_SYSMENU) != 0 && (g.flags & LG_TOOL) == 0 {
        left += d.sysmenu_w;
    }
    if (g.flags & LG_MAX) != 0 {
        top = d.btn_top_max;
        right = d.btn_right_max;
    }
    if (g.flags & LG_TOOL) != 0 {
        size = d.tool_btn;
        height = d.tool_btn;
        top = d.tool_btn_top;
        right = d.tool_btn_right;
    }
    let mut i = 0;
    while i < LG_NBTN {
        let slot = LG_NBTN - 1 - i;
        let r = lp_rect(
            g.outer
                .x
                .wrapping_add(g.outer.w)
                .wrapping_sub(right)
                .wrapping_sub(slot.wrapping_add(1).wrapping_mul(size))
                .wrapping_sub(slot.wrapping_mul(d.btn_gap)),
            g.outer.y.wrapping_add(top),
            size,
            height,
        );
        g.btn[i as usize] = lp_rect(g.outer.x, g.outer.y, 0, 0);
        let f = lg_btn_flags(w, g, i);
        if f != 0 && lg_inside(r, g.caption) && r.x >= left {
            g.btn[i as usize] = r;
            g.flags |= f;
        }
        i += 1;
    }
}

fn zone_sysmenu(g: &mut t_lgeo) {
    let d = det_now();
    let ic = d.caption_icon;
    g.sysmenu = lp_rect(g.outer.x, g.outer.y, 0, 0);
    g.icon = g.sysmenu;
    if (g.flags & LG_SYSMENU) == 0 || (g.flags & LG_TOOL) != 0 {
        return;
    }
    g.sysmenu = lp_isect(
        lp_rect(g.caption.x, g.caption.y, d.sysmenu_w, g.caption.h),
        g.caption,
    );
    g.icon = lp_rect(
        g.caption.x + d.icon_left,
        g.outer.y + (g.cap_end - ic) / 2,
        ic,
        ic,
    );
    if !lg_inside(g.icon, g.sysmenu) {
        g.icon = lp_rect(g.outer.x, g.outer.y, 0, 0);
    }
}

fn zone_title(g: &mut t_lgeo) {
    let pad = det_now().title_pad;
    let mut left = g.caption.x + pad;
    if g.sysmenu.w > 0 {
        left = g.sysmenu.x + g.sysmenu.w + pad;
    }
    let mut right = g.caption.x + g.caption.w - pad;
    let mut i = 0;
    while i < LG_NBTN {
        if (g.flags & (LG_SHOWN << i)) != 0 && g.btn[i as usize].x - pad < right {
            right = g.btn[i as usize].x - pad;
        }
        i += 1;
    }
    g.title = lp_rect(g.outer.x, g.outer.y, 0, 0);
    if right > left && g.caption.h > 0 {
        g.title = lp_rect(left, g.caption.y, right - left, g.caption.h);
    }
}

fn zone_client(g: &mut t_lgeo) {
    g.client = g.outer;
    if (g.flags & LG_FRAMED) == 0 {
        return;
    }
    let raw = lp_rect(
        g.outer.x + g.border,
        g.outer.y + g.cap_end,
        g.outer.w - 2 * g.border,
        g.outer.h - g.cap_end - g.bottom,
    );
    g.client = lp_isect(raw, g.outer);
    if g.client.w <= 0 || g.client.h <= 0 {
        g.client = lp_rect(g.outer.x, g.outer.y, 0, 0);
    }
}

pub(crate) fn lg_zones(g: *mut t_lgeo) {
    // SAFETY: g non nul.
    let g = unsafe { &mut *g };
    zone_sysmenu(g);
    zone_title(g);
    zone_client(g);
}

pub(crate) fn lg_make(w: *const t_lunawin, g: *mut t_lgeo) {
    // SAFETY: g non nul, POD mis à zéro avant remplissage.
    unsafe { g.write(core::mem::zeroed()) };
    // SAFETY: w non nul.
    let wref = unsafe { &*w };
    // SAFETY: g vient d'être initialisé.
    unsafe {
        (*g).outer = lg_sane(wref.outer);
        (*g).flags = lg_flags(wref);
    }
    lg_dims(g);
    lg_caption(g);
    lg_buttons(w, g);
    lg_zones(g);
}

pub(crate) fn luna_window_client(w: *const t_lunawin) -> t_rect {
    if w.is_null() {
        return zrect();
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    g.client
}

pub(crate) fn luna_caption_rect(w: *const t_lunawin) -> t_rect {
    if w.is_null() {
        return zrect();
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    g.caption
}

pub(crate) fn luna_button_rect(w: *const t_lunawin, ht: u32) -> t_rect {
    if w.is_null() {
        return zrect();
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    let mut i = 0;
    while i < LG_NBTN {
        if lg_btn_ht(i) == ht && (g.flags & (LG_SHOWN << i)) != 0 {
            return g.btn[i as usize];
        }
        i += 1;
    }
    if ht == HT_SYSMENU {
        return g.sysmenu;
    }
    lp_rect(g.outer.x, g.outer.y, 0, 0)
}

fn edge_side(a: i32, b: i32, wa: i32, wb: i32) -> i32 {
    if a < wa && b < wb {
        if a < b {
            return -1;
        }
        if a > b {
            return 1;
        }
        return 0;
    }
    if a < wa {
        return -1;
    }
    if b < wb {
        return 1;
    }
    0
}

fn hit_edge(g: &t_lgeo, p: t_point) -> u32 {
    const EDGE: [[u32; 3]; 3] = [
        [HT_TOPLEFT, HT_TOP, HT_TOPRIGHT],
        [HT_LEFT, HT_NOWHERE, HT_RIGHT],
        [HT_BOTTOMLEFT, HT_BOTTOM, HT_BOTTOMRIGHT],
    ];
    let d = det_now();
    let l = p.x - g.outer.x;
    let t = p.y - g.outer.y;
    let mut sx = edge_side(l, g.outer.w - 1 - l, d.resize, d.resize);
    let mut sy = edge_side(t, g.outer.h - 1 - t, d.resize, d.resize);
    if sx != 0 && sy == 0 && lp_min(t, g.outer.h - 1 - t) < d.corner_len {
        sy = edge_side(t, g.outer.h - 1 - t, d.corner_len, d.corner_len);
    } else if sy != 0 && sx == 0 && lp_min(l, g.outer.w - 1 - l) < d.corner_len {
        sx = edge_side(l, g.outer.w - 1 - l, d.corner_len, d.corner_len);
    }
    EDGE[(sy + 1) as usize][(sx + 1) as usize]
}

fn hit_caption(g: &t_lgeo, p: t_point) -> u32 {
    let mut i = 0;
    while i < LG_NBTN {
        if (g.flags & (LG_SHOWN << i)) != 0 && lp_in(g.btn[i as usize], p) {
            if (g.flags & (LG_ENABLED << i)) != 0 {
                return lg_btn_ht(i);
            }
            return HT_CAPTION;
        }
        i += 1;
    }
    if lp_in(g.sysmenu, p) {
        return HT_SYSMENU;
    }
    if lp_in(g.caption, p) {
        return HT_CAPTION;
    }
    HT_NOWHERE
}

pub(crate) fn luna_hit_test(w: *const t_lunawin, p: t_point) -> u32 {
    if w.is_null() {
        return HT_NOWHERE;
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    if !lp_in(g.outer, p) {
        return HT_NOWHERE;
    }
    if (g.flags & LG_FRAMED) == 0 {
        return HT_CLIENT;
    }
    let mut h = HT_NOWHERE;
    if (g.flags & LG_SIZABLE) != 0 {
        h = hit_edge(&g, p);
    }
    if h == HT_NOWHERE {
        h = hit_caption(&g, p);
    }
    if h == HT_NOWHERE && lp_in(g.client, p) {
        h = HT_CLIENT;
    }
    h
}

fn frame_title(s: *mut t_surface, w: &t_lunawin, g: &t_lgeo) {
    if w.title.is_null() || g.title.w <= 0 {
        return;
    }
    let pal = pal_now();
    let t = t_ltext {
        font: if (g.flags & LG_TOOL) != 0 {
            FONT_UI_BOLD
        } else {
            FONT_TITLE
        },
        color: if w.active { pal.ink } else { pal.muted },
        shadow: 0,
        text: w.title,
        box_: g.title,
        align: LA_LEFT,
    };
    lp_text(s, &t);
}

pub(crate) fn lp_frame_radius(g: *const t_lgeo) -> i32 {
    // SAFETY: g non nul.
    let g = unsafe { &*g };
    if (g.flags & LG_MAX) != 0 {
        0
    } else {
        det_now().title_radius
    }
}

pub(crate) fn lp_frame_bands(g: *const t_lgeo, out: *mut t_rect) -> i32 {
    // SAFETY: g et out non nuls. out a deux cases, comme le C.
    let g = unsafe { &*g };
    let rad = lp_frame_radius(g);
    if g.outer.h <= g.cap_end + rad {
        // SAFETY: out non nul, une case, comme le C.
        unsafe { out.write(g.outer) };
        return 1;
    }
    // SAFETY: out non nul, première bande.
    unsafe {
        out.write(lp_rect(g.outer.x, g.outer.y, g.outer.w, g.cap_end));
    }
    if rad == 0 {
        return 1;
    }
    // SAFETY: out non nul, deuxième case allouée par l'appelant.
    unsafe {
        out.add(1).write(lp_rect(
            g.outer.x,
            g.outer.y + g.outer.h - rad,
            g.outer.w,
            rad,
        ));
    }
    2
}

fn band_paint(s: *mut t_surface, b: &mut t_lglass, r: t_rect, active: bool) {
    b.r = r;
    if !active {
        b.spec = (b.spec & 0x00ff_ffff) | ((b.spec >> 25) << 24);
    }
    lp_glass(s, b);
}

pub(crate) fn lp_frame_glass(s: *mut t_surface, g: *const t_lgeo, active: bool) {
    // SAFETY: g non nul.
    let g = unsafe { &*g };
    // SAFETY: t_lglass est un POD C, rempli avant lecture.
    let mut b = unsafe { core::mem::zeroed::<t_lglass>() };
    if !lp_glass_make(&mut b, g.outer, LUNA_GLASS_WINDOW) {
        return;
    }
    let mut r = [zrect(), zrect()];
    let n = lp_frame_bands(g, r.as_mut_ptr());
    let rad = lp_frame_radius(g);
    b.rad_top = rad;
    b.rad_bot = rad;
    if n == 2 {
        b.rad_bot = 0;
        b.rim[2] = 0;
    }
    band_paint(s, &mut b, r[0], active);
    if n < 2 {
        return;
    }
    b.rad_top = 0;
    b.rad_bot = rad;
    b.rim[0] = 0;
    b.rim[2] = pal_now().rim_bottom;
    b.spec = 0;
    band_paint(s, &mut b, r[1], active);
}

pub(crate) fn luna_window_glass_bands(w: *const t_lunawin, out: *mut t_rect) -> i32 {
    if w.is_null() || out.is_null() {
        return 0;
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    if (g.flags & LG_FRAMED) == 0 || g.outer.w <= 0 || g.outer.h <= 0 {
        return 0;
    }
    lp_frame_bands(&g, out)
}

fn cap_span(g: &t_lgeo) -> t_rect {
    let mut u = zrect();
    let mut i = 0;
    while i < LG_NBTN {
        if (g.flags & (LG_SHOWN << i)) != 0 {
            if u.w == 0 {
                u = g.btn[i as usize];
            } else {
                let b = g.btn[i as usize];
                u = lp_rect(u.x, u.y, b.x + b.w - u.x, u.h);
            }
        }
        i += 1;
    }
    u
}

fn capsule_back(s: *mut t_surface, u: t_rect) {
    let mut st = crate::abi::t_lstop { at: 0, c: 0 };
    let mut gr = crate::abi::t_lgrad {
        st: core::ptr::null(),
        n: 0,
        tint: 0,
        tint_t: 0,
    };
    lp_flat(&mut gr, &mut st, pal_now().control);
    let b = t_lbox {
        r: lp_rect(
            u.x - crate::abi::VT_CAPGROUP_PAD,
            u.y - crate::abi::VT_CAPGROUP_PAD,
            u.w + 2 * crate::abi::VT_CAPGROUP_PAD,
            u.h + 2 * crate::abi::VT_CAPGROUP_PAD,
        ),
        rad: crate::abi::VT_R_ITEM,
        ring: core::ptr::null(),
        nring: 0,
        fill: &gr,
    };
    lp_box(s, &b);
    lp_fill(
        s,
        lp_rect(b.r.x + b.rad, b.r.y, b.r.w - 2 * b.rad, 1),
        pal_now().rim_top,
    );
}

fn cap_color(cb: &t_lcb) -> t_color {
    let pal = pal_now();
    if cb.st == crate::abi::LS_DISABLED {
        return pal.dim;
    }
    if cb.kind == HT_CLOSE && cb.st != crate::abi::LS_NORMAL {
        return pal.danger;
    }
    pal.muted
}

fn cap_back(s: *mut t_surface, cb: &t_lcb) {
    if cb.st != crate::abi::LS_HOT && cb.st != crate::abi::LS_PRESSED {
        return;
    }
    let pal = pal_now();
    let col = if cb.st == crate::abi::LS_PRESSED {
        pal.press
    } else {
        pal.hover
    };
    let mut st = crate::abi::t_lstop { at: 0, c: 0 };
    let mut gr = crate::abi::t_lgrad {
        st: core::ptr::null(),
        n: 0,
        tint: 0,
        tint_t: 0,
    };
    lp_flat(&mut gr, &mut st, col);
    let b = t_lbox {
        r: cb.r,
        rad: crate::abi::VT_R_CAPBTN,
        ring: core::ptr::null(),
        nring: 0,
        fill: &gr,
    };
    lp_box(s, &b);
}

fn glyph_box(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
    let mut st = crate::abi::t_lstop { at: 0, c: 0 };
    let mut g = crate::abi::t_lgrad {
        st: core::ptr::null(),
        n: 0,
        tint: 0,
        tint_t: 0,
    };
    lp_flat(&mut g, &mut st, c);
    let l = t_llayers {
        r,
        rad: [rad, rad, rad, rad],
        ring: core::ptr::null(),
        nring: 0,
        fill: &g,
        hole: lp_inset(r, 2),
        skip: 0,
    };
    lp_layers(s, &l);
}

fn glyph_reachable(s: *const t_surface, c: t_point) -> bool {
    // SAFETY: s non nul, surface du cadre.
    let s = unsafe { &*s };
    c.x >= -GLYPH_REACH
        && c.y >= -GLYPH_REACH
        && c.x <= s.w.wrapping_add(GLYPH_REACH)
        && c.y <= s.h.wrapping_add(GLYPH_REACH)
}

fn glyph_close(s: *mut t_surface, c: t_point, col: t_color) {
    if !glyph_reachable(s, c) {
        return;
    }
    let l1 = t_lline {
        a: lp_pt(
            c.x.wrapping_mul(16).wrapping_sub(56),
            c.y.wrapping_mul(16).wrapping_sub(56),
        ),
        b: lp_pt(
            c.x.wrapping_mul(16).wrapping_add(56),
            c.y.wrapping_mul(16).wrapping_add(56),
        ),
        w: 28,
        round: true,
    };
    lp_line(s, &l1, col);
    let l2 = t_lline {
        a: lp_pt(
            c.x.wrapping_mul(16).wrapping_add(56),
            c.y.wrapping_mul(16).wrapping_sub(56),
        ),
        b: lp_pt(
            c.x.wrapping_mul(16).wrapping_sub(56),
            c.y.wrapping_mul(16).wrapping_add(56),
        ),
        w: 28,
        round: true,
    };
    lp_line(s, &l2, col);
}

pub(crate) fn lp_capglyph(s: *mut t_surface, cb: *const t_lcb, col: t_color) {
    // SAFETY: cb non nul.
    let cb = unsafe { &*cb };
    let c = lp_pt(cb.r.x + cb.r.w / 2, cb.r.y + cb.r.h / 2);
    if cb.kind == HT_MINBUTTON {
        lp_fill(s, lp_rect(c.x - 5, c.y, 10, 2), col);
    } else if cb.kind == HT_MAXBUTTON && !cb.maximized {
        glyph_box(s, lp_rect(c.x - 4, c.y - 4, 9, 9), 2, col);
    } else if cb.kind == HT_MAXBUTTON {
        lp_fill(s, lp_rect(c.x - 2, c.y - 4, 6, 2), col);
        lp_fill(s, lp_rect(c.x + 2, c.y - 4, 2, 6), col);
        glyph_box(s, lp_rect(c.x - 4, c.y - 2, 7, 7), 2, col);
    } else {
        glyph_close(s, c, col);
    }
}

fn cap_button(s: *mut t_surface, w: &t_lunawin, g: &t_lgeo, i: i32) {
    let kind = lg_btn_ht(i);
    let mut st = crate::abi::LS_NORMAL;
    if (g.flags & (LG_ENABLED << i)) == 0 {
        st = crate::abi::LS_DISABLED;
    } else if w.pressed == kind {
        st = crate::abi::LS_PRESSED;
    } else if w.hot == kind {
        st = crate::abi::LS_HOT;
    }
    let cb = t_lcb {
        r: g.btn[i as usize],
        kind,
        st,
        active: w.active,
        maximized: w.maximized,
    };
    cap_back(s, &cb);
    lp_capglyph(s, &cb, cap_color(&cb));
}

pub(crate) fn lp_capbuttons(s: *mut t_surface, w: *const t_lunawin, g: *const t_lgeo) {
    // SAFETY: w non nul.
    let w = unsafe { &*w };
    // SAFETY: g non nul.
    let g = unsafe { &*g };
    let mut i = 0;
    while i < LG_NBTN {
        if (g.flags & (LG_SHOWN << i)) != 0 {
            cap_button(s, w, g, i);
        }
        i += 1;
    }
}

pub(crate) fn lp_capsule(s: *mut t_surface, w: *const t_lunawin, g: *const t_lgeo) {
    // SAFETY: g non nul.
    let g = unsafe { &*g };
    let u = cap_span(g);
    if u.w > 0 && (g.flags & LG_TOOL) == 0 {
        capsule_back(s, u);
    }
    lp_capbuttons(s, w, g);
}

pub(crate) fn luna_window_frame(s: *mut t_surface, w: *const t_lunawin) {
    if !lp_ok(s) || w.is_null() {
        return;
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    if (g.flags & LG_FRAMED) == 0 || g.outer.w <= 0 || g.outer.h <= 0 {
        return;
    }
    // SAFETY: w non nul.
    let wref = unsafe { &*w };
    lp_frame_glass(s, &g, wref.active);
    if g.icon.w > 0 && wref.icon != ICON_NONE {
        luna_icon(s, lp_pt(g.icon.x, g.icon.y), wref.icon, g.icon.w);
    }
    frame_title(s, wref, &g);
    lp_capsule(s, w, &g);
}

pub(crate) fn luna_client_band(w: *const t_lunawin) -> i32 {
    if w.is_null() {
        return 0;
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    if (g.flags & LG_FRAMED) == 0 {
        return 0;
    }
    lp_min(lp_frame_radius(&g), g.client.h)
}

fn band_color(src: *const t_surface, p: t_point, cl: t_rect, fb: t_color) -> t_color {
    let u = p.x.wrapping_sub(cl.x);
    let v = p.y.wrapping_sub(cl.y);
    if src.is_null() {
        return fb;
    }
    // SAFETY: src non nul.
    let src = unsafe { &*src };
    if src.px.is_null() || src.stride < src.w || u < 0 || v < 0 || u >= src.w || v >= src.h {
        return fb;
    }
    let i = (v as usize)
        .wrapping_mul(src.stride as usize)
        .wrapping_add(u as usize);
    // SAFETY: u, v sont dans le tampon, stride couvre la largeur.
    unsafe { *src.px.add(i) }
}

fn band_pixel(s: *mut t_surface, m: &t_lglass, p: t_point, c: t_color) {
    let cov = lp_rmask(m, p.x, p.y);
    if cov >= 16 {
        gfx_put(s, p, c | 0xff00_0000);
    } else if cov > 0 {
        gfx_put(
            s,
            p,
            lp_mix(gfx_get(s, p), c, (cov * 255 / 16) as u32) | 0xff00_0000,
        );
    }
}

pub(crate) fn luna_client_band_paint(
    s: *mut t_surface,
    w: *const t_lunawin,
    src: *const t_surface,
    fb: t_color,
) {
    if !lp_ok(s) || w.is_null() {
        return;
    }
    // SAFETY: t_lgeo est un POD C, rempli par lg_make avant toute lecture.
    let mut g = unsafe { core::mem::zeroed() };
    lg_make(w, &mut g);
    // SAFETY: t_lglass est un POD C, rempli avant lecture.
    let mut m = unsafe { core::mem::zeroed::<t_lglass>() };
    m.r = g.outer;
    m.hole = zrect();
    m.rad_top = 0;
    m.rad_bot = lp_frame_radius(&g);
    let v = lp_visible(s, g.client);
    let band = luna_client_band(w);
    let mut y = lp_max(v.y, g.client.y + g.client.h - band) - 1;
    loop {
        y += 1;
        if y >= v.y + v.h {
            break;
        }
        let mut x = v.x - 1;
        loop {
            x += 1;
            if x >= v.x + v.w {
                break;
            }
            band_pixel(
                s,
                &m,
                lp_pt(x, y),
                band_color(src, lp_pt(x, y), g.client, fb),
            );
        }
    }
}

pub(crate) mod ffi {
    use crate::abi::{t_color, t_lcb, t_lgeo, t_lunawin, t_point, t_rect, t_surface};

    c_fn!(lg_btn_ht(i: i32) -> u32 { super::lg_btn_ht(i) });
    c_fn!(lg_inside(a: t_rect, b: t_rect) -> bool { super::lg_inside(a, b) });
    c_fn!(lg_buttons(w: *const t_lunawin, g: *mut t_lgeo) { super::lg_buttons(w, g) });
    c_fn!(lg_zones(g: *mut t_lgeo) { super::lg_zones(g) });
    c_fn!(lg_make(w: *const t_lunawin, g: *mut t_lgeo) { super::lg_make(w, g) });
    c_fn!(luna_window_client(w: *const t_lunawin) -> t_rect { super::luna_window_client(w) });
    c_fn!(luna_caption_rect(w: *const t_lunawin) -> t_rect { super::luna_caption_rect(w) });
    c_fn!(luna_button_rect(w: *const t_lunawin, ht: u32) -> t_rect { super::luna_button_rect(w, ht) });
    c_fn!(luna_hit_test(w: *const t_lunawin, p: t_point) -> u32 { super::luna_hit_test(w, p) });
    c_fn!(lp_frame_radius(g: *const t_lgeo) -> i32 { super::lp_frame_radius(g) });
    c_fn!(lp_frame_bands(g: *const t_lgeo, out: *mut t_rect) -> i32 { super::lp_frame_bands(g, out) });
    c_fn!(lp_frame_glass(s: *mut t_surface, g: *const t_lgeo, active: bool) {
        super::lp_frame_glass(s, g, active)
    });
    c_fn!(luna_window_glass_bands(w: *const t_lunawin, out: *mut t_rect) -> i32 {
        super::luna_window_glass_bands(w, out)
    });
    c_fn!(lp_capglyph(s: *mut t_surface, cb: *const t_lcb, col: t_color) {
        super::lp_capglyph(s, cb, col)
    });
    c_fn!(lp_capbuttons(s: *mut t_surface, w: *const t_lunawin, g: *const t_lgeo) {
        super::lp_capbuttons(s, w, g)
    });
    c_fn!(lp_capsule(s: *mut t_surface, w: *const t_lunawin, g: *const t_lgeo) {
        super::lp_capsule(s, w, g)
    });
    c_fn!(luna_window_frame(s: *mut t_surface, w: *const t_lunawin) { super::luna_window_frame(s, w) });
    c_fn!(luna_client_band(w: *const t_lunawin) -> i32 { super::luna_client_band(w) });
    c_fn!(luna_client_band_paint(
        s: *mut t_surface,
        w: *const t_lunawin,
        src: *const t_surface,
        fb: t_color
    ) {
        super::luna_client_band_paint(s, w, src, fb)
    });
}
