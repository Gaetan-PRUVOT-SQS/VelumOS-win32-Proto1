use core::ffi::c_char;

use crate::abi::{
    HT_NOWHERE, ICON_PROGRAM, ICON_RUN, LC_BLACK, LC_WHITE, LS_NORMAL, LUNA_COLOR_TEXT,
    LUNA_LAUNCHER_HEIGHT, LUNA_LAUNCHER_LOGOFF, LUNA_LAUNCHER_MARGIN_X, LUNA_LAUNCHER_NONE,
    LUNA_LAUNCHER_POWER, LUNA_LAUNCHER_WIDTH, LUNA_LM_APPS_MAX, LUNA_LM_LABEL_MAX, LUNA_LM_NONE,
    LUNA_LM_QUERY_MAX, LUNA_LM_RECENTS, LUNA_LM_ROWS, LUNA_LV_ALL, LUNA_LV_PINNED, LUNA_LV_RECENT,
    LUNA_LV_SEARCH, LUNA_THEME_DARK, LUNA_THEME_LIGHT, VT_ACCENT, VT_ACCENT_HI, VT_ACCENT_SEL_BG,
    VT_ACCENT_SEL_BG_D, VT_ACCENT_SEL_RING, VT_ACCENT_SEL_RING_D, VT_ACCENT_TEXT_DARK, VT_DANGER,
    VT_DANGER_D, VT_DIM, VT_DIM_D, VT_INK, VT_INK_2, VT_INK_2_D, VT_INK_D, VT_MUTED, VT_MUTED_D,
    VT_OPAQUE_CONTENT, VT_OPAQUE_CONTENT_D, VT_OPAQUE_PANEL, VT_OPAQUE_PANEL_D, gfx_fill, gfx_get,
    t_lmapp, t_lmodel, t_lunabtn, t_lunascroll, t_point, t_surface, t_taskbarlayout, t_traygeo,
};
use crate::frame::{luna_hit_test, luna_window_client};
use crate::glass::luna_glass_need;
use crate::ink::lp_mix;
use crate::launch::{
    luna_launcher_hit, luna_launcher_layout, luna_launcher_place, luna_launcher_place_in,
    luna_launcher_row_rect, luna_launcher_tab_rect, luna_lm_backspace, luna_lm_clear_query,
    luna_lm_init, luna_lm_match, luna_lm_move, luna_lm_row, luna_lm_scroll, luna_lm_select,
    luna_lm_selected, luna_lm_set_apps, luna_lm_set_tab, luna_lm_touch, luna_lm_type, luna_lm_view,
};
use crate::mark::lp_wordmark_w;
use crate::num::{lp_floor16, lp_isqrt, lp_rect, lp_round16};
use crate::pal::{self, det_now, met_now};
use crate::shape::lp_corner_cov;

fn c(s: &[u8]) -> *const c_char {
    s.as_ptr().cast()
}

