use crate::abi::{
    LUNA_COLOR_TEXT, LUNA_COLORS, LUNA_THEME_DARK, LUNA_THEME_LIGHT, VT_ACCENT, VT_ACCENT_HI,
    VT_ACCENT_SEL_BG, VT_ACCENT_SEL_BG_D, VT_ACCENT_SEL_RING, VT_ACCENT_SEL_RING_D,
    VT_ACCENT_TEXT_DARK, VT_DANGER, VT_DANGER_D, VT_DIM, VT_DIM_D, VT_GLASS_CHIP, VT_GLASS_CHIP_D,
    VT_GLASS_CONTENT, VT_GLASS_CONTENT_D, VT_GLASS_CONTROL, VT_GLASS_CONTROL_D, VT_GLASS_DOCK,
    VT_GLASS_DOCK_D, VT_GLASS_FRAME, VT_GLASS_FRAME_D, VT_GLASS_INPUT, VT_GLASS_INPUT_D,
    VT_GLASS_ITEM_SEL, VT_GLASS_ITEM_SEL_D, VT_GLASS_PANEL, VT_GLASS_PANEL_D, VT_GLASS_WINDOW,
    VT_GLASS_WINDOW_D, VT_INK, VT_INK_2, VT_INK_2_D, VT_INK_D, VT_MUTED, VT_MUTED_D,
    VT_OPAQUE_CONTENT, VT_OPAQUE_CONTENT_D, VT_OPAQUE_PANEL, VT_OPAQUE_PANEL_D, VT_RIM_BOTTOM,
    VT_RIM_BOTTOM_D, VT_RIM_IN, VT_RIM_IN_D, VT_RIM_TOP, VT_RIM_TOP_D, VT_SAT_DARK_X10,
    VT_SAT_LIGHT_X10, VT_SAT_PANEL_DARK_X10, VT_SAT_PANEL_LIGHT_X10, VT_SHADOW_DOCK,
    VT_SHADOW_DOCK_D, VT_SHADOW_WIN, VT_SHADOW_WIN_D, VT_SPEC_TOP, VT_SPEC_TOP_D, VT_WALL_AMBER,
    VT_WALL_AMBER_D, VT_WALL_BASE, VT_WALL_BASE_D, VT_WALL_ROSE, VT_WALL_ROSE_D, VT_WALL_TEAL,
    VT_WALL_TEAL_D, VT_WALL_VIOLET, VT_WALL_VIOLET_D, t_color, t_lpal, t_lunadetail, t_lunametrics,
};
use core::mem::offset_of;

static G_METRICS: t_lunametrics = t_lunametrics {
    caption_h: 50,
    frame_w: 0,
    frame_bottom: 0,
    btn_w: 28,
    btn_h: 24,
    taskbar_h: 80,
    start_w: 42,
    tray_w: 190,
    scroll_w: 17,
    menu_item_h: 36,
    icon_small: 24,
    icon_large: 56,
};

static G_DETAIL: t_lunadetail = t_lunadetail {
    btn_gap: 2,
    btn_top: 13,
    btn_right: 13,
    btn_top_max: 13,
    btn_right_max: 13,
    sysmenu_w: 40,
    title_pad: 6,
    corner_len: 24,
    tool_caption_h: 36,
    tool_btn: 24,
    tool_btn_top: 6,
    tool_btn_right: 10,
    check_size: 13,
    group_line_y: 6,
    group_pad: 8,
    progress_block: 8,
    progress_gap: 2,
    menu_header_h: 54,
    menu_footer_h: 38,
    menu_foot_item_w: 150,
    menu_foot_icon: 24,
    avatar: 48,
    task_gap: 3,
    thumb_min: 8,
    title_radius: 24,
    resize: 6,
    icon_left: 14,
    caption_icon: 22,
};

