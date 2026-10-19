use crate::abi::{
    LC_BLACK, LC_WHITE, LCT_FOCUS_GAP, LCT_FOCUS_W, LCT_RAD_CTRL, LCT_RAD_FIELD, LCT_RAD_GROUP,
    LCT_RAD_ITEM, LCT_RAD_MENU, LCT_RIM_PRIMARY, LOP_LINE, LP_COORD_MAX, LP_RMAX, LS_DEFAULT,
    LS_DISABLED, LS_FOCUSED, LS_HOT, LS_NORMAL, LS_PRESSED, VT_ACCENT, VT_ACCENT_HI,
    VT_RADIO_BORDER, VT_RADIO_BORDER_D, t_color, t_lchoice, t_lop, t_lrfill, t_lscale, t_lunabtn,
    t_lunascroll, t_lunastate, t_point, t_rect, t_scrollparts, t_surface,
};
use crate::decor::lp_arrow;
use crate::ink::{lp_plot, lp_visible};
use crate::num::{
    lp_clamp, lp_clean, lp_in, lp_inset, lp_isect, lp_max, lp_min, lp_ok, lp_pt, lp_pt_ok, lp_rect,
};
use crate::ops::{lp_ops, op};
use crate::pal::{
    det_now, lp_dark, lp_tone_alpha, lp_tone_hover, lp_tone_press, lp_tone_track, met_now, pal_now,
};
use crate::shape::lp_corner_cov;

const RADIO_SUB: i32 = 4;
const RADIO_FINE: i32 = 12;
const RADIO_THICK: i32 = 32;

static G_TICK: [t_lop; 2] = [
    op(LOP_LINE, 1, [7, 15, 11, 20, 5], LC_WHITE, LC_WHITE),
    op(LOP_LINE, 1, [11, 20, 20, 7, 5], LC_WHITE, LC_WHITE),
];

pub(crate) fn lp_btn_primary(b: *const t_lunabtn) -> bool {
    // SAFETY: b non nul, comme lp_btn_primary en C.
    let b = unsafe { &*b };
    if b.state == LS_DISABLED {
        return false;
    }
    b.is_default || b.state == LS_DEFAULT
}

pub(crate) fn luna_button_ink(b: *const t_lunabtn) -> t_color {
    let pal = pal_now();
    if b.is_null() {
        return pal.ink;
    }
    // SAFETY: b non nul.
    let b = unsafe { &*b };
    if b.state == LS_DISABLED {
        return pal.dim;
    }
    if lp_btn_primary(b) {
        return LC_WHITE;
    }
    pal.ink
}

pub(crate) fn lp_rmake(r: t_rect, rad: i32, top: t_color, bottom: t_color) -> t_lrfill {
    let r = lp_clean(r);
    let cap = lp_max(lp_min(r.w, r.h) / 2, 0);
    t_lrfill {
        r,
        rad: lp_clamp(rad, 0, cap),
        top,
        bottom,
    }
}

pub(crate) fn lp_rcov(f: *const t_lrfill, p: t_point) -> i32 {
    // SAFETY: f non nul, comme lp_rcov en C.
    let f = unsafe { &*f };
    if !lp_in(f.r, p) {
        return 0;
    }
    let ix = lp_min(p.x - f.r.x, f.r.x + f.r.w - 1 - p.x);
    let iy = lp_min(p.y - f.r.y, f.r.y + f.r.h - 1 - p.y);
    if ix >= f.rad || iy >= f.rad {
        return 16;
    }
    lp_corner_cov(f.rad, ix, iy)
}

pub(crate) fn lp_rfill(s: *mut t_surface, f: *const t_lrfill) {
    if !lp_ok(s) || f.is_null() {
        return;
    }
    // SAFETY: f non nul.
    let f = unsafe { &*f };
    let vis = lp_visible(s, f.r);
    let mut y = vis.y;
    while y < vis.y + vis.h {
        let num = i64::from(y.wrapping_sub(f.r.y)).wrapping_mul(255);
        let den = i64::from(lp_max(f.r.h - 1, 1));
        let c = crate::ink::lp_mix(f.top, f.bottom, (num / den) as u32);
        let mut x = vis.x;
        while x < vis.x + vis.w {
            let p = lp_pt(x, y);
            lp_plot(s, p, c, lp_rcov(f, p) as u32);
            x += 1;
        }
        y += 1;
    }
}