struct Theme(std::sync::MutexGuard<'static, ()>);

fn theme() -> Theme {
    static G: std::sync::Mutex<()> = std::sync::Mutex::new(());
    let g = G.lock().unwrap_or_else(|e| e.into_inner());
    // SAFETY: le mutex est tenu, un seul test écrit le thème.
    unsafe { pal::ffi::luna_theme_set(LUNA_THEME_LIGHT) };
    Theme(g)
}

impl Drop for Theme {
    fn drop(&mut self) {
        let _garde = &self.0;
        // SAFETY: le mutex est encore tenu par self.0.
        unsafe { pal::ffi::luna_theme_set(LUNA_THEME_LIGHT) };
    }
}

fn app(label: &[u8], id: u32, icon: i32) -> t_lmapp {
    let mut a = t_lmapp {
        label: [0; LUNA_LM_LABEL_MAX],
        id,
        icon,
    };
    let n = label.len().min(LUNA_LM_LABEL_MAX - 1);
    // SAFETY: n octets de label tiennent dans le tableau, le dernier reste NUL.
    unsafe { core::ptr::copy_nonoverlapping(label.as_ptr().cast(), a.label.as_mut_ptr(), n) };
    a
}

fn garnir(m: &mut t_lmodel) {
    let apps = [
        app(b"Bonjour", 0, ICON_PROGRAM),
        app(b"\xc3\x89diteur", 1, ICON_PROGRAM),
        app(b"Calculatrice", 2, ICON_PROGRAM),
        app(b"Console", 3, ICON_PROGRAM),
        app(b"Mon Navigateur", 4, ICON_PROGRAM),
        app(b"Zebre", 5, ICON_PROGRAM),
        app(b"Ex\xc3\xa9cuter...", 1000, ICON_RUN),
    ];
    luna_lm_init(m);
    assert_eq!(luna_lm_set_apps(m, apps.as_ptr(), 7), 0);
}

#[test]
fn maths_bornes() {
    assert_eq!(lp_floor16(0), 0);
    assert_eq!(lp_floor16(15), 0);
    assert_eq!(lp_floor16(16), 1);
    assert_eq!(lp_floor16(-1), -1);
    assert_eq!(lp_floor16(-16), -1);
    assert_eq!(lp_floor16(-17), -2);
    assert_eq!(lp_round16(0), 0);
    assert_eq!(lp_round16(8), 1);
    assert_eq!(lp_round16(-8), 0);
    assert_eq!(lp_round16(-9), -1);
    assert_eq!(lp_isqrt(0), 0);
    assert_eq!(lp_isqrt(1), 1);
    assert_eq!(lp_isqrt(4), 2);
    assert_eq!(lp_isqrt(9), 3);
    assert_eq!(lp_isqrt(16), 4);
    assert_eq!(lp_mix(0, 0x00ff_ffff, 128), 0x0080_8080);
    assert_eq!(lp_corner_cov(0, 0, 0), 0);
    assert_eq!(lp_corner_cov(1, 0, 0), 13);
}

#[test]
fn verre_et_mot() {
    assert_eq!(luna_glass_need(lp_rect(0, 0, 10, 10)), 18);
    assert_eq!(luna_glass_need(lp_rect(16_777_217, 0, 10, 10)), 0);
    assert_eq!(luna_glass_need(lp_rect(0, i32::MIN, 10, 10)), 0);
    assert_eq!(lp_wordmark_w(16), 87);
}

#[test]
fn couleurs_et_metriques() {
    let _t = theme();
    let light = [
        VT_INK,
        VT_INK_2,
        VT_MUTED,
        VT_DIM,
        VT_ACCENT,
        VT_ACCENT_HI,
        VT_ACCENT,
        VT_DANGER,
        VT_OPAQUE_CONTENT,
        VT_OPAQUE_PANEL,
        VT_ACCENT_SEL_BG,
        VT_ACCENT_SEL_RING,
    ];
    let dark = [
        VT_INK_D,
        VT_INK_2_D,
        VT_MUTED_D,
        VT_DIM_D,
        VT_ACCENT_HI,
        VT_ACCENT_HI,
        VT_ACCENT_TEXT_DARK,
        VT_DANGER_D,
        VT_OPAQUE_CONTENT_D,
        VT_OPAQUE_PANEL_D,
        VT_ACCENT_SEL_BG_D,
        VT_ACCENT_SEL_RING_D,
    ];
    for (i, w) in light.iter().enumerate() {
        assert_eq!(pal::luna_color(i as i32), *w);
    }
    // SAFETY: bascule du thème, le mutex est tenu.
    unsafe { pal::ffi::luna_theme_set(LUNA_THEME_DARK) };
    for (i, w) in dark.iter().enumerate() {
        assert_eq!(pal::luna_color(i as i32), *w);
    }
    assert_eq!(pal::luna_color(-1), pal::luna_color(LUNA_COLOR_TEXT));
    assert_eq!(pal::luna_color(99), pal::luna_color(LUNA_COLOR_TEXT));
    assert_eq!(pal::luna_color(i32::MAX), pal::luna_color(LUNA_COLOR_TEXT));
    // SAFETY: luna_color_selection ne déréférence rien.
    assert_eq!(unsafe { pal::ffi::luna_color_selection() }, 0xff0f_7d83);
    // SAFETY: thème ramené au clair sous le mutex.
    unsafe { pal::ffi::luna_theme_set(99) };
    // SAFETY: lecture du thème, le mutex est tenu.
    assert_eq!(unsafe { pal::ffi::luna_theme_get() }, LUNA_THEME_LIGHT);
    // SAFETY: lecture de la sélection, le mutex est tenu.
    assert_eq!(unsafe { pal::ffi::luna_color_selection() }, VT_ACCENT);
    let m = met_now();
    assert_eq!(m.caption_h, 50);
    assert_eq!(m.btn_w, 28);
    assert_eq!(m.btn_h, 24);
    assert_eq!(m.taskbar_h, 80);
    assert_eq!(m.start_w, 42);
    assert_eq!(m.scroll_w, 17);
    assert_eq!(m.icon_large, 56);
    assert_eq!(m.icon_small, 24);
    assert_eq!(det_now().check_size, 13);
    assert_eq!(det_now().title_radius, 24);
    assert_eq!(det_now().resize, 6);
    assert_eq!(det_now().thumb_min, 8);
    // SAFETY: NULL est le cas no-op du C.
    unsafe { pal::ffi::luna_metrics(core::ptr::null_mut()) };
    assert_eq!(
        luna_hit_test(core::ptr::null(), t_point { x: 0, y: 0 }),
        HT_NOWHERE
    );
    let z = luna_window_client(core::ptr::null());
    assert_eq!((z.x, z.y, z.w, z.h), (0, 0, 0, 0));
}

#[test]
fn placement_lanceur() {
    let cases = [
        (1280, 800, 80, 540, 289, 431),
        (1024, 768, 80, 540, 289, 399),
        (564, 600, 80, 540, 289, 231),
        (563, 600, 80, 539, 289, 231),
        (552, 600, 80, 528, 289, 231),
        (344, 600, 80, 320, 289, 231),
        (24, 600, 80, 0, 289, 231),
        (0, 600, 80, 0, 289, 231),
        (-5, 600, 80, 0, 289, 231),
        (800, 369, 80, 540, 289, 0),
        (800, 368, 80, 540, 288, 0),
        (800, 80, 80, 540, 0, 0),
        (800, 50, 80, 540, 0, 0),
        (800, 600, -10, 540, 289, 311),
    ];
    for (sw, sh, bar, w, h, y) in cases {
        let r = luna_launcher_place_in(sw, sh, bar);
        assert_eq!(r.x, LUNA_LAUNCHER_MARGIN_X);
        assert_eq!(r.w, w);
        assert_eq!(r.h, h);
        assert_eq!(r.y, y);
    }
    let wide = luna_launcher_place_in(1280, 800, 80);
    let place = luna_launcher_place(800, 80);
    assert_eq!(wide.x, place.x);
    assert_eq!(wide.y, place.y);
    assert_eq!(place.w, LUNA_LAUNCHER_WIDTH);
    assert_eq!(place.h, LUNA_LAUNCHER_HEIGHT);
}

#[test]
fn geometrie_lanceur() {
    let mut lay = crate::abi::t_launcherlayout {
        panel: lp_rect(0, 0, 0, 0),
        search: lp_rect(0, 0, 0, 0),
        title: lp_rect(0, 0, 0, 0),
        tile: [lp_rect(0, 0, 0, 0); 4],
        sep: lp_rect(0, 0, 0, 0),
        avatar: lp_rect(0, 0, 0, 0),
        user: lp_rect(0, 0, 0, 0),
        logoff: lp_rect(0, 0, 0, 0),
        poweroff: lp_rect(0, 0, 0, 0),
        ntiles: 0,
    };
    luna_launcher_layout(lp_rect(12, 20, 540, 289), 4, &mut lay);
    assert_eq!(lay.tile[0], lp_rect(30, 130, 121, 96));
    assert_eq!(lay.tile[1], lp_rect(157, 130, 122, 96));
    assert_eq!(lay.logoff, lp_rect(343, 255, 135, 36));
    assert_eq!(lay.poweroff, lp_rect(488, 255, 36, 36));
    assert_eq!(luna_launcher_hit(&lay, 30, 130), 0);
    assert_eq!(luna_launcher_hit(&lay, 343, 255), LUNA_LAUNCHER_LOGOFF);
    assert_eq!(luna_launcher_hit(&lay, 488, 255), LUNA_LAUNCHER_POWER);
    assert_eq!(luna_launcher_hit(&lay, 0, 0), LUNA_LAUNCHER_NONE);
    assert_eq!(
        luna_launcher_hit(core::ptr::null(), 0, 0),
        LUNA_LAUNCHER_NONE
    );
    assert_eq!(luna_launcher_tab_rect(&lay, 0), lp_rect(30, 100, 100, 24));
    assert_eq!(luna_launcher_row_rect(&lay, 0), lp_rect(30, 130, 497, 26));
}

#[test]
fn scores_recherche() {
    assert_eq!(luna_lm_match(c(b"Calculatrice\0"), c(b"cal\0")), 3);
    assert_eq!(luna_lm_match(c(b"Mon Navigateur\0"), c(b"nav\0")), 2);
    assert_eq!(luna_lm_match(c(b"Calculatrice\0"), c(b"cul\0")), 1);
    assert_eq!(luna_lm_match(c(b"Calculatrice\0"), c(b"xyz\0")), 0);
    assert_eq!(luna_lm_match(c(b"Calculatrice\0"), c(b"\0")), 1);
    assert_eq!(luna_lm_match(c(b"calculatrice\0"), c(b"CAL\0")), 3);
    assert_eq!(luna_lm_match(c(b"\xc3\x89diteur\0"), c(b"edi\0")), 3);
    assert_eq!(luna_lm_match(c(b"Editeur\0"), c(b"\xc3\xa9di\0")), 3);
    assert_eq!(luna_lm_match(c(b"Ab\0"), c(b"abc\0")), 0);
    assert_eq!(luna_lm_match(core::ptr::null(), c(b"a\0")), 0);
}

#[test]
fn filtre_utf8_recents() {
    let mut m = t_lmodel {
        query: [0; LUNA_LM_QUERY_MAX],
        qlen: 0,
        apps: [app(b"", 0, 0); LUNA_LM_APPS_MAX as usize],
        napps: 0,
        recent: [0; 4],
        nrecent: 0,
        shown: [0; LUNA_LM_APPS_MAX as usize],
        nshown: 0,
        tab: 0,
        top: 0,
        sel: 0,
        hot: 0,
    };
    garnir(&mut m);
    assert_eq!(luna_lm_type(&mut m, b'c' as u32), 1);
    assert_eq!(luna_lm_view(&m), LUNA_LV_SEARCH);
    assert_eq!(m.nshown, 3);
    assert_eq!(m.apps[m.shown[0] as usize].id, 2);
    assert_eq!(m.apps[m.shown[1] as usize].id, 3);
    assert_eq!(luna_lm_type(&mut m, b'o' as u32), 1);
    assert_eq!(m.nshown, 1);
    assert_eq!(m.sel, 0);
    assert_eq!(luna_lm_backspace(&mut m), 1);
    assert_eq!(m.nshown, 3);
    assert_eq!(luna_lm_backspace(&mut m), 1);
    assert_eq!(luna_lm_view(&m), LUNA_LV_PINNED);
    assert_eq!(luna_lm_backspace(&mut m), 0);

    garnir(&mut m);
    assert_eq!(luna_lm_type(&mut m, 0xe9), 1);
    assert_eq!(m.qlen, 2);
    assert_eq!(m.nshown, 6);
    assert_eq!(luna_lm_backspace(&mut m), 1);
    assert_eq!(m.qlen, 0);
    assert_eq!(luna_lm_type(&mut m, 0x20ac), 1);
    assert_eq!(m.qlen, 3);
    assert_eq!(luna_lm_type(&mut m, 0x1f600), 1);
    assert_eq!(m.qlen, 7);
    luna_lm_clear_query(&mut m);
    assert_eq!(m.qlen, 0);

    garnir(&mut m);
    assert_eq!(luna_lm_type(&mut m, 0x07), 0);
    assert_eq!(luna_lm_type(&mut m, 0x7f), 0);
    assert_eq!(luna_lm_type(&mut m, 0x85), 0);
    assert_eq!(luna_lm_type(&mut m, 0xd800), 0);
    assert_eq!(luna_lm_type(&mut m, 0x110000), 0);
    assert_eq!(luna_lm_type(core::ptr::null_mut(), b'a' as u32), 0);
    let mut i = 0;
    while i < LUNA_LM_QUERY_MAX + 8 {
        luna_lm_type(&mut m, b'a' as u32);
        i += 1;
    }
    assert_eq!(m.qlen, (LUNA_LM_QUERY_MAX - 1) as u32);
    assert_eq!(m.query[m.qlen as usize], 0);
    assert!(luna_lm_set_apps(&mut m, core::ptr::null(), LUNA_LM_APPS_MAX + 1) < 0);

    garnir(&mut m);
    luna_lm_set_tab(&mut m, LUNA_LV_RECENT);
    assert_eq!(m.nshown, 0);
    luna_lm_touch(&mut m, 3);
    luna_lm_touch(&mut m, 1);
    luna_lm_touch(&mut m, 3);
    assert_eq!(m.nrecent, 2);
    assert_eq!(m.apps[m.shown[0] as usize].id, 3);
    assert_eq!(m.apps[m.shown[1] as usize].id, 1);
    luna_lm_touch(&mut m, 0);
    luna_lm_touch(&mut m, 2);
    luna_lm_touch(&mut m, 5);
    assert_eq!(m.nrecent, LUNA_LM_RECENTS);
    assert_eq!(m.recent[0], 5);
    assert_eq!(m.recent[3], 3);
    luna_lm_touch(&mut m, 4242);
    assert_eq!(m.nshown, 3);

    garnir(&mut m);
    luna_lm_set_tab(&mut m, LUNA_LV_ALL);
    assert_eq!(m.nshown, 7);
    let want = [0u32, 2, 3, 1, 1000, 4, 5];
    let mut i = 0;
    while i < 7 {
        assert_eq!(m.apps[m.shown[i] as usize].id, want[i]);
        i += 1;
    }
    luna_lm_set_tab(&mut m, 99);
    assert_eq!(m.tab, LUNA_LV_ALL);
    luna_lm_set_tab(&mut m, -5);
    assert_eq!(m.tab, LUNA_LV_PINNED);

    garnir(&mut m);
    luna_lm_set_tab(&mut m, LUNA_LV_ALL);
    assert_eq!(luna_lm_move(&mut m, 1), 1);
    luna_lm_move(&mut m, 3);
    assert_eq!(m.sel, 4);
    assert_eq!(m.top, 1);
    luna_lm_move(&mut m, 100);
    assert_eq!(m.sel, 6);
    assert_eq!(m.top, 3);
    luna_lm_move(&mut m, -100);
    assert_eq!(m.sel, 0);
    assert_eq!(m.top, 0);
    assert_eq!(luna_lm_scroll(&mut m, 50), 3);
    assert_eq!(luna_lm_scroll(&mut m, -50), 0);
    let mut id = 0u32;
    assert_eq!(luna_lm_selected(&m, &mut id), 1);
    assert_eq!(id, 0);
    assert_eq!(luna_lm_select(&mut m, 2), 2);
    assert!(luna_lm_select(&mut m, LUNA_LM_ROWS) < 0);
    assert!(!luna_lm_row(&m, 0).is_null());
    assert!(luna_lm_row(&m, 9).is_null());

    luna_lm_init(&mut m);
    luna_lm_set_tab(&mut m, LUNA_LV_ALL);
    assert_eq!(luna_lm_move(&mut m, 1), LUNA_LM_NONE);
    assert_eq!(luna_lm_selected(&m, &mut id), 0);
    assert!(luna_lm_row(&m, 0).is_null());
    luna_lm_type(&mut m, b'z' as u32);
    assert_eq!(m.nshown, 0);
}

#[test]
fn barres() {
    let sb = t_lunascroll {
        r: lp_rect(0, 0, 17, 100),
        total: 100,
        page: 20,
        pos: 0,
        up: 0,
        down: 0,
        thumb: 0,
    };
    let mut parts = crate::abi::t_scrollparts {
        up: lp_rect(0, 0, 0, 0),
        down: lp_rect(0, 0, 0, 0),
        track: lp_rect(0, 0, 0, 0),
        thumb: lp_rect(0, 0, 0, 0),
    };
    crate::ctrl::luna_scroll_layout(&sb, &mut parts);
    assert_eq!(parts.up, lp_rect(0, 0, 17, 17));
    assert_eq!(parts.down, lp_rect(0, 83, 17, 17));
    assert_eq!(parts.track, lp_rect(0, 17, 17, 66));
    assert_eq!(parts.thumb, lp_rect(0, 17, 17, 13));

    let mut bar = t_taskbarlayout {
        start: lp_rect(0, 0, 0, 0),
        tasks: lp_rect(0, 0, 0, 0),
        tray: lp_rect(0, 0, 0, 0),
    };
    crate::dock::luna_taskbar_layout(lp_rect(0, 0, 800, 80), &mut bar);
    assert_eq!(bar.start, lp_rect(20, 19, 42, 42));
    assert_eq!(bar.tray, lp_rect(559, 19, 221, 42));
    assert_eq!(bar.tasks, lp_rect(74, 20, 477, 40));

    let mut g = t_traygeo {
        lang: lp_rect(0, 0, 0, 0),
        dots: lp_rect(0, 0, 0, 0),
        clock: lp_rect(0, 0, 0, 0),
        badge: lp_rect(0, 0, 0, 0),
    };
    crate::dock::luna_tray_geo(lp_rect(0, 0, 221, 42), &mut g);
    assert_eq!(g.badge, lp_rect(185, 6, 30, 30));
    assert_eq!(g.clock, lp_rect(97, 0, 74, 42));
    assert_eq!(g.dots, lp_rect(45, 0, 38, 42));
    assert_eq!(g.lang, lp_rect(14, 0, 17, 42));
}

fn surface(buf: &mut [u32], w: i32, h: i32) -> t_surface {
    t_surface {
        px: buf.as_mut_ptr(),
        w,
        h,
        stride: w,
        clip: lp_rect(0, 0, w, h),
    }
}

#[test]
fn pixels_controles() {
    let _t = theme();
    let mut px = [0u32; 60 * 40];
    let mut s = surface(&mut px, 60, 40);
    gfx_fill(&mut s, s.clip, LC_BLACK);
    crate::ctrl::luna_checkbox(&mut s, t_point { x: 20, y: 10 }, LS_NORMAL, true);
    crate::ctrl::luna_radio(&mut s, t_point { x: 40, y: 10 }, LS_NORMAL, true);
    assert_ne!(gfx_get(&s, t_point { x: 26, y: 16 }), LC_BLACK);
    assert_ne!(gfx_get(&s, t_point { x: 46, y: 16 }), LC_BLACK);
    assert_eq!(gfx_get(&s, t_point { x: 33, y: 16 }), 0xff00_0000);
    assert_eq!(gfx_get(&s, t_point { x: 19, y: 16 }), 0xff00_0000);
    assert_eq!(gfx_get(&s, t_point { x: 53, y: 16 }), 0xff00_0000);

    let mut px = [0u32; 100 * 60];
    let mut s = surface(&mut px, 100, 60);
    gfx_fill(&mut s, s.clip, LC_WHITE);
    let mut b = t_lunabtn {
        r: lp_rect(20, 14, 60, 32),
        state: LS_NORMAL,
        is_default: false,
        focus: true,
    };
    crate::ctrl::luna_button(&mut s, &b);
    assert_eq!(gfx_get(&s, t_point { x: 16, y: 30 }), VT_ACCENT);
    assert_eq!(gfx_get(&s, t_point { x: 17, y: 30 }), VT_ACCENT);
    assert_eq!(gfx_get(&s, t_point { x: 18, y: 30 }), LC_WHITE);
    assert_eq!(gfx_get(&s, t_point { x: 82, y: 30 }), VT_ACCENT);
    assert_eq!(gfx_get(&s, t_point { x: 50, y: 10 }), VT_ACCENT);
    assert_eq!(gfx_get(&s, t_point { x: 50, y: 49 }), VT_ACCENT);
    gfx_fill(&mut s, s.clip, LC_WHITE);
    b.focus = false;
    crate::ctrl::luna_button(&mut s, &b);
    assert_eq!(gfx_get(&s, t_point { x: 16, y: 30 }), LC_WHITE);
}
