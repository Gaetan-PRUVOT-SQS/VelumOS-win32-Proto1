use crate::abi::{
    ICON_LOGO, LD_STRIP_PAD_L, LD_TRAY_CLOCK_W, LD_TRAY_DOTS_W, LD_TRAY_GAP, LD_TRAY_LANG_W,
    LD_TRAY_PAD_R, LD_TRAY_W, LP_COORD_MAX, LS_DEFAULT, LS_FOCUSED, LS_HOT, LS_PRESSED,
    LUNA_GLASS_DOCK, VT_BADGE, VT_DOCK_GAP, VT_DOCK_H, VT_DOCK_MARGIN, VT_DOCK_PAD_X,
    VT_LAUNCHER_D, VT_MUTED, VT_MUTED_D, VT_R_TASK, VT_R_TRAY, VT_TASK_H, VT_TASK_IND_W, VT_TRAY_H,
    t_color, t_ldplate, t_lunastate, t_point, t_rect, t_surface, t_taskbarlayout, t_traygeo,
};
use crate::glass::luna_glass;
use crate::ink::{lp_plot, lp_visible};
use crate::num::{lp_clamp, lp_clean, lp_max, lp_min, lp_ok, lp_pt, lp_rect};
use crate::ops::luna_icon;
use crate::pal::{ld_accent, ld_pick, met_now};
use crate::shape::lp_corner_cov;

pub(crate) fn ld_cov(p: *const t_ldplate, x: i32, y: i32) -> i32 {
    // SAFETY: p non nul, comme ld_cov en C.
    let p = unsafe { &*p };
    if x < p.r.x || y < p.r.y || x >= p.r.x.wrapping_add(p.r.w) || y >= p.r.y.wrapping_add(p.r.h) {
        return 0;
    }
    let rad = lp_min(p.rad, lp_min(p.r.w, p.r.h) / 2);
    let dx = lp_min(
        x.wrapping_sub(p.r.x),
        p.r.x.wrapping_add(p.r.w).wrapping_sub(1).wrapping_sub(x),
    );
    let dy = lp_min(
        y.wrapping_sub(p.r.y),
        p.r.y.wrapping_add(p.r.h).wrapping_sub(1).wrapping_sub(y),
    );
    if rad <= 0 || dx >= rad || dy >= rad {
        return 16;
    }
    lp_corner_cov(rad, dx, dy)
}

fn ld_bound(p: &t_ldplate) -> t_ldplate {
    let mut c = *p;
    c.r = lp_clean(p.r);
    c.rad = lp_clamp(p.rad, 0, LP_COORD_MAX);
    c.thick = lp_clamp(p.thick, 0, lp_min(c.r.w, c.r.h) / 2);
    c
}

pub(crate) fn ld_paint(s: *mut t_surface, p: *const t_ldplate, fnp: crate::abi::t_ldcov) {
    // SAFETY: p non nul.
    let p = unsafe { &*p };
    let c = ld_bound(p);
    if !lp_ok(s) || c.r.w <= 0 || c.r.h <= 0 {
        return;
    }
    let v = lp_visible(s, c.r);
    let mut y = v.y;
    while y < v.y + v.h {
        let mut x = v.x;
        while x < v.x + v.w {
            // SAFETY: fnp est ld_cov, cov_ring ou cov_rim. c est vivant.
            let cov = unsafe { fnp(&c, x, y) };
            lp_plot(s, t_point { x, y }, c.c, lp_clamp(cov, 0, 16) as u32);
            x += 1;
        }
        y += 1;
    }
}

pub(crate) fn ld_plate(s: *mut t_surface, p: *const t_ldplate) {
    ld_paint(s, p, ffi::ld_cov);
}

unsafe extern "C" fn cov_ring(p: *const t_ldplate, x: i32, y: i32) -> i32 {
    // SAFETY: p non nul, passé par ld_paint.
    let src = unsafe { *p };
    let mut inn = src;
    inn.r = lp_rect(
        src.r.x.wrapping_add(src.thick),
        src.r.y.wrapping_add(src.thick),
        src.r.w.wrapping_sub(src.thick.wrapping_mul(2)),
        src.r.h.wrapping_sub(src.thick.wrapping_mul(2)),
    );
    inn.rad = src.rad.wrapping_sub(src.thick);
    ld_cov(p, x, y) - ld_cov(&inn, x, y)
}

unsafe extern "C" fn cov_rim(p: *const t_ldplate, x: i32, y: i32) -> i32 {
    // SAFETY: p non nul, passé par ld_paint.
    let mut low = unsafe { *p };
    low.r.y = low.r.y.wrapping_add(1);
    ld_cov(p, x, y) - ld_cov(&low, x, y)
}

pub(crate) fn ld_ring(s: *mut t_surface, p: *const t_ldplate) {
    // SAFETY: p non nul, comme ld_ring en C.
    if unsafe { (*p).thick } <= 0 {
        return;
    }
    ld_paint(s, p, cov_ring);
}

pub(crate) fn ld_rim(s: *mut t_surface, p: *const t_ldplate) {
    ld_paint(s, p, cov_rim);
}

