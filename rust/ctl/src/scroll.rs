use core::ffi::c_int;

use crate::abi::{
    CN_SCROLL, CT_SCROLL, CTL_ARROW, CTL_MEV_DOWN, CTL_MEV_MOVE, CTL_MEV_UP, CTL_MEV_WHEEL,
    CTL_SBR_MOVED, CTL_SBR_NONE, CTL_SBR_USED, CTL_SBZ_DEC, CTL_SBZ_INC, CTL_SBZ_NONE,
    CTL_SBZ_PAGE_DEC, CTL_SBZ_PAGE_INC, CTL_SBZ_THUMB, CTL_SCROLL_MAX, CTL_THUMB_MIN, INP_KEY_DOWN,
    INPM_ALT, INPM_CTRL, LS_DISABLED, LS_NORMAL, LS_PRESSED, VK_DOWN, VK_END, VK_HOME, VK_LEFT,
    VK_NEXT, VK_PRIOR, VK_RIGHT, VK_UP, t_ctl, t_ctlroot, t_inpevent, t_lunabtn, t_lunametrics,
    t_mouseev, t_point, t_rect, t_sbctx, t_sblo, t_scrst, t_surface,
};
use crate::color::ctl_color_track;
use crate::ffi::{
    gfx_fill, gfx_hline, gfx_vline, luna_button, luna_button_ink, luna_metrics, rect_contains,
    rect_empty, rect_make,
};
use crate::mouse::ctl_wheel_delta;
use crate::notify::ctl_notify;
use crate::tree::{ctl_dirty_add, ctl_enabled, ctl_in_tree};
use crate::widgets::ctl_focus_ring;

fn axis_rect(bar: t_rect, vertical: bool, off: i32, len: i32) -> t_rect {
    if vertical {
        rect_make(bar.x, bar.y + off, bar.w, len)
    } else {
        rect_make(bar.x + off, bar.y, len, bar.h)
    }
}

/// # Safety
/// `r` est un rectangle passé par valeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_vertical(r: t_rect) -> bool {
    r.h >= r.w
}

/// # Safety
/// `r` est un rectangle passé par valeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_len(r: t_rect, vertical: bool) -> i32 {
    if vertical { r.h } else { r.w }
}

fn thumb_rect(lo: &t_sblo, st: &t_scrst) -> t_rect {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let span = i64::from(unsafe { ctl_sb_len(lo.track, lo.vertical) });
    if st.max <= 0 || span < i64::from(CTL_THUMB_MIN) {
        return rect_make(0, 0, 0, 0);
    }
    let mut len = span * i64::from(st.page) / (i64::from(st.max) + i64::from(st.page));
    if len < i64::from(CTL_THUMB_MIN) {
        len = i64::from(CTL_THUMB_MIN);
    }
    let off = (span - len) * i64::from(st.pos) / i64::from(st.max);
    axis_rect(lo.track, lo.vertical, off as i32, len as i32)
}

/// # Safety
/// `st` et `lo` sont non nuls. `lo` est inscriptible.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_layout(bar: t_rect, st: *const t_scrst, lo: *mut t_sblo) {
    let mut m = t_lunametrics {
        caption_h: 0,
        frame_w: 0,
        frame_bottom: 0,
        btn_w: 0,
        btn_h: 0,
        taskbar_h: 0,
        start_w: 0,
        tray_w: 0,
        scroll_w: 0,
        menu_item_h: 0,
        icon_small: 0,
        icon_large: 0,
    };
    luna_metrics(&mut m);
    // SAFETY: lo et st sont non nuls.
    let lo = unsafe { &mut *lo };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let st = unsafe { &*st };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    lo.vertical = unsafe { ctl_sb_vertical(bar) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let len = unsafe { ctl_sb_len(bar, lo.vertical) };
    let mut arrow = m.scroll_w;
    if arrow > len / 2 {
        arrow = len / 2;
    }
    lo.dec = axis_rect(bar, lo.vertical, 0, arrow);
    lo.inc = axis_rect(bar, lo.vertical, len - arrow, arrow);
    lo.track = axis_rect(bar, lo.vertical, arrow, len - 2 * arrow);
    lo.thumb = thumb_rect(lo, st);
}

/// # Safety
/// `p` est un point passé par valeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_axis(vertical: bool, p: t_point) -> i32 {
    if vertical { p.y } else { p.x }
}

/// # Safety
/// `r` est un rectangle passé par valeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_start(vertical: bool, r: t_rect) -> i32 {
    if vertical { r.y } else { r.x }
}

