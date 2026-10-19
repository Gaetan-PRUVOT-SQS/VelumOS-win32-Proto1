use core::ffi::c_int;

use crate::abi::{
    CN_CLICKED, CT_PROGRESS, CT_RADIO, CTL_BOX, CTL_CHECKED, CTL_DEFAULT, CTL_GAP, CTL_NOBACK,
    CTL_PAD, CTL_RADIO_BOX, CTL_RING_W, CTL_ST_PRESSED, INP_KEY_DOWN, INPM_ALT, INPM_CTRL,
    INPM_REPEAT, LS_DISABLED, LS_HOT, LS_PRESSED, LUNA_FOCUS_MARGIN, VK_RETURN, VK_SPACE,
    VT_ACCENT, VT_ACCENT_HI, VT_OPAQUE_CONTENT, VT_OPAQUE_CONTENT_D, t_ctl, t_ctlroot, t_ctltext,
    t_inpevent, t_lunabtn, t_mouseev, t_point, t_progress, t_rect, t_surface,
};
use crate::color::ctl_color_selected_text;
use crate::ffi::{
    gfx_fill, gfx_put, luna_button, luna_button_ink, luna_checkbox, luna_color_accent,
    luna_color_window, luna_groupbox, luna_progress, luna_theme_get, luna_transparency_reduced,
    rect_intersect, rect_make,
};
use crate::notify::{ctl_activate, ctl_notify};
use crate::text::{ctl_font_of, ctl_lstate, ctl_text_draw, ctl_text_init, ctl_text_width};
use crate::tree::{ctl_dirty_add, ctl_in_tree};

const RAD_SUB: i32 = 4;
const RAD_EIGHTH: i32 = 8;
const RAD_RING_THIN: i32 = 12;
const RAD_RING_THICK: i32 = 32;
const RAD_BORDER_LIGHT: u32 = 0xff8a_93a8;
const RAD_BORDER_DARK: u32 = 0xff5d_667a;
const LUNA_THEME_DARK: i32 = crate::abi::LUNA_THEME_DARK;

/// # Safety
/// `c` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_btn_face(c: *const t_ctl) -> t_rect {
    // SAFETY: c est non nul.
    let r = unsafe { (*c).rect };
    if r.w <= 2 * LUNA_FOCUS_MARGIN || r.h <= 2 * LUNA_FOCUS_MARGIN {
        return r;
    }
    rect_make(
        r.x + LUNA_FOCUS_MARGIN,
        r.y + LUNA_FOCUS_MARGIN,
        r.w - 2 * LUNA_FOCUS_MARGIN,
        r.h - 2 * LUNA_FOCUS_MARGIN,
    )
}

/// # Safety
/// `r` et `c` sont non nuls. `r.surface` est vivant.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_btn_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c et r sont non nuls.
    let b = t_lunabtn {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        r: unsafe { ctl_btn_face(c) },
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        state: unsafe { ctl_lstate(c) },
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        is_default: unsafe { (*c).flags & CTL_DEFAULT != 0 },
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        focus: unsafe { (*r).focus == c },
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let surface = unsafe { (*r).surface };
    luna_button(surface, &b);
    let mut tx = t_ctltext {
        r#box: rect_make(0, 0, 0, 0),
        text: core::ptr::null(),
        flags: 0,
        color: 0,
        enabled: false,
        mnemonic: false,
        font: core::ptr::null(),
    };
    // SAFETY: tx est local, c est vivant.
    unsafe { ctl_text_init(&mut tx, c) };
    tx.r#box = rect_make(b.r.x + CTL_PAD, b.r.y, b.r.w - 2 * CTL_PAD, b.r.h);
    if tx.r#box.w < 0 {
        tx.r#box.w = 0;
    }
    tx.enabled = true;
    tx.color = luna_button_ink(&b);
    tx.flags |= crate::abi::CTL_ALIGN_CENTER;
    if b.is_default && b.state != LS_DISABLED {
        tx.flags |= crate::abi::CTL_BOLD;
    }
    if b.state == LS_PRESSED {
        tx.r#box.x += 1;
        tx.r#box.y += 1;
    }
    // SAFETY: surface et tx sont vivants.
    unsafe { ctl_text_draw(surface, &tx) };
}