static G_LIGHT: t_lpal = t_lpal {
    accent: VT_ACCENT,
    accent_hi: VT_ACCENT_HI,
    accent_text: VT_ACCENT,
    sel_bg: VT_ACCENT_SEL_BG,
    sel_ring: VT_ACCENT_SEL_RING,
    ink: VT_INK,
    ink_2: VT_INK_2,
    muted: VT_MUTED,
    dim: VT_DIM,
    danger: VT_DANGER,
    frame: VT_GLASS_FRAME,
    dock: VT_GLASS_DOCK,
    chip: VT_GLASS_CHIP,
    panel: VT_GLASS_PANEL,
    window: VT_GLASS_WINDOW,
    content: VT_GLASS_CONTENT,
    control: VT_GLASS_CONTROL,
    sel: VT_GLASS_ITEM_SEL,
    input: VT_GLASS_INPUT,
    hover: 0x66ff_ffff,
    press: VT_GLASS_ITEM_SEL,
    rim_top: VT_RIM_TOP,
    rim_in: VT_RIM_IN,
    rim_bottom: VT_RIM_BOTTOM,
    spec: VT_SPEC_TOP,
    shadow_win: VT_SHADOW_WIN,
    shadow_dock: VT_SHADOW_DOCK,
    opaque_panel: VT_OPAQUE_PANEL,
    opaque_content: VT_OPAQUE_CONTENT,
    wall: [
        VT_WALL_BASE,
        VT_WALL_TEAL,
        VT_WALL_VIOLET,
        VT_WALL_AMBER,
        VT_WALL_ROSE,
    ],
    sat: VT_SAT_LIGHT_X10,
    sat_panel: VT_SAT_PANEL_LIGHT_X10,
    veil_a: 191,
    veil_b: 140,
};

static G_DARK: t_lpal = t_lpal {
    accent: VT_ACCENT_HI,
    accent_hi: VT_ACCENT_HI,
    accent_text: VT_ACCENT_TEXT_DARK,
    sel_bg: VT_ACCENT_SEL_BG_D,
    sel_ring: VT_ACCENT_SEL_RING_D,
    ink: VT_INK_D,
    ink_2: VT_INK_2_D,
    muted: VT_MUTED_D,
    dim: VT_DIM_D,
    danger: VT_DANGER_D,
    frame: VT_GLASS_FRAME_D,
    dock: VT_GLASS_DOCK_D,
    chip: VT_GLASS_CHIP_D,
    panel: VT_GLASS_PANEL_D,
    window: VT_GLASS_WINDOW_D,
    content: VT_GLASS_CONTENT_D,
    control: VT_GLASS_CONTROL_D,
    sel: VT_GLASS_ITEM_SEL_D,
    input: VT_GLASS_INPUT_D,
    hover: 0x14ff_ffff,
    press: 0x24ff_ffff,
    rim_top: VT_RIM_TOP_D,
    rim_in: VT_RIM_IN_D,
    rim_bottom: VT_RIM_BOTTOM_D,
    spec: VT_SPEC_TOP_D,
    shadow_win: VT_SHADOW_WIN_D,
    shadow_dock: VT_SHADOW_DOCK_D,
    opaque_panel: VT_OPAQUE_PANEL_D,
    opaque_content: VT_OPAQUE_CONTENT_D,
    wall: [
        VT_WALL_BASE_D,
        VT_WALL_TEAL_D,
        VT_WALL_VIOLET_D,
        VT_WALL_AMBER_D,
        VT_WALL_ROSE_D,
    ],
    sat: VT_SAT_DARK_X10,
    sat_panel: VT_SAT_PANEL_DARK_X10,
    veil_a: 36,
    veil_b: 26,
};

static mut G_THEME: i32 = LUNA_THEME_LIGHT;
static mut G_REDUCED: bool = false;
static mut G_SCRATCH: *mut u32 = core::ptr::null_mut();
static mut G_SCRATCH_PX: usize = 0;

pub(crate) fn theme_get() -> i32 {
    // SAFETY: un seul fil, comme g_theme en C.
    unsafe { core::ptr::addr_of!(G_THEME).read() }
}

pub(crate) fn theme_set(v: i32) {
    // SAFETY: un seul fil, écriture d'un i32.
    unsafe { core::ptr::addr_of_mut!(G_THEME).write(v) }
}

pub(crate) fn reduced_get() -> bool {
    // SAFETY: un seul fil, comme g_reduced en C.
    unsafe { core::ptr::addr_of!(G_REDUCED).read() }
}

pub(crate) fn reduced_set(v: bool) {
    // SAFETY: un seul fil, écriture d'un bool.
    unsafe { core::ptr::addr_of_mut!(G_REDUCED).write(v) }
}

