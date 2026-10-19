use crate::abi::{
    FONT_UI, LA_CENTER, LC_WHITE, LG_DIM_MAX, LP_BLOBS, LP_VEILS, LP_WALL_MAX, LUNA_GLASS_CARD,
    t_color, t_lbc, t_lboot, t_lbox, t_lgrad, t_lstop, t_ltext, t_lvc, t_lwall, t_rect, t_surface,
};
use crate::glass::luna_glass;
use crate::ink::{lp_fill, lp_mix};
use crate::mark::{lp_wordmark, lp_wordmark_w};
use crate::num::{lp_clamp, lp_clean, lp_isect, lp_isqrt, lp_max, lp_min, lp_ok, lp_rect};
use crate::ops::lp_emblem;
use crate::pal::pal_now;
use crate::shape::lp_box;
use crate::text::lp_text;
use core::ffi::c_char;

const BLOBS: [[i32; 6]; LP_BLOBS] = [
    [120, 180, 550, 750, 62, 1],
    [880, 120, 500, 700, 62, 2],
    [720, 950, 650, 700, 62, 3],
    [180, 1000, 550, 600, 60, 4],
];
const VEILS: [[i32; 8]; LP_VEILS] = [
    [300, 490, 400, 190, 1001, -213, 40, 0],
    [800, 230, 350, 150, 974, 316, 50, 1],
];
const BOOT_TEXT: &[u8] = b"D\xc3\xa9marrage en cours\0";

pub(crate) fn lp_smooth256(t: i32) -> i32 {
    let t = lp_clamp(t, 0, 256);
    t.wrapping_mul(t)
        .wrapping_mul(768i32.wrapping_sub(t.wrapping_mul(2)))
        >> 16
}

fn blob_setup(b: &mut t_lbc, w: i32, h: i32, i: usize) {
    let p = BLOBS[i];
    b.cx = p[0] * w / 1000;
    b.cy = p[1] * h / 1000;
    let rx = lp_max(p[2] * w / 1000, 1);
    let ry = lp_max(p[3] * h / 1000, 1);
    b.irx = (1 << 26) / rx;
    b.iry = (1 << 26) / ry;
    b.thr = p[4] * 1024 / 100;
    b.ithr = (255 << 16) / b.thr;
    b.col = pal_now().wall[p[5] as usize];
}

fn veil_setup(v: &mut t_lvc, w: i32, h: i32, i: usize) {
    let g = VEILS[i];
    let pal = pal_now();
    v.cx = g[0] * w / 1000;
    v.cy = g[1] * h / 1000;
    v.a = lp_max(g[2] * w / 1000, 1);
    v.b = lp_max(g[3] * h / 1000, 1);
    v.co = g[4];
    v.si = g[5];
    v.sigma = g[6];
    v.down = g[7] == 0;
    v.peak = if i == 1 { pal.veil_b } else { pal.veil_a };
}

pub(crate) fn lp_wall_setup(c: *mut t_lwall, r: t_rect, s: *const t_surface) {
    // SAFETY: c non nul.
    let c = unsafe { &mut *c };
    let w = lp_clamp(r.w, 8, LP_WALL_MAX);
    let h = lp_clamp(r.h, 8, LP_WALL_MAX);
    c.w = w;
    c.h = h;
    c.ox = r.x;
    c.oy = r.y;
    c.vis = crate::ink::lp_visible(s, r);
    c.base = pal_now().wall[0];
    let mut i = 0;
    while i < LP_BLOBS {
        blob_setup(&mut c.bc[i], w, h, i);
        i += 1;
    }
    i = 0;
    while i < LP_VEILS {
        veil_setup(&mut c.vc[i], w, h, i);
        i += 1;
    }
}