/// # Safety
/// `r`, `c` et `ev` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_btn_key(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    ev: *const t_inpevent,
) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    if ev.r#type != INP_KEY_DOWN {
        return false;
    }
    if ev.code != VK_SPACE && ev.code != VK_RETURN {
        return false;
    }
    if ev.mods & (INPM_ALT | INPM_CTRL) != 0 {
        return false;
    }
    if ev.mods & INPM_REPEAT == 0 {
        // SAFETY: r et c sont vivants.
        unsafe { ctl_activate(r, c) };
    }
    true
}

/// # Safety
/// `r` et `c` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_btn_activate(r: *mut t_ctlroot, c: *mut t_ctl) -> bool {
    // SAFETY: r et c sont vivants.
    unsafe { ctl_notify(r, c, CN_CLICKED) };
    true
}

fn band(s: *mut t_surface, r: t_rect, col: u32) {
    let w = CTL_RING_W;
    let inn = r.h - 2 * w;
    gfx_fill(s, rect_make(r.x, r.y, r.w, w), col);
    gfx_fill(s, rect_make(r.x, r.y + r.h - w, r.w, w), col);
    gfx_fill(s, rect_make(r.x, r.y + w, w, inn), col);
    gfx_fill(s, rect_make(r.x + r.w - w, r.y + w, w, inn), col);
}

fn paint_ring(s: *mut t_surface, r: t_rect, col: u32) {
    if r.w <= 0 || r.h <= 0 {
        return;
    }
    if r.w <= 2 * CTL_RING_W || r.h <= 2 * CTL_RING_W {
        gfx_fill(s, r, col);
        return;
    }
    band(s, r, col);
}

/// # Safety
/// `s` est une surface vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_focus_ring(s: *mut t_surface, r: t_rect) {
    paint_ring(s, r, luna_color_accent());
}

/// # Safety
/// `s` est une surface vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_focus_ring_row(s: *mut t_surface, r: t_rect, sel: bool) {
    if sel {
        // SAFETY: pas de pointeur. La couleur est une constante.
        paint_ring(s, r, unsafe { ctl_color_selected_text() });
    } else {
        paint_ring(s, r, luna_color_accent());
    }
}

fn chk_ring(r: *mut t_ctlroot, c: *const t_ctl, tx: &t_ctltext) {
    // SAFETY: r et c sont non nuls.
    if unsafe { (*r).focus.cast_const() != c } {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let font = unsafe { ctl_font_of(c) };
    if font.is_null() {
        return;
    }
    let m = LUNA_FOCUS_MARGIN;
    // SAFETY: tx est vivant, font n'est pas nul.
    let width = unsafe { ctl_text_width(tx) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let height = unsafe { (*font).height };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    let ring = t_rect {
        x: tx.r#box.x - m,
        w: width + 2 * m,
        h: height + 2 * m,
        y: rect.y + (rect.h - (height + 2 * m)) / 2,
    };
    // SAFETY: la surface de r est vivante.
    unsafe { ctl_focus_ring((*r).surface, rect_intersect(ring, rect)) };
}

/// # Safety
/// `r` et `c` sont non nuls. La surface est vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_chk_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c est non nul.
    let on = unsafe { (*c).flags & CTL_CHECKED != 0 };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let ty = unsafe { (*c).r#type };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    let mut box_w = CTL_BOX;
    if ty == CT_RADIO {
        box_w = CTL_RADIO_BOX;
    }
    let p = t_point {
        x: rect.x,
        y: rect.y + (rect.h - box_w) / 2,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let surface = unsafe { (*r).surface };
    // SAFETY: c est vivant.
    let st = unsafe { ctl_lstate(c) };
    if ty == CT_RADIO {
        // SAFETY: surface est vivante.
        unsafe { ctl_radio_draw(surface, p, st, on) };
    } else {
        luna_checkbox(surface, p, st, on);
    }
    let mut tx = t_ctltext {
        r#box: rect_make(0, 0, 0, 0),
        text: core::ptr::null(),
        flags: 0,
        color: 0,
        enabled: false,
        mnemonic: false,
        font: core::ptr::null(),
    };
    // SAFETY: tx est local, c est vivant.
    unsafe { ctl_text_init(&mut tx, c) };
    tx.r#box.x += box_w + CTL_GAP;
    tx.r#box.w -= box_w + CTL_GAP;
    if tx.r#box.w < 0 {
        tx.r#box.w = 0;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_text_draw(surface, &tx) };
    chk_ring(r, c, &tx);
}

/// # Safety
/// `r`, `c` et `ev` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_chk_key(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    ev: *const t_inpevent,
) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    if ev.r#type != INP_KEY_DOWN || ev.code != VK_SPACE {
        return false;
    }
    if ev.mods & (INPM_ALT | INPM_CTRL) != 0 {
        return false;
    }
    if ev.mods & INPM_REPEAT == 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_activate(r, c) };
    }
    true
}