fn dock_rect(bar: t_rect) -> t_rect {
    let h = lp_min(VT_DOCK_H, bar.h);
    let y = lp_max(bar.y, bar.y + bar.h - VT_DOCK_MARGIN - h);
    lp_rect(
        bar.x + VT_DOCK_MARGIN,
        y,
        lp_max(bar.w - 2 * VT_DOCK_MARGIN, 0),
        h,
    )
}

pub(crate) fn luna_taskbar(s: *mut t_surface, r: t_rect) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w <= 2 * VT_DOCK_MARGIN || r.h <= 0 {
        return;
    }
    luna_glass(s, dock_rect(r), LUNA_GLASS_DOCK);
}

pub(crate) fn luna_taskbar_layout(bar: t_rect, out: *mut t_taskbarlayout) {
    if out.is_null() {
        return;
    }
    let d = dock_rect(lp_clean(bar));
    let m = met_now();
    let mut lay = t_taskbarlayout {
        start: lp_rect(
            d.x + VT_DOCK_PAD_X,
            d.y + (d.h - m.start_w) / 2,
            m.start_w,
            m.start_w,
        ),
        tasks: lp_rect(0, 0, 0, 0),
        tray: lp_rect(0, 0, 0, 0),
    };
    let tray = lp_clamp(
        d.w - 2 * VT_DOCK_PAD_X - m.start_w - VT_DOCK_GAP,
        0,
        LD_TRAY_W,
    );
    lay.tray = lp_rect(
        d.x + d.w - VT_DOCK_PAD_X - tray,
        d.y + (d.h - VT_TRAY_H) / 2,
        tray,
        VT_TRAY_H,
    );
    lay.tasks = lp_rect(
        lay.start.x + lay.start.w + VT_DOCK_GAP + LD_STRIP_PAD_L,
        d.y + (d.h - VT_TASK_H) / 2,
        0,
        VT_TASK_H,
    );
    lay.tasks.w = lp_max(lay.tray.x - VT_DOCK_GAP - lay.tasks.x, 0);
    // SAFETY: out non nul.
    unsafe { *out = lay };
}

fn plate_color(st: t_lunastate) -> t_color {
    if st == LS_PRESSED || st == LS_DEFAULT || st == LS_FOCUSED {
        return ld_pick(3_019_898_879, 0x29ff_ffff);
    }
    if st == LS_HOT {
        return ld_pick(0x66ff_ffff, 0x14ff_ffff);
    }
    ld_pick(0x38ff_ffff, 0x0dff_ffff)
}

fn task_shadow(s: *mut t_surface, r: t_rect) {
    let mut k = 3;
    while k > 0 {
        let p = t_ldplate {
            r: lp_rect(
                r.x - k + 1,
                r.y + 2 - k + 1,
                r.w + 2 * (k - 1),
                r.h + 2 * (k - 1),
            ),
            rad: VT_R_TASK + k - 1,
            c: ld_pick(0x0c28_2850, 0x1a00_0000),
            thick: 0,
        };
        ld_plate(s, &p);
        k -= 1;
    }
}

pub(crate) fn luna_task_button(s: *mut t_surface, r: t_rect, st: t_lunastate) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w <= 0 || r.h <= 0 {
        return;
    }
    let active = st == LS_PRESSED || st == LS_DEFAULT || st == LS_FOCUSED;
    if active {
        task_shadow(s, r);
    }
    let mut p = t_ldplate {
        r,
        rad: VT_R_TASK,
        c: plate_color(st),
        thick: 0,
    };
    ld_plate(s, &p);
    if !active {
        return;
    }
    p.c = ld_pick(0xffff_ffff, 0x38ff_ffff);
    ld_rim(s, &p);
    if r.w < VT_TASK_IND_W || r.h < 3 + 3 {
        return;
    }
    p.r = lp_rect(
        r.x + (r.w - VT_TASK_IND_W) / 2,
        r.y + r.h - 6,
        VT_TASK_IND_W,
        3,
    );
    p.rad = 1;
    p.c = ld_accent();
    ld_plate(s, &p);
}

fn orb_color(st: t_lunastate) -> t_color {
    if st == LS_HOT {
        ld_pick(0xa6ff_ffff, 0x2eff_ffff)
    } else {
        ld_pick(0x8cff_ffff, 0x1aff_ffff)
    }
}

pub(crate) fn luna_start_button(s: *mut t_surface, r: t_rect, st: t_lunastate) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w <= 0 || r.h <= 0 {
        return;
    }
    let mut p = t_ldplate {
        r,
        rad: lp_min(r.w, r.h) / 2,
        c: orb_color(st),
        thick: 0,
    };
    ld_plate(s, &p);
    p.c = ld_pick(0xffff_ffff, 0x40ff_ffff);
    ld_rim(s, &p);
    let logo = lp_min(r.w, r.h) * 32 / VT_LAUNCHER_D;
    luna_icon(
        s,
        lp_pt(r.x + (r.w - logo) / 2, r.y + (r.h - logo) / 2),
        ICON_LOGO,
        logo,
    );
    if st == LS_PRESSED {
        let ring = t_ldplate {
            r: lp_rect(r.x - 5, r.y - 5, r.w + 10, r.h + 10),
            rad: lp_min(r.w, r.h) / 2 + 5,
            c: ld_accent(),
            thick: 2,
        };
        ld_ring(s, &ring);
    }
}

