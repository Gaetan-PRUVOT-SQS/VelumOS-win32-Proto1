use crate::abi::{
    LUNA_GLASS_CONSOLE, LUNA_GLASS_KINDS, LUNA_THEME_DARK, LX_PASSES, LX_SCALE, VT_BLUR_CHIP,
    VT_BLUR_DOCK, VT_BLUR_PANEL, VT_BLUR_WINDOW, VT_GLASS_CHIP, VT_GLASS_CHIP_D, VT_GLASS_CONSOLE,
    VT_GLASS_DOCK, VT_GLASS_DOCK_D, VT_GLASS_FRAME, VT_GLASS_FRAME_D, VT_GLASS_PANEL,
    VT_GLASS_PANEL_D, VT_GLASS_WINDOW, VT_GLASS_WINDOW_D, VT_R_DOCK, VT_R_PANEL, VT_R_WINDOW,
    VT_R_WINDOW_LG, VT_SPEC_END_PCT, gfx_get, gfx_put, t_color, t_gkind, t_glasskind, t_lcring,
    t_lgl, t_lglass, t_point, t_rect, t_surface,
};
use crate::ink::{lp_fill, lp_mix, lp_visible};
use crate::num::{lp_clamp, lp_clean, lp_in, lp_isect, lp_max, lp_min, lp_ok, lp_rect};
use crate::pal::{lp_gscratch, lp_pal, reduced_get, theme_get};
use crate::shape::{lp_corner_cov, lp_corner_ring};

const GB_RMAX: i32 = 4096;
const GK_PILL_MAX: i32 = 4096;

fn cell_avg(s: &t_surface, c: t_rect) -> t_color {
    let mut sum = [0u32; 3];
    let mut n = 0u32;
    let mut y = c.y;
    while y < c.y + c.h {
        let mut x = c.x;
        while x < c.x + c.w {
            let i = (y as usize)
                .wrapping_mul(s.stride as usize)
                .wrapping_add(x as usize);
            // SAFETY: la cellule est dans la surface, comme cell_avg en C.
            let p = unsafe { *s.px.add(i) };
            sum[0] = sum[0].wrapping_add((p >> 16) & 0xff);
            sum[1] = sum[1].wrapping_add((p >> 8) & 0xff);
            sum[2] = sum[2].wrapping_add(p & 0xff);
            n = n.wrapping_add(1);
            x += 1;
        }
        y += 1;
    }
    0xff00_0000 | ((sum[0] / n) << 16) | ((sum[1] / n) << 8) | (sum[2] / n)
}

pub(crate) fn lp_gb_down(s: *const t_surface, src: t_rect, o: *mut crate::abi::t_lgb) {
    // SAFETY: s non nul, comme lp_gb_down.
    let sr = unsafe { &*s };
    // SAFETY: o non nul, distinct de s.
    let o = unsafe { &mut *o };
    o.src = src;
    o.w = (src.w + LX_SCALE - 1) / LX_SCALE;
    o.h = (src.h + LX_SCALE - 1) / LX_SCALE;
    let mut cy = 0;
    while cy < o.h {
        let mut cx = 0;
        while cx < o.w {
            let c = lp_rect(
                src.x + cx * LX_SCALE,
                src.y + cy * LX_SCALE,
                lp_min(LX_SCALE, src.w - cx * LX_SCALE),
                lp_min(LX_SCALE, src.h - cy * LX_SCALE),
            );
            let i = (cy * o.w + cx) as isize;
            // SAFETY: i est dans le tampon a, fourni par le scratch.
            unsafe { *o.a.offset(i) = cell_avg(sr, c) };
            cx += 1;
        }
        cy += 1;
    }
}

fn line_at(inp: *const u32, p: &t_lgl, i: i32) -> u32 {
    let k = lp_clamp(i, 0, p.n - 1);
    // SAFETY: l'index k * step vise une case de la ligne, comme en C.
    unsafe { *inp.offset(k.wrapping_mul(p.step) as isize) }
}

fn line_add(inp: *const u32, p: &t_lgl, i: i32, s: &mut [i32; 3]) {
    let c = line_at(inp, p, i);
    s[0] += ((c >> 16) & 0xff) as i32;
    s[1] += ((c >> 8) & 0xff) as i32;
    s[2] += (c & 0xff) as i32;
}