/// # Safety
/// `r` et `c` sont non nuls. `r` a une surface.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_chk_activate(r: *mut t_ctlroot, c: *mut t_ctl) -> bool {
    // SAFETY: c est non nul.
    unsafe { (*c).flags ^= CTL_CHECKED };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_notify(r, c, CN_CLICKED) };
    true
}

/// # Safety
/// `r` et `c` sont non nuls. La surface est vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_pnl_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c est non nul.
    if unsafe { (*c).flags & CTL_NOBACK != 0 } {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    gfx_fill(unsafe { (*r).surface }, rect, luna_color_window());
}

/// # Safety
/// `r` et `c` sont non nuls. La surface est vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lbl_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    let mut tx = t_ctltext {
        r#box: rect_make(0, 0, 0, 0),
        text: core::ptr::null(),
        flags: 0,
        color: 0,
        enabled: false,
        mnemonic: false,
        font: core::ptr::null(),
    };
    // SAFETY: tx est local, c est vivant.
    unsafe { ctl_text_init(&mut tx, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_text_draw((*r).surface, &tx) };
}

/// # Safety
/// `r` et `c` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lbl_activate(r: *mut t_ctlroot, c: *mut t_ctl) -> bool {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { crate::focus::ctl_focus_after(r, c) }
}

/// # Safety
/// `r` et `c` sont non nuls. La surface est vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_grp_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c est non nul.
    let font = unsafe { ctl_font_of(c) };
    if font.is_null() {
        return;
    }
    let mut tx = t_ctltext {
        r#box: rect_make(0, 0, 0, 0),
        text: core::ptr::null(),
        flags: 0,
        color: 0,
        enabled: false,
        mnemonic: false,
        font: core::ptr::null(),
    };
    // SAFETY: tx est local, c est vivant.
    unsafe { ctl_text_init(&mut tx, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mut w = unsafe { ctl_text_width(&tx) };
    if w > 0 {
        w += 2 * CTL_GAP;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let surface = unsafe { (*r).surface };
    luna_groupbox(surface, rect, w);
    tx.r#box.x += 2 * CTL_GAP;
    // SAFETY: font n'est pas nul.
    tx.r#box.h = unsafe { (*font).height };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_text_draw(surface, &tx) };
}

fn press_track(r: *mut t_ctlroot, c: *mut t_ctl, inside: bool) -> bool {
    // SAFETY: c est non nul.
    let was = unsafe { (*c).state & CTL_ST_PRESSED != 0 };
    if was != inside {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*c).state ^= CTL_ST_PRESSED };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
    true
}

fn press_end(r: *mut t_ctlroot, c: *mut t_ctl, inside: bool) -> bool {
    // SAFETY: c et r sont non nuls.
    let was = unsafe { (*c).state & CTL_ST_PRESSED != 0 };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*c).state &= !CTL_ST_PRESSED };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*r).capture = core::ptr::null_mut() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    if was && inside {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_activate(r, c) };
    }
    true
}