fn ring_row(s: *mut t_surface, f: &t_lrfill, inn: &t_lrfill, y: i32) {
    let vis = lp_visible(s, f.r);
    let mut x = vis.x;
    while x < vis.x + vis.w {
        let p = lp_pt(x, y);
        let cov = lp_rcov(f, p) - lp_rcov(inn, p);
        if cov > 0 {
            lp_plot(s, p, f.top, cov as u32);
        }
        x += 1;
    }
}

pub(crate) fn lp_rring(s: *mut t_surface, f: *const t_lrfill, mut w: i32) {
    if !lp_ok(s) || f.is_null() || w <= 0 {
        return;
    }
    // SAFETY: f non nul.
    let f = unsafe { &*f };
    w = lp_min(w, LP_RMAX);
    let inn = lp_rmake(lp_inset(f.r, w), f.rad - w, 0, 0);
    let vis = lp_visible(s, f.r);
    let mut y = vis.y;
    while y < vis.y + vis.h {
        ring_row(s, f, &inn, y);
        y += 1;
    }
}

pub(crate) fn lp_rtop(s: *mut t_surface, f: *const t_lrfill) {
    if !lp_ok(s) || f.is_null() {
        return;
    }
    // SAFETY: f non nul.
    let f = unsafe { &*f };
    if f.r.h <= 0 {
        return;
    }
    let vis = lp_visible(s, lp_rect(f.r.x, f.r.y, f.r.w, 1));
    let mut x = vis.x;
    while x < vis.x + vis.w {
        let p = lp_pt(x, vis.y);
        lp_plot(s, p, f.top, lp_rcov(f, p) as u32);
        x += 1;
    }
}

pub(crate) fn lp_rfocus(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
    let r = lp_clean(r);
    if r.w <= 0 || r.h <= 0 {
        return;
    }
    let m = LCT_FOCUS_GAP + LCT_FOCUS_W;
    let f = lp_rmake(lp_inset(r, -m), rad + m, c, c);
    lp_rring(s, &f, LCT_FOCUS_W);
}

fn btn_shade(c: t_color, st: i32) -> t_color {
    if st == LS_HOT {
        return crate::ink::lp_mix(c, LC_WHITE, 36);
    }
    if st == LS_PRESSED {
        return crate::ink::lp_mix(c, LC_BLACK, 56);
    }
    c
}

fn btn_primary(s: *mut t_surface, b: &t_lunabtn, rad: i32) {
    let mut f = lp_rmake(
        b.r,
        rad,
        btn_shade(VT_ACCENT_HI, b.state),
        btn_shade(VT_ACCENT, b.state),
    );
    lp_rfill(s, &f);
    f.top = LCT_RIM_PRIMARY;
    f.bottom = LCT_RIM_PRIMARY;
    lp_rtop(s, &f);
}

fn btn_body(s: *mut t_surface, b: &t_lunabtn, rad: i32) {
    let mut fill = pal_now().control;
    if b.state == LS_DISABLED {
        fill = lp_tone_alpha(fill, (fill >> 24) / 2);
    }
    let mut f = lp_rmake(lp_inset(b.r, 1), rad - 1, fill, fill);
    lp_rfill(s, &f);
    if b.state != LS_HOT && b.state != LS_PRESSED {
        return;
    }
    f.top = if b.state == LS_PRESSED {
        lp_tone_press()
    } else {
        lp_tone_hover()
    };
    f.bottom = f.top;
    lp_rfill(s, &f);
}

fn btn_rims(s: *mut t_surface, b: &t_lunabtn, rad: i32) {
    let pal = pal_now();
    let edge = lp_tone_alpha(pal.muted, 0x59);
    let mut f = lp_rmake(b.r, rad, edge, edge);
    lp_rring(s, &f, 1);
    f = lp_rmake(lp_inset(b.r, 1), rad - 1, pal.rim_top, pal.rim_top);
    lp_rtop(s, &f);
}