fn line_sub(inp: *const u32, p: &t_lgl, i: i32, s: &mut [i32; 3]) {
    let c = line_at(inp, p, i);
    s[0] -= ((c >> 16) & 0xff) as i32;
    s[1] -= ((c >> 8) & 0xff) as i32;
    s[2] -= (c & 0xff) as i32;
}

pub(crate) fn lp_gb_line(inp: *const u32, out: *mut u32, p: *const t_lgl) {
    // SAFETY: p non nul.
    let p = unsafe { &*p };
    let mut sum = [0i32; 3];
    let win = 2 * p.r + 1;
    let mut i = -p.r;
    while i <= p.r {
        line_add(inp, p, i, &mut sum);
        i += 1;
    }
    i = 0;
    while i < p.n {
        let c = 0xff00_0000
            | (((sum[0] / win) as u32) << 16)
            | (((sum[1] / win) as u32) << 8)
            | ((sum[2] / win) as u32);
        // SAFETY: i * step est l'index de sortie, comme en C.
        unsafe { *out.offset(i.wrapping_mul(p.step) as isize) = c };
        line_add(inp, p, i + p.r + 1, &mut sum);
        line_sub(inp, p, i - p.r, &mut sum);
        i += 1;
    }
}

pub(crate) fn lp_gb_blur(o: *mut crate::abi::t_lgb, mut r: i32) {
    // SAFETY: o non nul, tampons a et b vivants.
    let o = unsafe { &mut *o };
    r = lp_clamp(r, 1, GB_RMAX);
    let mut pass = 0;
    while pass < LX_PASSES {
        let p = t_lgl { n: o.w, step: 1, r };
        let mut i = 0;
        while i < o.h {
            // SAFETY: i * w vise une ligne du tampon réduit.
            lp_gb_line(
                unsafe { o.a.offset((i * o.w) as isize) },
                unsafe { o.b.offset((i * o.w) as isize) },
                &p,
            );
            i += 1;
        }
        let p = t_lgl {
            n: o.h,
            step: o.w,
            r,
        };
        i = 0;
        while i < o.w {
            // SAFETY: i vise une colonne du tampon réduit.
            lp_gb_line(
                unsafe { o.b.offset(i as isize) },
                unsafe { o.a.offset(i as isize) },
                &p,
            );
            i += 1;
        }
        pass += 1;
    }
}

fn lane(c: &[u32; 4], w: &[i32; 4], sh: u32) -> t_color {
    let mut v = 0u32;
    let mut k = 0;
    while k < 4 {
        v = v.wrapping_add(((c[k] >> sh) & 0xff).wrapping_mul(w[k] as u32));
        k += 1;
    }
    (v >> 16) & 0xff
}

pub(crate) fn lp_gb_sample(o: *const crate::abi::t_lgb, x: i32, y: i32) -> t_color {
    // SAFETY: o non nul.
    let o = unsafe { &*o };
    let sx = (x - o.src.x) * 64 - 96;
    let sy = (y - o.src.y) * 64 - 96;
    let i = [
        lp_clamp(sx >> 8, 0, o.w - 1),
        lp_clamp((sx >> 8) + 1, 0, o.w - 1),
        lp_clamp(sy >> 8, 0, o.h - 1),
        lp_clamp((sy >> 8) + 1, 0, o.h - 1),
    ];
    let w = [
        (256 - (sx & 255)) * (256 - (sy & 255)),
        (sx & 255) * (256 - (sy & 255)),
        (256 - (sx & 255)) * (sy & 255),
        (sx & 255) * (sy & 255),
    ];
    let mut c = [0u32; 4];
    let idx = [
        i[2] * o.w + i[0],
        i[2] * o.w + i[1],
        i[3] * o.w + i[0],
        i[3] * o.w + i[1],
    ];
    let mut k = 0;
    while k < 4 {
        // SAFETY: l'index est borné par w et h du tampon réduit.
        c[k] = unsafe { *o.a.offset(idx[k] as isize) };
        k += 1;
    }
    0xff00_0000 | (lane(&c, &w, 16) << 16) | (lane(&c, &w, 8) << 8) | lane(&c, &w, 0)
}