/// # Safety
/// `lo` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_zone(lo: *const t_sblo, p: t_point) -> u32 {
    // SAFETY: lo est non nul.
    let lo = unsafe { &*lo };
    if rect_contains(lo.dec, p) {
        return CTL_SBZ_DEC;
    }
    if rect_contains(lo.inc, p) {
        return CTL_SBZ_INC;
    }
    if rect_contains(lo.thumb, p) {
        return CTL_SBZ_THUMB;
    }
    if !rect_contains(lo.track, p) {
        return CTL_SBZ_NONE;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let at = unsafe { ctl_sb_axis(lo.vertical, p) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let start = unsafe { ctl_sb_start(lo.vertical, lo.thumb) };
    if at < start {
        CTL_SBZ_PAGE_DEC
    } else {
        CTL_SBZ_PAGE_INC
    }
}

/// # Safety
/// `lo` et `st` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_pos_from(lo: *const t_sblo, st: *const t_scrst, coord: i32) -> i32 {
    // SAFETY: les deux pointeurs sont non nuls.
    let lo = unsafe { &*lo };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let st = unsafe { &*st };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let span = i64::from(unsafe { ctl_sb_len(lo.track, lo.vertical) })
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        - i64::from(unsafe { ctl_sb_len(lo.thumb, lo.vertical) });
    if span <= 0 || st.max <= 0 {
        return 0;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rel = i64::from(coord) - i64::from(unsafe { ctl_sb_start(lo.vertical, lo.track) });
    if rel <= 0 {
        return 0;
    }
    if rel >= span {
        return st.max;
    }
    ((rel * i64::from(st.max) + span / 2) / span) as i32
}

/// # Safety
/// `st` est non nul et inscriptible.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_set(st: *mut t_scrst, mut pos: i32) -> bool {
    // SAFETY: st est non nul.
    let st = unsafe { &mut *st };
    if pos < 0 {
        pos = 0;
    }
    if pos > st.max {
        pos = st.max;
    }
    if pos == st.pos {
        return false;
    }
    st.pos = pos;
    true
}

fn step_of(zone: u32, page: i32) -> i32 {
    if zone == CTL_SBZ_DEC {
        return -1;
    }
    if zone == CTL_SBZ_INC {
        return 1;
    }
    if zone == CTL_SBZ_PAGE_DEC {
        return -page;
    }
    page
}

fn sb_down(x: &t_sbctx, lo: &t_sblo, p: t_point) -> c_int {
    // SAFETY: lo est vivant.
    let z = unsafe { ctl_sb_zone(lo, p) };
    if z == CTL_SBZ_NONE {
        return CTL_SBR_NONE;
    }
    // SAFETY: x.r, x.c et x.st sont non nuls pour un clic sur la barre.
    unsafe { (*x.r).capture = x.c };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*x.st).zone = z };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*x.c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(x.r, rect) };
    if z == CTL_SBZ_THUMB {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let at = unsafe { ctl_sb_axis(lo.vertical, p) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let start = unsafe { ctl_sb_start(lo.vertical, lo.thumb) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*x.st).grab = at - start };
        return CTL_SBR_USED;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let pos = unsafe { (*x.st).pos + step_of(z, (*x.st).page) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { ctl_sb_set(x.st, pos) } {
        return CTL_SBR_MOVED;
    }
    CTL_SBR_USED
}

fn sb_drag(x: &t_sbctx, lo: &t_sblo, p: t_point) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let at = unsafe { ctl_sb_axis(lo.vertical, p) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let grab = unsafe { (*x.st).grab };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let pos = unsafe { ctl_sb_pos_from(lo, x.st, at - grab) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !unsafe { ctl_sb_set(x.st, pos) } {
        return CTL_SBR_USED;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*x.c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(x.r, rect) };
    CTL_SBR_MOVED
}

fn sb_up(x: &t_sbctx) -> c_int {
    // SAFETY: x.st, x.r et x.c sont non nuls.
    unsafe { (*x.st).zone = CTL_SBZ_NONE };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*x.st).grab = 0 };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*x.r).capture = core::ptr::null_mut() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*x.c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(x.r, rect) };
    CTL_SBR_USED
}