pub(crate) fn luna_tray_geo(tray: t_rect, out: *mut t_traygeo) {
    if out.is_null() {
        return;
    }
    let tray = lp_clean(tray);
    let mut x = tray.x + tray.w - LD_TRAY_PAD_R - VT_BADGE;
    let mut g = t_traygeo {
        badge: lp_rect(x, tray.y + (tray.h - VT_BADGE) / 2, VT_BADGE, VT_BADGE),
        clock: lp_rect(0, 0, 0, 0),
        dots: lp_rect(0, 0, 0, 0),
        lang: lp_rect(0, 0, 0, 0),
    };
    x -= LD_TRAY_GAP + LD_TRAY_CLOCK_W;
    g.clock = lp_rect(x, tray.y, LD_TRAY_CLOCK_W, tray.h);
    x -= LD_TRAY_GAP + LD_TRAY_DOTS_W;
    g.dots = lp_rect(x, tray.y, LD_TRAY_DOTS_W, tray.h);
    x -= LD_TRAY_GAP + LD_TRAY_LANG_W;
    g.lang = lp_rect(x, tray.y, LD_TRAY_LANG_W, tray.h);
    // SAFETY: out non nul.
    unsafe { *out = g };
}

fn tray_dots(s: *mut t_surface, d: t_rect) {
    let y = d.y + (d.h - 8) / 2;
    let mut p = t_ldplate {
        r: lp_rect(d.x, y, 7, 7),
        rad: 4,
        c: ld_pick(VT_MUTED, VT_MUTED_D),
        thick: 0,
    };
    ld_plate(s, &p);
    p.r = lp_rect(d.x + 12, y, 7, 7);
    ld_plate(s, &p);
    p.r = lp_rect(d.x + 24, y, 14, 8);
    p.rad = 3;
    p.thick = 1;
    ld_ring(s, &p);
}

pub(crate) fn luna_tray(s: *mut t_surface, r: t_rect) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w <= 0 || r.h <= 0 {
        return;
    }
    let mut p = t_ldplate {
        r,
        rad: VT_R_TRAY,
        c: ld_pick(0x59ff_ffff, 0x12ff_ffff),
        thick: 0,
    };
    ld_plate(s, &p);
    p.c = ld_pick(0xccff_ffff, 0x1fff_ffff);
    ld_rim(s, &p);
    let mut g = t_traygeo {
        lang: lp_rect(0, 0, 0, 0),
        dots: lp_rect(0, 0, 0, 0),
        clock: lp_rect(0, 0, 0, 0),
        badge: lp_rect(0, 0, 0, 0),
    };
    luna_tray_geo(r, &mut g);
    tray_dots(s, g.dots);
    p.r = g.badge;
    p.rad = 11;
    p.c = ld_pick(3_019_898_879, 0x1fff_ffff);
    ld_plate(s, &p);
    p.c = ld_pick(0xffff_ffff, 0x33ff_ffff);
    ld_rim(s, &p);
}

pub(crate) mod ffi {
    use crate::abi::{
        t_ldcov, t_ldplate, t_lunastate, t_rect, t_surface, t_taskbarlayout, t_traygeo,
    };

    c_fn!(ld_cov(p: *const t_ldplate, x: i32, y: i32) -> i32 { super::ld_cov(p, x, y) });
    c_fn!(ld_paint(s: *mut t_surface, p: *const t_ldplate, fnp: t_ldcov) { super::ld_paint(s, p, fnp) });
    c_fn!(ld_plate(s: *mut t_surface, p: *const t_ldplate) { super::ld_plate(s, p) });
    c_fn!(ld_ring(s: *mut t_surface, p: *const t_ldplate) { super::ld_ring(s, p) });
    c_fn!(ld_rim(s: *mut t_surface, p: *const t_ldplate) { super::ld_rim(s, p) });
    c_fn!(luna_taskbar(s: *mut t_surface, r: t_rect) { super::luna_taskbar(s, r) });
    c_fn!(luna_taskbar_layout(bar: t_rect, out: *mut t_taskbarlayout) {
        super::luna_taskbar_layout(bar, out)
    });
    c_fn!(luna_task_button(s: *mut t_surface, r: t_rect, st: t_lunastate) {
        super::luna_task_button(s, r, st)
    });
    c_fn!(luna_start_button(s: *mut t_surface, r: t_rect, st: t_lunastate) {
        super::luna_start_button(s, r, st)
    });
    c_fn!(luna_tray_geo(tray: t_rect, out: *mut t_traygeo) { super::luna_tray_geo(tray, out) });
    c_fn!(luna_tray(s: *mut t_surface, r: t_rect) { super::luna_tray(s, r) });
}