pub(crate) fn luna_button(s: *mut t_surface, src: *const t_lunabtn) {
    if !lp_ok(s) || src.is_null() {
        return;
    }
    // SAFETY: src non nul.
    let src = unsafe { &*src };
    let mut b = *src;
    b.r = lp_clean(src.r);
    if b.r.w <= 0 || b.r.h <= 0 {
        return;
    }
    let rad = lp_min(LCT_RAD_CTRL, b.r.h / 2);
    if lp_btn_primary(&b) {
        btn_primary(s, &b, rad);
    } else {
        btn_body(s, &b, rad);
    }
    if !lp_btn_primary(&b) && b.state != LS_DISABLED {
        btn_rims(s, &b, rad);
    }
    if (b.focus || b.state == LS_FOCUSED) && b.state != LS_DISABLED {
        lp_rfocus(s, b.r, rad, pal_now().accent);
    }
}

pub(crate) fn lp_choice_colors(c: *const t_lchoice, f: *mut t_lrfill, ring: *mut t_color) {
    // SAFETY: c, f et ring non nuls, comme lp_choice_colors en C.
    let c = unsafe { &*c };
    let pal = pal_now();
    let mut top = pal.input;
    let mut edge = lp_tone_alpha(pal.muted, 0xa6);
    if c.st == LS_HOT || c.on {
        edge = pal.accent;
    }
    if c.st == LS_PRESSED {
        top = lp_tone_press();
    }
    if c.st == LS_DISABLED {
        top = lp_tone_alpha(top, (top >> 24) / 2);
        edge = lp_tone_alpha(pal.dim, 0xa6);
    }
    let mut bottom = top;
    if c.on && c.solid && c.st != LS_DISABLED {
        top = VT_ACCENT_HI;
        bottom = VT_ACCENT;
    }
    // SAFETY: f et ring non nuls, exclusifs le temps de l'écriture.
    unsafe {
        (*f).top = top;
        (*f).bottom = bottom;
        *ring = edge;
    }
}

pub(crate) fn lp_choice(s: *mut t_surface, c: *const t_lchoice) {
    // SAFETY: c non nul. L'appelant, comme lp_choice en C, le garantit.
    let c = unsafe { &*c };
    let mut f = t_lrfill {
        r: lp_rect(0, 0, 0, 0),
        rad: 0,
        top: 0,
        bottom: 0,
    };
    let mut ring = 0;
    lp_choice_colors(c, &mut f, &mut ring);
    let edge = lp_rmake(c.r, c.rad, ring, ring);
    f.r = lp_inset(edge.r, 1);
    f.rad = lp_max(edge.rad - 1, 0);
    lp_rfill(s, &f);
    lp_rring(s, &edge, 1);
}

fn radio_cover(box_: i32, q: t_point, filet: i32) -> u32 {
    let mut n = 0u32;
    let mut i = 0;
    let lim = i64::from(box_)
        .wrapping_mul(i64::from(RADIO_SUB))
        .wrapping_mul(i64::from(box_))
        .wrapping_mul(i64::from(RADIO_SUB));
    let inner_i = box_.wrapping_mul(RADIO_SUB).wrapping_sub(filet);
    let inner = i64::from(inner_i).wrapping_mul(i64::from(inner_i));
    while i < RADIO_SUB * RADIO_SUB {
        let dx =
            q.x.wrapping_mul(RADIO_SUB)
                .wrapping_add(i % RADIO_SUB)
                .wrapping_mul(2)
                .wrapping_add(1)
                .wrapping_sub(box_.wrapping_mul(RADIO_SUB));
        let dy =
            q.y.wrapping_mul(RADIO_SUB)
                .wrapping_add(i / RADIO_SUB)
                .wrapping_mul(2)
                .wrapping_add(1)
                .wrapping_sub(box_.wrapping_mul(RADIO_SUB));
        let d2 = i64::from(dx)
            .wrapping_mul(i64::from(dx))
            .wrapping_add(i64::from(dy).wrapping_mul(i64::from(dy)));
        if d2 <= lim && d2 > inner {
            n += 1;
        }
        i += 1;
    }
    n
}