/// # Safety
/// `x` et `e` sont non nuls. Les pointeurs de `x` sont vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_mouse(x: *const t_sbctx, e: *const t_mouseev) -> c_int {
    // SAFETY: x et e sont non nuls.
    let x = unsafe { &*x };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let e = unsafe { &*e };
    let mut lo = t_sblo {
        dec: rect_make(0, 0, 0, 0),
        inc: rect_make(0, 0, 0, 0),
        track: rect_make(0, 0, 0, 0),
        thumb: rect_make(0, 0, 0, 0),
        vertical: false,
    };
    // SAFETY: st est vivant, lo est local.
    unsafe { ctl_sb_layout(x.bar, x.st, &mut lo) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mine = unsafe { (*x.r).capture == x.c };
    if e.kind == CTL_MEV_DOWN {
        return sb_down(x, &lo, e.at);
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if e.kind == CTL_MEV_MOVE && mine && unsafe { (*x.st).zone } == CTL_SBZ_THUMB {
        return sb_drag(x, &lo, e.at);
    }
    if e.kind == CTL_MEV_UP && mine {
        return sb_up(x);
    }
    CTL_SBR_NONE
}

fn tri_v(s: *mut t_surface, apex: t_point, dy: i32, col: u32) {
    let mut i = 0i32;
    while i < CTL_ARROW {
        gfx_hline(
            s,
            t_point {
                x: apex.x - i,
                y: apex.y + i * dy,
            },
            2 * i + 1,
            col,
        );
        i += 1;
    }
}

fn tri_h(s: *mut t_surface, apex: t_point, dx: i32, col: u32) {
    let mut i = 0i32;
    while i < CTL_ARROW {
        gfx_vline(
            s,
            t_point {
                x: apex.x + i * dx,
                y: apex.y - i,
            },
            2 * i + 1,
            col,
        );
        i += 1;
    }
}

/// # Safety
/// `s` est une surface vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_arrow(s: *mut t_surface, r: t_rect, dir: c_int, col: u32) {
    let mid = t_point {
        x: r.x + r.w / 2,
        y: r.y + r.h / 2,
    };
    if dir == 0 {
        tri_v(
            s,
            t_point {
                x: mid.x,
                y: mid.y - CTL_ARROW / 2,
            },
            1,
            col,
        );
    } else if dir == 1 {
        tri_v(
            s,
            t_point {
                x: mid.x,
                y: mid.y + CTL_ARROW / 2 - 1,
            },
            -1,
            col,
        );
    } else if dir == 2 {
        tri_h(
            s,
            t_point {
                x: mid.x - CTL_ARROW / 2,
                y: mid.y,
            },
            1,
            col,
        );
    } else {
        tri_h(
            s,
            t_point {
                x: mid.x + CTL_ARROW / 2 - 1,
                y: mid.y,
            },
            -1,
            col,
        );
    }
}

fn paint_part(s: *mut t_surface, r: t_rect, st: i32) -> u32 {
    let b = t_lunabtn {
        r,
        state: st,
        is_default: false,
        focus: false,
    };
    luna_button(s, &b);
    luna_button_ink(&b)
}

fn part_state(st: &t_scrst, zone: u32, live: bool) -> i32 {
    if !live {
        return LS_DISABLED;
    }
    if st.zone == zone {
        LS_PRESSED
    } else {
        LS_NORMAL
    }
}

fn step_button(s: *mut t_surface, mut r: t_rect, dir: c_int, state: i32) {
    let ink = paint_part(s, r, state);
    if state == LS_PRESSED {
        r.x += 1;
        r.y += 1;
    }
    // SAFETY: s est la surface du défilement.
    unsafe { ctl_sb_arrow(s, r, dir, ink) };
}

fn arrow_dir(vertical: bool, inc: bool) -> c_int {
    let base = if vertical { 0 } else { 2 };
    if inc { base + 1 } else { base }
}

/// # Safety
/// `s` est une surface vivante. `st` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_sb_paint(
    s: *mut t_surface,
    bar: t_rect,
    st: *const t_scrst,
    en: bool,
) {
    let mut lo = t_sblo {
        dec: rect_make(0, 0, 0, 0),
        inc: rect_make(0, 0, 0, 0),
        track: rect_make(0, 0, 0, 0),
        thumb: rect_make(0, 0, 0, 0),
        vertical: false,
    };
    // SAFETY: st est non nul, lo est local.
    unsafe { ctl_sb_layout(bar, st, &mut lo) };
    // SAFETY: st est non nul.
    let st = unsafe { &*st };
    let live = en && st.max > 0;
    // SAFETY: pas de pointeur au-delà de la couleur.
    gfx_fill(s, lo.track, unsafe { ctl_color_track() });
    step_button(
        s,
        lo.dec,
        arrow_dir(lo.vertical, false),
        part_state(st, CTL_SBZ_DEC, live),
    );
    step_button(
        s,
        lo.inc,
        arrow_dir(lo.vertical, true),
        part_state(st, CTL_SBZ_INC, live),
    );
    if !rect_empty(lo.thumb) {
        paint_part(s, lo.thumb, part_state(st, CTL_SBZ_THUMB, live));
    }
}