pub(crate) fn lp_pal() -> *const t_lpal {
    if theme_get() == LUNA_THEME_DARK {
        &raw const G_DARK
    } else {
        &raw const G_LIGHT
    }
}

pub(crate) fn lm_metrics() -> *const t_lunametrics {
    &raw const G_METRICS
}

pub(crate) fn lm_detail() -> *const t_lunadetail {
    &raw const G_DETAIL
}

pub(crate) fn luna_color(id: i32) -> t_color {
    let id = if !(0..LUNA_COLORS).contains(&id) {
        LUNA_COLOR_TEXT
    } else {
        id
    };
    const OFF: [usize; 12] = [
        offset_of!(t_lpal, ink),
        offset_of!(t_lpal, ink_2),
        offset_of!(t_lpal, muted),
        offset_of!(t_lpal, dim),
        offset_of!(t_lpal, accent),
        offset_of!(t_lpal, accent_hi),
        offset_of!(t_lpal, accent_text),
        offset_of!(t_lpal, danger),
        offset_of!(t_lpal, opaque_content),
        offset_of!(t_lpal, opaque_panel),
        offset_of!(t_lpal, sel_bg),
        offset_of!(t_lpal, sel_ring),
    ];
    let base = lp_pal().cast::<u8>();
    // SAFETY: OFF[id] vise un t_color dans la palette, comme g_off en C.
    unsafe { base.add(OFF[id as usize]).cast::<t_color>().read() }
}

pub(crate) fn lp_gscratch(px: usize, a: *mut *mut u32, b: *mut *mut u32) -> bool {
    // SAFETY: lecture des globaux scratch, un seul fil.
    let (buf, n) = unsafe {
        (
            core::ptr::addr_of!(G_SCRATCH).read(),
            core::ptr::addr_of!(G_SCRATCH_PX).read(),
        )
    };
    if buf.is_null() || px == 0 || px > n / 2 {
        return false;
    }
    // SAFETY: a et b non nuls, comme les &o->a du C.
    unsafe {
        *a = buf;
        *b = buf.add(px);
    }
    true
}

pub(crate) fn lp_dark() -> bool {
    theme_get() == LUNA_THEME_DARK
}

pub(crate) fn lp_tone_alpha(c: t_color, a: u32) -> t_color {
    (c & 0x00ff_ffff) | (a << 24)
}

pub(crate) fn lp_tone_hover() -> t_color {
    if lp_dark() { 0x14ff_ffff } else { 0x66ff_ffff }
}

pub(crate) fn lp_tone_press() -> t_color {
    if lp_dark() {
        0x24ff_ffff
    } else {
        3_019_898_879
    }
}

pub(crate) fn lp_tone_track() -> t_color {
    if lp_dark() { 0x1aff_ffff } else { 0x1f14_1824 }
}

pub(crate) fn ld_dark() -> bool {
    theme_get() == LUNA_THEME_DARK
}

pub(crate) fn ld_pick(light: t_color, dark: t_color) -> t_color {
    if ld_dark() { dark } else { light }
}

pub(crate) fn ld_accent() -> t_color {
    ld_pick(VT_ACCENT, VT_ACCENT_TEXT_DARK)
}

pub(crate) fn pal_now() -> t_lpal {
    // SAFETY: lp_pal vise G_LIGHT ou G_DARK, immuables.
    unsafe { *lp_pal() }
}

pub(crate) fn met_now() -> t_lunametrics {
    // SAFETY: métriques statiques.
    unsafe { *lm_metrics() }
}

pub(crate) fn det_now() -> t_lunadetail {
    // SAFETY: détail statique.
    unsafe { *lm_detail() }
}

pub(crate) mod ffi {
    use crate::abi::{
        LUNA_COLOR_ACCENT_HI, LUNA_COLOR_ACCENT_TEXT, LUNA_COLOR_DANGER, LUNA_COLOR_DIM,
        LUNA_COLOR_MUTED, LUNA_COLOR_PANEL, LUNA_COLOR_SELECTION_BG, LUNA_COLOR_SELECTION_RING,
        LUNA_COLOR_TEXT_2, LUNA_THEME_DARK, LUNA_THEME_LIGHT, t_color, t_lpal, t_lunadetail,
        t_lunametrics, t_lunatheme,
    };