pub(crate) fn lp_saturate(c: t_color, sat10: i32) -> t_color {
    let mut ch = [
        ((c >> 16) & 0xff) as i32,
        ((c >> 8) & 0xff) as i32,
        (c & 0xff) as i32,
    ];
    let lum = (ch[0] * 77 + ch[1] * 151 + ch[2] * 28) >> 8;
    let mut k = 0;
    while k < 3 {
        ch[k] = lp_clamp(lum + (ch[k] - lum) * sat10 / 10, 0, 255);
        k += 1;
    }
    0xff00_0000 | ((ch[0] as u32) << 16) | ((ch[1] as u32) << 8) | (ch[2] as u32)
}

pub(crate) fn lp_rmask(g: *const t_lglass, x: i32, y: i32) -> i32 {
    // SAFETY: g non nul.
    let g = unsafe { &*g };
    let r = g.r;
    if x < r.x || y < r.y || x >= r.x + r.w || y >= r.y + r.h {
        return 0;
    }
    let ix = lp_min(x - r.x, r.x + r.w - 1 - x);
    let mut rad = lp_min(g.rad_top, lp_min(r.w, r.h) / 2);
    let mut iy = y - r.y;
    if iy >= rad {
        rad = lp_min(g.rad_bot, lp_min(r.w, r.h) / 2);
        iy = r.y + r.h - 1 - y;
    }
    if ix >= rad || iy >= rad {
        return 16;
    }
    lp_corner_cov(rad, ix, iy)
}

fn kind_at(i: i32) -> t_gkind {
    const K: [t_gkind; 9] = [
        t_gkind {
            fill: [VT_GLASS_DOCK, VT_GLASS_DOCK_D],
            blur: VT_BLUR_DOCK,
            rad: VT_R_DOCK,
            panel: false,
            pill: false,
        },
        t_gkind {
            fill: [VT_GLASS_PANEL, VT_GLASS_PANEL_D],
            blur: VT_BLUR_PANEL,
            rad: VT_R_PANEL,
            panel: true,
            pill: false,
        },
        t_gkind {
            fill: [VT_GLASS_PANEL, VT_GLASS_PANEL_D],
            blur: VT_BLUR_PANEL,
            rad: VT_R_WINDOW_LG,
            panel: true,
            pill: false,
        },
        t_gkind {
            fill: [VT_GLASS_CHIP, VT_GLASS_CHIP_D],
            blur: VT_BLUR_WINDOW,
            rad: VT_R_PANEL,
            panel: true,
            pill: false,
        },
        t_gkind {
            fill: [VT_GLASS_CHIP, VT_GLASS_CHIP_D],
            blur: VT_BLUR_CHIP,
            rad: 0,
            panel: false,
            pill: true,
        },
        t_gkind {
            fill: [VT_GLASS_FRAME, VT_GLASS_FRAME_D],
            blur: VT_BLUR_PANEL,
            rad: VT_R_WINDOW_LG,
            panel: false,
            pill: false,
        },
        t_gkind {
            fill: [VT_GLASS_WINDOW, VT_GLASS_WINDOW_D],
            blur: VT_BLUR_WINDOW,
            rad: VT_R_WINDOW,
            panel: false,
            pill: false,
        },
        t_gkind {
            fill: [VT_GLASS_CONSOLE, VT_GLASS_CONSOLE],
            blur: VT_BLUR_WINDOW,
            rad: VT_R_WINDOW,
            panel: false,
            pill: false,
        },
        t_gkind {
            fill: [0x80ff_ffff, 0x9928_2e3e],
            blur: VT_BLUR_CHIP,
            rad: VT_R_DOCK,
            panel: false,
            pill: false,
        },
    ];
    K[i as usize]
}

fn kind_fill(g: &mut t_lglass, k: &t_gkind, dark: bool) {
    // SAFETY: palette immuable.
    let p = unsafe { &*lp_pal() };
    g.fill = k.fill[usize::from(dark)];
    g.blur = k.blur;
    g.sat = if k.panel { p.sat_panel } else { p.sat };
    g.opaque = p.opaque_panel;
    g.spec = p.spec;
    g.rim = [p.rim_top, p.rim_in, p.rim_bottom];
}