/// # Safety
/// `c` est non nul. `c.priv` est un `t_scrst` mis à zéro.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_scr_init(c: *mut t_ctl) {
    // SAFETY: le privé d'un défilement est un t_scrst.
    unsafe { (*(*c).r#priv.cast::<t_scrst>()).page = 1 };
}

/// # Safety
/// `r` et `c` sont non nuls. La surface est vivante. `c.priv` est un `t_scrst`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_scr_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c et r sont non nuls.
    let en = unsafe { ctl_enabled(c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let st = unsafe { (*c).r#priv.cast::<t_scrst>() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_paint((*r).surface, rect, st, en) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*r).focus } != c {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_focus_ring((*r).surface, rect) };
}

fn scr_apply(r: *mut t_ctlroot, c: *mut t_ctl, pos: i32) -> bool {
    // SAFETY: c.priv est un t_scrst.
    let st = unsafe { (*c).r#priv.cast::<t_scrst>() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { ctl_sb_set(st, pos) } {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_notify(r, c, CN_SCROLL) };
    }
    true
}

/// # Safety
/// `r`, `c` et `e` sont non nuls. `c.priv` est un `t_scrst`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_scr_mouse(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    e: *const t_mouseev,
) -> bool {
    // SAFETY: e est non nul, le privé est un t_scrst.
    let st = unsafe { (*c).r#priv.cast::<t_scrst>() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*e).kind } == CTL_MEV_WHEEL {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let pos = unsafe { (*st).pos - ctl_wheel_delta((*e).wheel) };
        return scr_apply(r, c, pos);
    }
    let x = t_sbctx {
        r,
        c,
        st,
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        bar: unsafe { (*c).rect },
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let res = unsafe { ctl_sb_mouse(&x, e) };
    if res == CTL_SBR_MOVED {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_notify(r, c, CN_SCROLL) };
    }
    res != CTL_SBR_NONE
}

/// # Safety
/// `r`, `c` et `ev` sont non nuls. `c.priv` est un `t_scrst`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_scr_key(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    ev: *const t_inpevent,
) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    if ev.r#type != INP_KEY_DOWN || ev.mods & (INPM_ALT | INPM_CTRL) != 0 {
        return false;
    }
    // SAFETY: le privé est un t_scrst. La copie meurt avant scr_apply.
    let (pos, page, max) = unsafe {
        let st = &*(*c).r#priv.cast::<t_scrst>();
        (st.pos, st.page, st.max)
    };
    if ev.code == VK_UP || ev.code == VK_LEFT {
        return scr_apply(r, c, pos - 1);
    }
    if ev.code == VK_DOWN || ev.code == VK_RIGHT {
        return scr_apply(r, c, pos + 1);
    }
    if ev.code == VK_PRIOR {
        return scr_apply(r, c, pos - page);
    }
    if ev.code == VK_NEXT {
        return scr_apply(r, c, pos + page);
    }
    if ev.code == VK_HOME {
        return scr_apply(r, c, 0);
    }
    if ev.code == VK_END {
        return scr_apply(r, c, max);
    }
    false
}

fn scroll_of(r: *mut t_ctlroot, c: *mut t_ctl) -> *mut t_scrst {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || c.is_null() || unsafe { (*r).root.is_null() } {
        return core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*c).r#type } != CT_SCROLL || unsafe { !ctl_in_tree(r, c) } {
        return core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*c).r#priv.cast() }
}

/// # Safety
/// `r` et `c` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_scroll_set(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    mut max: c_int,
    mut page: c_int,
) {
    let st = scroll_of(r, c);
    if st.is_null() {
        return;
    }
    // SAFETY: st est le privé du défilement.
    let old = unsafe { *st };
    max = max.clamp(0, CTL_SCROLL_MAX);
    page = page.clamp(1, CTL_SCROLL_MAX);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*st).max = max };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*st).page = page };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_set(st, (*st).pos) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if old.max != unsafe { (*st).max }
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        || old.page != unsafe { (*st).page }
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        || old.pos != unsafe { (*st).pos }
    {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
}

/// # Safety
/// `c` est nul, ou son privé est un `t_scrst` s'il est un défilement.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_scroll_pos(c: *const t_ctl) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if c.is_null() || unsafe { (*c).r#type } != CT_SCROLL {
        return 0;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*(*c).r#priv.cast::<t_scrst>()).pos }
}

/// # Safety
/// `r` et `c` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_scroll_move(r: *mut t_ctlroot, c: *mut t_ctl, pos: c_int) {
    let st = scroll_of(r, c);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !st.is_null() && unsafe { ctl_sb_set(st, pos) } {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
}