fn blob_alpha(b: &t_lbc, x: i32, y: i32) -> i32 {
    let nx = i64::from(x.wrapping_sub(b.cx)).wrapping_mul(i64::from(b.irx)) >> 16;
    let ny = i64::from(y.wrapping_sub(b.cy)).wrapping_mul(i64::from(b.iry)) >> 16;
    let d2 = nx.wrapping_mul(nx).wrapping_add(ny.wrapping_mul(ny));
    if d2 >= i64::from(b.thr).wrapping_mul(i64::from(b.thr)) {
        return 0;
    }
    let d = i64::from(lp_isqrt(d2 as u64));
    let scaled = (d.wrapping_mul(i64::from(b.ithr)) >> 16) as i32;
    let inner = 256i32.wrapping_sub(scaled.wrapping_mul(256).wrapping_div(255));
    lp_smooth256(inner).wrapping_mul(255).wrapping_div(256)
}

pub(crate) fn lp_wall_blobs(c: *const t_lwall, x: i32, y: i32) -> t_color {
    // SAFETY: c non nul.
    let c = unsafe { &*c };
    let mut col = c.base;
    let mut i = LP_BLOBS;
    while i > 0 {
        i -= 1;
        let a = blob_alpha(&c.bc[i], x, y);
        if a > 0 {
            col = lp_mix(col, c.bc[i].col, a as u32);
        }
    }
    col
}

fn veil_mask(v: &t_lvc, u: i32, w: i32) -> i32 {
    let nu = i64::from(u).wrapping_mul(1024).wrapping_div(i64::from(v.a));
    let nv = i64::from(w).wrapping_mul(1024).wrapping_div(i64::from(v.b));
    let sum = nu.wrapping_mul(nu).wrapping_add(nv.wrapping_mul(nv));
    let d = i64::from(lp_isqrt(sum as u64));
    let num = nu
        .wrapping_mul(nu)
        .wrapping_mul(i64::from(v.a))
        .wrapping_add(nv.wrapping_mul(nv).wrapping_mul(i64::from(v.b)));
    let m = num.wrapping_div(sum.wrapping_add(1));
    let sd = d.wrapping_sub(1024).wrapping_mul(m).wrapping_div(1024);
    let den = i64::from(3i32.wrapping_mul(v.sigma));
    let q = sd.wrapping_mul(256).wrapping_div(den);
    lp_smooth256(128i32.wrapping_sub(q as i32))
}

fn veil_alpha(v: &t_lvc, x: i32, y: i32) -> i32 {
    let dx = x.wrapping_sub(v.cx);
    let dy = y.wrapping_sub(v.cy);
    let u = dx
        .wrapping_mul(v.co)
        .wrapping_add(dy.wrapping_mul(v.si))
        .wrapping_div(1024);
    let w = dy
        .wrapping_mul(v.co)
        .wrapping_sub(dx.wrapping_mul(v.si))
        .wrapping_div(1024);
    let lim_u = v.a.wrapping_add(v.sigma.wrapping_mul(2));
    let lim_w = v.b.wrapping_add(v.sigma.wrapping_mul(2));
    if lp_max(u, u.wrapping_neg()) > lim_u || lp_max(w, w.wrapping_neg()) > lim_w {
        return 0;
    }
    let mut g =
        v.b.wrapping_add(w)
            .wrapping_mul(256)
            .wrapping_div(v.b.wrapping_mul(2));
    if v.down {
        g = 256i32.wrapping_sub(g);
    }
    g = lp_clamp(g, 0, 256);
    veil_mask(v, u, w)
        .wrapping_mul(g)
        .wrapping_div(256)
        .wrapping_mul(v.peak)
        .wrapping_div(256)
}

pub(crate) fn lp_wall_veils(c: *const t_lwall, mut col: t_color, x: i32, y: i32) -> t_color {
    // SAFETY: c non nul.
    let c = unsafe { &*c };
    let mut i = 0;
    while i < LP_VEILS {
        let a = veil_alpha(&c.vc[i], x, y);
        if a > 0 {
            col = lp_mix(col, LC_WHITE, a as u32);
        }
        i += 1;
    }
    col
}