fn radio_color(c: &t_lchoice) -> t_color {
    let pal = pal_now();
    if c.st == LS_DISABLED {
        return lp_tone_alpha(pal.dim, 0xa6);
    }
    if c.on || c.st == LS_HOT {
        return pal.accent;
    }
    if lp_dark() {
        VT_RADIO_BORDER_D
    } else {
        VT_RADIO_BORDER
    }
}

fn radio_filet(s: *mut t_surface, c: &t_lchoice) {
    let col = radio_color(c);
    let filet = if c.on { RADIO_THICK } else { RADIO_FINE };
    let vis = lp_visible(s, c.r);
    let mut y = vis.y;
    while y < vis.y + vis.h {
        let mut x = vis.x;
        while x < vis.x + vis.w {
            let q = lp_pt(x, y);
            let cov = radio_cover(c.r.w, lp_pt(q.x - c.r.x, q.y - c.r.y), filet);
            lp_plot(s, q, col, cov);
            x += 1;
        }
        y += 1;
    }
}

pub(crate) fn lp_radio(s: *mut t_surface, c: *const t_lchoice) {
    if !lp_ok(s) || c.is_null() {
        return;
    }
    // SAFETY: c non nul.
    let c = unsafe { &*c };
    if c.r.w <= 0 || c.r.h != c.r.w {
        return;
    }
    let mut f = t_lrfill {
        r: c.r,
        rad: c.r.w / 2,
        top: 0,
        bottom: 0,
    };
    let mut ring = 0;
    lp_choice_colors(c, &mut f, &mut ring);
    f.r = c.r;
    f.rad = c.r.w / 2;
    lp_rfill(s, &f);
    radio_filet(s, c);
}

pub(crate) fn luna_checkbox(s: *mut t_surface, p: t_point, st: t_lunastate, on: bool) {
    if !lp_ok(s) || !lp_pt_ok(p) {
        return;
    }
    let n = det_now().check_size;
    let c = t_lchoice {
        r: lp_rect(p.x, p.y, n, n),
        rad: 4,
        st,
        on,
        solid: true,
    };
    lp_choice(s, &c);
    let sc = t_lscale {
        ox: p.x,
        oy: p.y,
        num: 1,
        den: 2,
    };
    if on {
        lp_ops(s, &sc, G_TICK.as_ptr(), 2);
    }
}

pub(crate) fn luna_radio(s: *mut t_surface, p: t_point, st: t_lunastate, on: bool) {
    if !lp_ok(s) || !lp_pt_ok(p) {
        return;
    }
    let n = det_now().check_size;
    let c = t_lchoice {
        r: lp_rect(p.x, p.y, n, n),
        rad: n / 2,
        st,
        on,
        solid: false,
    };
    lp_radio(s, &c);
}

fn sg_rect(r: &t_rect, vert: bool, off: i32, len: i32) -> t_rect {
    if vert {
        lp_rect(r.x, r.y + off, r.w, len)
    } else {
        lp_rect(r.x + off, r.y, len, r.h)
    }
}

fn sg_thumb(sb: &t_lunascroll, vert: bool, a: i32, trk: i32) -> t_rect {
    if sb.total <= 0 || sb.page <= 0 || sb.page >= sb.total || trk <= 0 {
        return lp_rect(sb.r.x, sb.r.y, 0, 0);
    }
    let mut len = i64::from(trk) * i64::from(sb.page) / i64::from(sb.total);
    len = i64::from(lp_clamp(len as i32, det_now().thumb_min, trk));
    let range = i64::from(sb.total.wrapping_sub(sb.page));
    let off = i64::from(trk) - len;
    let off = off * i64::from(lp_clamp(sb.pos, 0, range as i32)) / range;
    sg_rect(&sb.r, vert, a + off as i32, len as i32)
}