/// # Safety
/// `r`, `c` et `e` sont non nuls. `r` a une surface.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_press_mouse(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    e: *const t_mouseev,
) -> bool {
    // SAFETY: e est non nul.
    let kind = unsafe { (*e).kind };
    if kind == crate::abi::CTL_MEV_DOWN {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*r).capture = c };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*c).state |= CTL_ST_PRESSED };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
        return true;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*r).capture } != c {
        return false;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let at = unsafe { (*e).at };
    let inside = crate::ffi::rect_contains(rect, at);
    if kind == crate::abi::CTL_MEV_MOVE {
        return press_track(r, c, inside);
    }
    if kind == crate::abi::CTL_MEV_UP {
        return press_end(r, c, inside);
    }
    true
}

/// # Safety
/// `r` et `c` sont nuls ou vivants. `c`, s'il est une jauge, est dans l'arbre.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_progress_set(r: *mut t_ctlroot, c: *mut t_ctl, mut pct: u32) {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || c.is_null() || unsafe { (*r).root.is_null() } {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*c).r#type } != CT_PROGRESS || unsafe { !ctl_in_tree(r, c) } {
        return;
    }
    if pct > 100 {
        pct = 100;
    }
    // SAFETY: le privé d'une jauge est un t_progress.
    let same = unsafe { (*(*c).r#priv.cast::<t_progress>()).pct == pct };
    if same {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*(*c).r#priv.cast::<t_progress>()).pct = pct };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
}

/// # Safety
/// `r` et `c` sont non nuls. `c.priv` est un `t_progress`. La surface est vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_prg_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: le privé est un t_progress, c et r sont non nuls.
    let pct = unsafe { (*(*c).r#priv.cast::<t_progress>()).pct };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    luna_progress(unsafe { (*r).surface }, rect, pct);
}

fn rad_ring_color(st: i32, on: bool, dark: bool) -> u32 {
    let mut c = if dark {
        RAD_BORDER_DARK
    } else {
        RAD_BORDER_LIGHT
    };
    if on || st == LS_HOT {
        c = if dark { VT_ACCENT_HI } else { VT_ACCENT };
    }
    if st == LS_DISABLED {
        c = (c & 0x00ff_ffff) | 0x7f00_0000;
    }
    c
}

fn rad_cover(px: i32, py: i32, ring8: i32) -> (u32, u32) {
    let mut ring = 0u32;
    let mut inside = 0u32;
    let inn = (CTL_RADIO_BOX / 2) * RAD_EIGHTH - ring8;
    let mut i = 0i32;
    while i < RAD_SUB * RAD_SUB {
        let j = (px * RAD_SUB + i % RAD_SUB) * 2 + 1 - CTL_RADIO_BOX * 4;
        let mut d2 = (py * RAD_SUB + i / RAD_SUB) * 2 + 1 - CTL_RADIO_BOX * 4;
        d2 = j * j + d2 * d2;
        if d2 <= inn * inn {
            inside += 1;
        } else if d2 <= (CTL_RADIO_BOX * 4) * (CTL_RADIO_BOX * 4) {
            ring += 1;
        }
        i += 1;
    }
    (ring, inside)
}

fn rad_alpha(c: u32, cover: u32) -> u32 {
    let a = (c >> 24) * cover / ((RAD_SUB * RAD_SUB) as u32);
    (c & 0x00ff_ffff) | (a << 24)
}

/// # Safety
/// `s` est une surface vivante. `st` est un état Luna.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_radio_draw(s: *mut t_surface, p: t_point, st: c_int, on: bool) {
    let dark = luna_theme_get() == LUNA_THEME_DARK;
    let fill = if dark {
        VT_OPAQUE_CONTENT_D
    } else {
        VT_OPAQUE_CONTENT
    };
    let mut y = 0i32;
    while y < CTL_RADIO_BOX {
        let mut x = 0i32;
        while x < CTL_RADIO_BOX {
            let (ring, inside) = rad_cover(x, y, if on { RAD_RING_THICK } else { RAD_RING_THIN });
            let q = t_point {
                x: p.x + x,
                y: p.y + y,
            };
            if luna_transparency_reduced() && inside != 0 {
                gfx_put(s, q, rad_alpha(fill, inside));
            }
            if ring != 0 {
                gfx_put(s, q, rad_alpha(rad_ring_color(st, on, dark), ring));
            }
            x += 1;
        }
        y += 1;
    }
}