fn wall_row(s: *mut t_surface, c: &t_lwall, y: i32) {
    let mut x = c.vis.x;
    while x < c.vis.x + c.vis.w {
        let u = lp_min(x - c.ox, c.w - 1);
        let v = lp_min(y - c.oy, c.h - 1);
        let col = lp_wall_veils(c, lp_wall_blobs(c, u, v), u, v) | 0xff00_0000;
        // SAFETY: (x, y) est dans le visible, px non nul par lp_ok.
        unsafe {
            let sr = &*s;
            let i = (y as usize)
                .wrapping_mul(sr.stride as usize)
                .wrapping_add(x as usize);
            *sr.px.add(i) = col;
        }
        x += 1;
    }
}

pub(crate) fn luna_wallpaper(s: *mut t_surface, r: t_rect) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w <= 0 || r.h <= 0 {
        return;
    }
    // SAFETY: t_lwall est un POD C, rempli par lp_wall_setup avant lecture.
    let mut c = unsafe { core::mem::zeroed::<t_lwall>() };
    lp_wall_setup(&mut c, r, s);
    let mut y = c.vis.y;
    while y < c.vis.y + c.vis.h {
        wall_row(s, &c, y);
        y += 1;
    }
}

pub(crate) fn luna_logon_bg(s: *mut t_surface, r: t_rect) {
    luna_wallpaper(s, r);
}

fn bar_pill(s: *mut t_surface, r: t_rect, top: t_color, bot: t_color) {
    if r.w < 1 || r.h < 1 {
        return;
    }
    let mut st = [
        t_lstop { at: 0, c: top },
        t_lstop {
            at: lp_max(r.h - 1, 1),
            c: bot,
        },
    ];
    let g = t_lgrad {
        st: st.as_ptr(),
        n: 2,
        tint: 0,
        tint_t: 0,
    };
    let b = t_lbox {
        r,
        rad: lp_min(r.h, r.w) / 2,
        ring: core::ptr::null(),
        nring: 0,
        fill: &g,
    };
    lp_box(s, &b);
    let _ = &mut st;
}

fn bar_shine(s: *mut t_surface, f: t_rect, c: t_color) {
    let rad = f.h / 2;
    if f.h < 6 || f.w <= 2 * rad + 2 {
        return;
    }
    lp_fill(s, lp_rect(f.x + rad, f.y + f.h / 4, f.w - 2 * rad, 1), c);
}

fn bar_span(r: &t_rect, mut pct: u32, tick: u32) -> t_rect {
    if pct > 0 {
        if pct > 100 {
            pct = 100;
        }
        return lp_rect(
            r.x,
            r.y,
            r.w.wrapping_mul(pct as i32).wrapping_div(100),
            r.h,
        );
    }
    let seg = r.h.wrapping_mul(4);
    let sum = r.w.wrapping_add(seg);
    if sum == 0 {
        return lp_rect(r.x, r.y, 0, 0);
    }
    let x = (tick.wrapping_mul(3) % (sum as u32)) as i32;
    let x = x.wrapping_sub(seg);
    lp_isect(lp_rect(r.x.wrapping_add(x), r.y, seg, r.h), *r)
}

pub(crate) fn lp_boot_bar(s: *mut t_surface, r: *const t_rect, pct: u32, tick: u32) {
    if !lp_ok(s) || r.is_null() {
        return;
    }
    // SAFETY: r non nul.
    let r = unsafe { &*r };
    if r.w < 2 || r.h < 2 || r.w > LG_DIM_MAX {
        return;
    }
    let pal = pal_now();
    let track = (pal.ink & 0x00ff_ffff) | 0x2400_0000;
    bar_pill(s, *r, track, track);
    let f = bar_span(r, pct, tick);
    bar_pill(s, f, pal.accent, pal.accent);
    bar_shine(s, f, lp_mix(pal.accent_hi, LC_WHITE, 128));
}