pub(crate) fn lp_glass_make(g: *mut t_lglass, r: t_rect, kind: t_glasskind) -> bool {
    if !(0..LUNA_GLASS_KINDS).contains(&kind) {
        return false;
    }
    let k = kind_at(kind);
    // SAFETY: g non nul.
    let g = unsafe { &mut *g };
    g.r = r;
    g.hole = lp_rect(0, 0, 0, 0);
    g.rad_top = if k.pill {
        lp_min(r.h / 2, GK_PILL_MAX)
    } else {
        k.rad
    };
    g.rad_bot = g.rad_top;
    kind_fill(g, &k, theme_get() == LUNA_THEME_DARK);
    if kind == LUNA_GLASS_CONSOLE {
        g.opaque = 0xff0e_111a;
    }
    true
}

pub(crate) fn luna_glass_need(r: t_rect) -> usize {
    let r = lp_clean(r);
    2 * ((r.w + LX_SCALE - 1) / LX_SCALE) as usize * ((r.h + LX_SCALE - 1) / LX_SCALE) as usize
}

fn glass_pixel(g: &t_lglass, o: Option<&crate::abi::t_lgb>, p: t_point) -> t_color {
    let mut c = g.opaque;
    if let Some(o) = o {
        c = lp_saturate(lp_gb_sample(o, p.x, p.y), g.sat);
        c = lp_mix(c, g.fill | 0xff00_0000, g.fill >> 24);
    }
    let spec_h = g.r.h * VT_SPEC_END_PCT / 100;
    if spec_h > 0 && p.y - g.r.y < spec_h {
        let a = ((g.spec >> 24) as i32) * (spec_h - (p.y - g.r.y)) / spec_h;
        c = lp_mix(c, g.spec | 0xff00_0000, a as u32);
    }
    c
}

fn glass_row(s: *mut t_surface, g: &t_lglass, o: Option<&crate::abi::t_lgb>, v: t_rect) {
    let mut p = t_point { x: v.x, y: v.y };
    while p.x < v.x + v.w {
        if !lp_in(g.hole, p) {
            let cov = lp_rmask(g, p.x, p.y);
            if cov != 0 {
                let mut c = glass_pixel(g, o, p);
                if cov < 16 {
                    c = lp_mix(gfx_get(s, p), c, (cov * 255 / 16) as u32);
                }
                gfx_put(s, p, c | 0xff00_0000);
            }
        }
        p.x += 1;
    }
}

fn glass_blurred(s: *const t_surface, g: &t_lglass, o: *mut crate::abi::t_lgb) -> bool {
    // SAFETY: s validé par lp_ok.
    let sr = unsafe { &*s };
    let src = lp_isect(g.r, lp_rect(0, 0, sr.w, sr.h));
    let px =
        ((src.w + LX_SCALE - 1) / LX_SCALE) as usize * ((src.h + LX_SCALE - 1) / LX_SCALE) as usize;
    // SAFETY: o non nul, on écrit a et b.
    let or = unsafe { &mut *o };
    if reduced_get() || src.w <= 0 || src.h <= 0 || !lp_gscratch(px, &mut or.a, &mut or.b) {
        return false;
    }
    lp_gb_down(s, src, o);
    lp_gb_blur(o, lp_max(g.blur / LX_SCALE, 1));
    true
}

pub(crate) fn lp_glass(s: *mut t_surface, inn: *const t_lglass) {
    if !lp_ok(s) || inn.is_null() {
        return;
    }
    // SAFETY: inn non nul.
    let inn = unsafe { &*inn };
    let mut g = *inn;
    g.r = lp_clean(inn.r);
    if g.r.w <= 0 || g.r.h <= 0 {
        return;
    }
    let mut v = lp_visible(s, g.r);
    if v.w <= 0 || v.h <= 0 {
        return;
    }
    let mut o = crate::abi::t_lgb {
        a: core::ptr::null_mut(),
        b: core::ptr::null_mut(),
        w: 0,
        h: 0,
        src: lp_rect(0, 0, 0, 0),
    };
    let blurred = glass_blurred(s, &g, &mut o);
    while v.h > 0 {
        glass_row(
            s,
            &g,
            if blurred { Some(&o) } else { None },
            lp_rect(v.x, v.y, v.w, 1),
        );
        v.y += 1;
        v.h -= 1;
    }
    lp_glass_rim(s, &g);
}