pub(crate) fn luna_scroll_layout(src: *const t_lunascroll, out: *mut t_scrollparts) {
    if src.is_null() || out.is_null() {
        return;
    }
    // SAFETY: src non nul.
    let src = unsafe { &*src };
    let mut sb = *src;
    sb.r = lp_clean(src.r);
    let vert = sb.r.h >= sb.r.w;
    let len = if vert { sb.r.h } else { sb.r.w };
    let a = lp_min(met_now().scroll_w, len / 2);
    let parts = t_scrollparts {
        up: sg_rect(&sb.r, vert, 0, a),
        down: sg_rect(&sb.r, vert, len - a, a),
        track: sg_rect(&sb.r, vert, a, len - 2 * a),
        thumb: sg_thumb(&sb, vert, a, len - 2 * a),
    };
    // SAFETY: out non nul, exclusif.
    unsafe { *out = parts };
}

pub(crate) fn lp_sthumb(s: *mut t_surface, r: *const t_rect, st: t_lunastate, vert: bool) {
    // SAFETY: r non nul, comme lp_sthumb en C.
    let r = unsafe { &*r };
    if r.w <= 0 || r.h <= 0 {
        return;
    }
    let pal = pal_now();
    let mut c = lp_tone_alpha(pal.muted, 0x80);
    if st == LS_HOT {
        c = lp_tone_alpha(pal.muted, 0xb3);
    }
    if st == LS_PRESSED {
        c = pal.accent;
    }
    let inn = if vert {
        lp_rect(r.x + 2, r.y, lp_max(r.w - 4, 1), r.h)
    } else {
        lp_rect(r.x, r.y + 2, r.w, lp_max(r.h - 4, 1))
    };
    let f = lp_rmake(inn, LP_RMAX, c, c);
    lp_rfill(s, &f);
}

fn sc_button(s: *mut t_surface, r: t_rect, st: t_lunastate, dir: i32) {
    let pal = pal_now();
    let mut arrow = pal.muted;
    if st == LS_DISABLED {
        arrow = pal.dim;
    }
    if st == LS_HOT || st == LS_PRESSED {
        let mut f = lp_rmake(r, LP_RMAX, lp_tone_hover(), lp_tone_hover());
        if st == LS_PRESSED {
            f.top = lp_tone_press();
            f.bottom = f.top;
        }
        lp_rfill(s, &f);
    }
    lp_arrow(s, r, dir, arrow);
}

pub(crate) fn luna_scrollbar_ex(s: *mut t_surface, src: *const t_lunascroll) {
    if !lp_ok(s) || src.is_null() {
        return;
    }
    // SAFETY: src non nul.
    let src = unsafe { &*src };
    let mut sb = *src;
    sb.r = lp_clean(src.r);
    if sb.r.w <= 0 || sb.r.h <= 0 {
        return;
    }
    let mut p = t_scrollparts {
        up: lp_rect(0, 0, 0, 0),
        down: lp_rect(0, 0, 0, 0),
        track: lp_rect(0, 0, 0, 0),
        thumb: lp_rect(0, 0, 0, 0),
    };
    luna_scroll_layout(&sb, &mut p);
    let vert = sb.r.h >= sb.r.w;
    let f = lp_rmake(sb.r, LP_RMAX, lp_tone_track(), lp_tone_track());
    lp_rfill(s, &f);
    sc_button(s, p.up, sb.up, 2 - 2 * i32::from(vert));
    sc_button(s, p.down, sb.down, 3 - 2 * i32::from(vert));
    lp_sthumb(s, &p.thumb, sb.thumb, vert);
}

pub(crate) fn luna_scrollbar(s: *mut t_surface, r: t_rect, st: t_lunastate) {
    let mut sb = t_lunascroll {
        r,
        total: 2,
        page: 1,
        pos: 0,
        up: LS_NORMAL,
        down: LS_NORMAL,
        thumb: st,
    };
    if st == LS_DISABLED {
        sb.total = 0;
        sb.up = LS_DISABLED;
        sb.down = LS_DISABLED;
    }
    luna_scrollbar_ex(s, &sb);
}

pub(crate) fn luna_progress(s: *mut t_surface, r: t_rect, mut pct: u32) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w < 2 || r.h < 2 {
        return;
    }
    let track = lp_tone_track();
    let mut f = lp_rmake(r, r.h / 2, track, track);
    lp_rfill(s, &f);
    if pct > 100 {
        pct = 100;
    }
    let w = (i64::from(r.w) * i64::from(pct) / 100) as i32;
    if w <= 0 {
        return;
    }
    let accent = pal_now().accent;
    f = lp_rmake(lp_rect(r.x, r.y, w, r.h), r.h / 2, accent, accent);
    lp_rfill(s, &f);
}