fn boot_stack(w: i32, h: i32, b: &mut t_lboot, e: i32) {
    let mh = lp_max(e / 5, 8);
    let bw = lp_clamp(w * 22 / 100, 96, 280);
    let bh = lp_clamp(e / 14, 6, 8);
    let mut y = (h - (e + e / 4 + mh + e / 3 + bh + 24)) / 2;
    b.emblem = lp_rect((w - e) / 2, y, e, e);
    y += e + e / 4;
    let mw = lp_wordmark_w(mh);
    b.mark = lp_rect((w - mw) / 2, y, mw, mh);
    y += mh + e / 3;
    b.bar = lp_rect((w - bw) / 2, y, bw, bh);
    b.text = lp_rect(0, y + bh + 10, w, 14);
}

fn boot_layout(w: i32, h: i32, b: &mut t_lboot) {
    let e = lp_clamp(lp_min(w, h) / 6, 32, 120);
    let pad = e / 2;
    boot_stack(w, h, b, e);
    let span = lp_max(b.bar.w, b.mark.w) + 2 * pad;
    b.card = lp_isect(
        lp_rect(
            (w - span) / 2,
            b.emblem.y - pad,
            span,
            b.text.y + b.text.h - b.emblem.y + 2 * pad,
        ),
        lp_rect(0, 0, w, h),
    );
}

fn boot_text(s: *mut t_surface, b: &t_lboot) {
    let t = t_ltext {
        font: FONT_UI,
        color: pal_now().muted,
        shadow: 0,
        text: BOOT_TEXT.as_ptr().cast::<c_char>(),
        box_: b.text,
        align: LA_CENTER,
    };
    lp_text(s, &t);
}

pub(crate) fn luna_boot_draw(s: *mut t_surface, pct: u32, tick: u32) {
    if !lp_ok(s) {
        return;
    }
    // SAFETY: lp_ok a lu la surface, w et h sont valides.
    let (w, h) = unsafe { ((*s).w, (*s).h) };
    luna_logon_bg(s, lp_rect(0, 0, w, h));
    let mut b = t_lboot {
        emblem: lp_rect(0, 0, 0, 0),
        mark: lp_rect(0, 0, 0, 0),
        bar: lp_rect(0, 0, 0, 0),
        text: lp_rect(0, 0, 0, 0),
        card: lp_rect(0, 0, 0, 0),
    };
    boot_layout(w, h, &mut b);
    if b.card.w > 0 && b.card.h > 0 {
        luna_glass(s, b.card, LUNA_GLASS_CARD);
    }
    lp_emblem(s, b.emblem);
    lp_wordmark(s, b.mark, pal_now().ink);
    lp_boot_bar(s, &b.bar, pct, tick);
    boot_text(s, &b);
}

pub(crate) mod ffi {
    use crate::abi::{t_color, t_lwall, t_rect, t_surface};

    c_fn!(lp_smooth256(t: i32) -> i32 { super::lp_smooth256(t) });
    c_fn!(lp_wall_setup(c: *mut t_lwall, r: t_rect, s: *const t_surface) {
        super::lp_wall_setup(c, r, s)
    });
    c_fn!(lp_wall_blobs(c: *const t_lwall, x: i32, y: i32) -> t_color { super::lp_wall_blobs(c, x, y) });
    c_fn!(lp_wall_veils(c: *const t_lwall, col: t_color, x: i32, y: i32) -> t_color {
        super::lp_wall_veils(c, col, x, y)
    });
    c_fn!(luna_wallpaper(s: *mut t_surface, r: t_rect) { super::luna_wallpaper(s, r) });
    c_fn!(luna_logon_bg(s: *mut t_surface, r: t_rect) { super::luna_logon_bg(s, r) });
    c_fn!(lp_boot_bar(s: *mut t_surface, r: *const t_rect, pct: u32, tick: u32) {
        super::lp_boot_bar(s, r, pct, tick)
    });
    c_fn!(luna_boot_draw(s: *mut t_surface, pct: u32, tick: u32) { super::luna_boot_draw(s, pct, tick) });
}