fn rim_corner(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
    let mut k = t_lcring {
        x: r.x,
        y: r.y,
        dx: 1,
        dy: 1,
        rad,
        c,
    };
    lp_corner_ring(s, &k);
    k.x = r.x + r.w - 1;
    k.dx = -1;
    lp_corner_ring(s, &k);
}

fn rim_bottom(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
    let mut k = t_lcring {
        x: r.x,
        y: r.y + r.h - 1,
        dx: 1,
        dy: -1,
        rad,
        c,
    };
    lp_corner_ring(s, &k);
    k.x = r.x + r.w - 1;
    k.dx = -1;
    lp_corner_ring(s, &k);
}

pub(crate) fn lp_glass_rim(s: *mut t_surface, g: *const t_lglass) {
    // SAFETY: g non nul.
    let g = unsafe { &*g };
    let r = g.r;
    let rt = lp_clamp(g.rad_top, 0, lp_min(r.w, r.h) / 2);
    let rb = lp_clamp(g.rad_bot, 0, lp_min(r.w, r.h) / 2);
    rim_corner(s, r, rt, g.rim[1]);
    rim_bottom(s, r, rb, g.rim[1]);
    lp_fill(s, lp_rect(r.x + rt, r.y, r.w - 2 * rt, 1), g.rim[1]);
    lp_fill(s, lp_rect(r.x + rt, r.y, r.w - 2 * rt, 1), g.rim[0]);
    lp_fill(
        s,
        lp_rect(r.x + rb, r.y + r.h - 1, r.w - 2 * rb, 1),
        g.rim[1],
    );
    lp_fill(
        s,
        lp_rect(r.x + rb, r.y + r.h - 1, r.w - 2 * rb, 1),
        g.rim[2],
    );
    lp_fill(s, lp_rect(r.x, r.y + rt, 1, r.h - rt - rb), g.rim[1]);
    lp_fill(
        s,
        lp_rect(r.x + r.w - 1, r.y + rt, 1, r.h - rt - rb),
        g.rim[1],
    );
}

pub(crate) fn luna_glass(s: *mut t_surface, r: t_rect, kind: t_glasskind) {
    let r = lp_clean(r);
    let mut g = t_lglass {
        r: lp_rect(0, 0, 0, 0),
        hole: lp_rect(0, 0, 0, 0),
        rad_top: 0,
        rad_bot: 0,
        blur: 0,
        sat: 0,
        fill: 0,
        opaque: 0,
        spec: 0,
        rim: [0; 3],
    };
    if !lp_ok(s) || !lp_glass_make(&mut g, r, kind) {
        return;
    }
    lp_glass(s, &g);
}

pub(crate) mod ffi {
    use crate::abi::{t_color, t_glasskind, t_lgb, t_lgl, t_lglass, t_rect, t_surface};

    c_fn!(lp_rmask(g: *const t_lglass, x: i32, y: i32) -> i32 { super::lp_rmask(g, x, y) });
    c_fn!(lp_gb_down(s: *const t_surface, src: t_rect, o: *mut t_lgb) { super::lp_gb_down(s, src, o) });
    c_fn!(lp_gb_blur(o: *mut t_lgb, r: i32) { super::lp_gb_blur(o, r) });
    c_fn!(lp_gb_line(i: *const u32, o: *mut u32, p: *const t_lgl) { super::lp_gb_line(i, o, p) });
    c_fn!(lp_gb_sample(o: *const t_lgb, x: i32, y: i32) -> t_color { super::lp_gb_sample(o, x, y) });
    c_fn!(lp_saturate(c: t_color, sat10: i32) -> t_color { super::lp_saturate(c, sat10) });
    c_fn!(lp_glass_make(g: *mut t_lglass, r: t_rect, kind: t_glasskind) -> bool {
        super::lp_glass_make(g, r, kind)
    });
    c_fn!(luna_glass_need(r: t_rect) -> usize { super::luna_glass_need(r) });
    c_fn!(lp_glass(s: *mut t_surface, g: *const t_lglass) { super::lp_glass(s, g) });
    c_fn!(lp_glass_rim(s: *mut t_surface, g: *const t_lglass) { super::lp_glass_rim(s, g) });
    c_fn!(luna_glass(s: *mut t_surface, r: t_rect, kind: t_glasskind) { super::luna_glass(s, r, kind) });
}