fn panel_paint(s: *mut t_surface, r: t_rect, rad: i32) {
    let pal = pal_now();
    let edge = crate::ink::lp_mix(pal.opaque_panel, pal.ink, 46);
    let mut f = lp_rmake(r, rad, edge, edge);
    lp_rfill(s, &f);
    f = lp_rmake(
        lp_inset(f.r, 1),
        rad - 1,
        pal.opaque_panel,
        pal.opaque_panel,
    );
    lp_rfill(s, &f);
}

pub(crate) fn luna_menu_panel(s: *mut t_surface, r: t_rect) {
    if lp_ok(s) {
        panel_paint(s, r, LCT_RAD_MENU);
    }
}

pub(crate) fn luna_menu_item(s: *mut t_surface, r: t_rect, st: t_lunastate) {
    if !lp_ok(s) || (st != LS_HOT && st != LS_PRESSED && st != LS_FOCUSED) {
        return;
    }
    let pal = pal_now();
    let c = if st == LS_PRESSED {
        pal.sel_ring
    } else {
        pal.sel_bg
    };
    let mut f = lp_rmake(r, LCT_RAD_ITEM, c, c);
    lp_rfill(s, &f);
    if st == LS_FOCUSED {
        f.top = pal.sel_ring;
        lp_rring(s, &f, 1);
    }
}

pub(crate) fn luna_tooltip(s: *mut t_surface, r: t_rect) {
    if lp_ok(s) {
        panel_paint(s, r, LCT_RAD_ITEM);
    }
}

fn edit_colors(enabled: bool) -> (t_color, t_color) {
    let pal = pal_now();
    let mut fill = pal.input;
    let mut ring = lp_tone_alpha(pal.muted, 0x73);
    if !enabled {
        fill = lp_tone_alpha(fill, (fill >> 24) / 2);
        ring = lp_tone_alpha(pal.dim, 0x73);
    }
    (fill, ring)
}

pub(crate) fn luna_edit_frame(s: *mut t_surface, r: t_rect, enabled: bool) {
    if !lp_ok(s) {
        return;
    }
    let r = lp_clean(r);
    let (fill, ring) = edit_colors(enabled);
    let rad = lp_min(LCT_RAD_FIELD, r.h / 2);
    let mut f = lp_rmake(lp_inset(r, 1), rad - 1, fill, fill);
    lp_rfill(s, &f);
    f = lp_rmake(r, rad, ring, ring);
    lp_rring(s, &f, 1);
}

fn group_part(s: *mut t_surface, f: &t_lrfill, part: t_rect) {
    // SAFETY: s non nul, lp_ok déjà vrai. On sauve le clip comme group_part en C.
    let keep = unsafe { (*s).clip };
    // SAFETY: même surface, écriture du clip le temps du trait.
    unsafe { (*s).clip = lp_isect(keep, part) };
    lp_rring(s, f, 1);
    // SAFETY: restauration du clip sauvé.
    unsafe { (*s).clip = keep };
}

fn group_draw(s: *mut t_surface, f: &t_lrfill, label_w: i32) {
    if label_w == 0 {
        lp_rring(s, f, 1);
        return;
    }
    let gap0 = f.r.x + det_now().group_pad;
    let gap1 = gap0 + label_w;
    group_part(s, f, lp_rect(f.r.x, f.r.y, gap0 - f.r.x, f.r.h));
    group_part(s, f, lp_rect(gap1, f.r.y, f.r.x + f.r.w - gap1, f.r.h));
    group_part(s, f, lp_rect(gap0, f.r.y + 1, label_w, f.r.h - 1));
}