    c_fn!(luna_metrics(out: *mut t_lunametrics) {
        if !out.is_null() {
            // SAFETY: out non nul, exclusive le temps de l'appel.
            unsafe { out.write(*super::lm_metrics()) };
        }
    });
    c_fn!(lm_metrics() -> *const t_lunametrics { super::lm_metrics() });
    c_fn!(lm_detail() -> *const t_lunadetail { super::lm_detail() });
    c_fn!(lp_pal() -> *const t_lpal { super::lp_pal() });
    c_fn!(luna_theme_set(theme: t_lunatheme) {
        if theme == LUNA_THEME_DARK {
            super::theme_set(LUNA_THEME_DARK);
        } else {
            super::theme_set(LUNA_THEME_LIGHT);
        }
    });
    c_fn!(luna_theme_get() -> t_lunatheme { super::theme_get() });
    c_fn!(luna_transparency_set(reduced: bool) { super::reduced_set(reduced) });
    c_fn!(luna_transparency_reduced() -> bool { super::reduced_get() });
    c_fn!(luna_color(id: i32) -> t_color { super::luna_color(id) });
    c_fn!(luna_color_text_2() -> t_color { super::luna_color(LUNA_COLOR_TEXT_2) });
    c_fn!(luna_color_muted() -> t_color { super::luna_color(LUNA_COLOR_MUTED) });
    c_fn!(luna_color_dim() -> t_color { super::luna_color(LUNA_COLOR_DIM) });
    c_fn!(luna_color_danger() -> t_color { super::luna_color(LUNA_COLOR_DANGER) });
    c_fn!(luna_color_accent() -> t_color { super::pal_accent() });
    c_fn!(luna_color_text() -> t_color { super::pal_ink() });
    c_fn!(luna_color_window() -> t_color { super::pal_window() });
    c_fn!(luna_color_selection() -> t_color { super::sel() });
    c_fn!(luna_color_accent_hi() -> t_color { super::luna_color(LUNA_COLOR_ACCENT_HI) });
    c_fn!(luna_color_accent_text() -> t_color { super::luna_color(LUNA_COLOR_ACCENT_TEXT) });
    c_fn!(luna_color_panel() -> t_color { super::luna_color(LUNA_COLOR_PANEL) });
    c_fn!(luna_color_selection_bg() -> t_color { super::luna_color(LUNA_COLOR_SELECTION_BG) });
    c_fn!(luna_color_selection_ring() -> t_color { super::luna_color(LUNA_COLOR_SELECTION_RING) });
    c_fn!(luna_glass_scratch(buf: *mut u32, px: usize) { super::scratch(buf, px) });
    c_fn!(lp_gscratch(px: usize, a: *mut *mut u32, b: *mut *mut u32) -> bool {
        super::lp_gscratch(px, a, b)
    });
    c_fn!(lp_dark() -> bool { super::lp_dark() });
    c_fn!(lp_tone_alpha(c: t_color, a: u32) -> t_color { super::lp_tone_alpha(c, a) });
    c_fn!(lp_tone_hover() -> t_color { super::lp_tone_hover() });
    c_fn!(lp_tone_press() -> t_color { super::lp_tone_press() });
    c_fn!(lp_tone_track() -> t_color { super::lp_tone_track() });
    c_fn!(ld_dark() -> bool { super::ld_dark() });
    c_fn!(ld_pick(light: t_color, dark: t_color) -> t_color { super::ld_pick(light, dark) });
    c_fn!(ld_accent() -> t_color { super::ld_accent() });
}

fn pal_accent() -> t_color {
    // SAFETY: palette immuable.
    unsafe { (*lp_pal()).accent }
}

fn pal_ink() -> t_color {
    // SAFETY: palette immuable.
    unsafe { (*lp_pal()).ink }
}

fn pal_window() -> t_color {
    // SAFETY: palette immuable.
    unsafe { (*lp_pal()).opaque_content }
}

fn sel() -> t_color {
    if theme_get() == LUNA_THEME_DARK {
        0xff0f_7d83
    } else {
        pal_accent()
    }
}

fn scratch(buf: *mut u32, mut px: usize) {
    if buf.is_null() {
        px = 0;
    }
    // SAFETY: un seul fil, comme g_scratch en C.
    unsafe {
        core::ptr::addr_of_mut!(G_SCRATCH).write(buf);
        core::ptr::addr_of_mut!(G_SCRATCH_PX).write(px);
    }
}