pub(crate) fn luna_groupbox(s: *mut t_surface, r: t_rect, label_w: i32) {
    let r = lp_clean(r);
    let label_w = lp_clamp(label_w, 0, LP_COORD_MAX);
    let dy = det_now().group_line_y;
    if !lp_ok(s) || r.w < 8 || r.h < 8 || r.h - dy < 8 {
        return;
    }
    let c = lp_tone_alpha(pal_now().muted, 0x4d);
    let f = lp_rmake(lp_rect(r.x, r.y + dy, r.w, r.h - dy), LCT_RAD_GROUP, c, c);
    group_draw(s, &f, label_w);
}

pub(crate) mod ffi {
    use crate::abi::{
        t_color, t_lchoice, t_lrfill, t_lunabtn, t_lunascroll, t_lunastate, t_point, t_rect,
        t_scrollparts, t_surface,
    };

    c_fn!(lp_btn_primary(b: *const t_lunabtn) -> bool { super::lp_btn_primary(b) });
    c_fn!(luna_button_ink(b: *const t_lunabtn) -> t_color { super::luna_button_ink(b) });
    c_fn!(lp_rmake(r: t_rect, rad: i32, top: t_color, bottom: t_color) -> t_lrfill {
        super::lp_rmake(r, rad, top, bottom)
    });
    c_fn!(lp_rcov(f: *const t_lrfill, p: t_point) -> i32 { super::lp_rcov(f, p) });
    c_fn!(lp_rfill(s: *mut t_surface, f: *const t_lrfill) { super::lp_rfill(s, f) });
    c_fn!(lp_rring(s: *mut t_surface, f: *const t_lrfill, w: i32) { super::lp_rring(s, f, w) });
    c_fn!(lp_rtop(s: *mut t_surface, f: *const t_lrfill) { super::lp_rtop(s, f) });
    c_fn!(lp_rfocus(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
        super::lp_rfocus(s, r, rad, c)
    });
    c_fn!(luna_button(s: *mut t_surface, src: *const t_lunabtn) { super::luna_button(s, src) });
    c_fn!(lp_choice_colors(c: *const t_lchoice, f: *mut t_lrfill, ring: *mut t_color) {
        super::lp_choice_colors(c, f, ring)
    });
    c_fn!(lp_choice(s: *mut t_surface, c: *const t_lchoice) { super::lp_choice(s, c) });
    c_fn!(lp_radio(s: *mut t_surface, c: *const t_lchoice) { super::lp_radio(s, c) });
    c_fn!(luna_checkbox(s: *mut t_surface, p: t_point, st: t_lunastate, on: bool) {
        super::luna_checkbox(s, p, st, on)
    });
    c_fn!(luna_radio(s: *mut t_surface, p: t_point, st: t_lunastate, on: bool) {
        super::luna_radio(s, p, st, on)
    });
    c_fn!(luna_scroll_layout(src: *const t_lunascroll, out: *mut t_scrollparts) {
        super::luna_scroll_layout(src, out)
    });
    c_fn!(lp_sthumb(s: *mut t_surface, r: *const t_rect, st: t_lunastate, vert: bool) {
        super::lp_sthumb(s, r, st, vert)
    });
    c_fn!(luna_scrollbar_ex(s: *mut t_surface, src: *const t_lunascroll) {
        super::luna_scrollbar_ex(s, src)
    });
    c_fn!(luna_scrollbar(s: *mut t_surface, r: t_rect, st: t_lunastate) {
        super::luna_scrollbar(s, r, st)
    });
    c_fn!(luna_progress(s: *mut t_surface, r: t_rect, pct: u32) { super::luna_progress(s, r, pct) });
    c_fn!(luna_menu_panel(s: *mut t_surface, r: t_rect) { super::luna_menu_panel(s, r) });
    c_fn!(luna_menu_item(s: *mut t_surface, r: t_rect, st: t_lunastate) {
        super::luna_menu_item(s, r, st)
    });
    c_fn!(luna_tooltip(s: *mut t_surface, r: t_rect) { super::luna_tooltip(s, r) });
    c_fn!(luna_edit_frame(s: *mut t_surface, r: t_rect, enabled: bool) {
        super::luna_edit_frame(s, r, enabled)
    });
    c_fn!(luna_groupbox(s: *mut t_surface, r: t_rect, label_w: i32) {
        super::luna_groupbox(s, r, label_w)
    });
}
