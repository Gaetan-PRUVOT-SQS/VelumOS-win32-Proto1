use core::ffi::{c_char, c_int};

use crate::abi::{
    CN_CHANGED, CN_ENTER, CT_EDIT, CTL_EDIT_INSET, CTL_EDR_ENTER, CTL_EDR_TEXT, CTL_EDR_USED,
    CTL_EDR_VIEW, CTL_FRAME, CTL_MEV_DOWN, CTL_MEV_MOVE, CTL_MEV_UP, CTL_PASSWORD, CTL_TEXT_MAX,
    CTL_VK_A, CTL_VK_C, CTL_VK_V, CTL_VK_X, E_INVAL, INP_CHAR, INPM_ALT, INPM_CTRL, INPM_SHIFT,
    VK_BACK, VK_DELETE, VK_END, VK_HOME, VK_LEFT, VK_RETURN, VK_RIGHT, t_ctl, t_ctlroot, t_edit,
    t_edview, t_inpevent, t_mouseev, t_point, t_rect, t_surface, t_textreq,
};
use crate::clip::{ctl_clip_get, ctl_clip_set};
use crate::color::{ctl_color_back, ctl_color_selected_text, ctl_color_text};
use crate::ffi::{
    font_draw, font_text_width, gfx_fill, gfx_set_clip, gfx_vline, luna_color_selection,
    luna_edit_frame, rect_intersect, rect_make,
};
use crate::notify::ctl_notify;
use crate::raw::{fill_byte, scopy, scopy_distinct, slen};
use crate::text::{ctl_copy_text, ctl_font_of};
use crate::tree::{ctl_dirty_add, ctl_enabled, ctl_in_tree};
use crate::utf8::{ctl_u8_count, ctl_u8_encode, ctl_u8_len, ctl_u8_next, ctl_u8_prev};

fn text_of(c: *const t_ctl) -> *const c_char {
    // SAFETY: c est non nul. text est le tableau du contrôle.
    unsafe { (*c).text.as_ptr().cast() }
}

fn text_mut(c: *mut t_ctl) -> *mut c_char {
    // SAFETY: c est non nul. text est inscriptible.
    unsafe { (*c).text.as_mut_ptr().cast() }
}

fn empty_view() -> t_edview {
    t_edview {
        font: core::ptr::null(),
        disp: [0; CTL_TEXT_MAX],
        len: 0,
        org: t_point { x: 0, y: 0 },
        col: 0,
    }
}

/// # Safety
/// `e` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_has_sel(e: *const t_edit) -> bool {
    // SAFETY: e est non nul.
    let e = unsafe { &*e };
    e.caret != e.anchor
}

/// # Safety
/// `e` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_sel_from(e: *const t_edit) -> u32 {
    // SAFETY: e est non nul.
    let e = unsafe { &*e };
    if e.caret < e.anchor {
        e.caret
    } else {
        e.anchor
    }
}

/// # Safety
/// `e` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_sel_to(e: *const t_edit) -> u32 {
    // SAFETY: e est non nul.
    let e = unsafe { &*e };
    if e.caret > e.anchor {
        e.caret
    } else {
        e.anchor
    }
}

/// # Safety
/// `c` et `e` sont non nuls. `c.text` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_erase_sel(c: *mut t_ctl, e: *mut t_edit) -> bool {
    // SAFETY: e est non nul.
    if unsafe { !ctl_edt_has_sel(e) } {
        return false;
    }
    // SAFETY: e est non nul.
    let from = unsafe { ctl_edt_sel_from(e) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let to = unsafe { ctl_edt_sel_to(e) };
    let base = text_mut(c);
    let len = slen(base);
    let tail = len.wrapping_sub(to).wrapping_add(1) as usize;
    scopy(
        base.wrapping_add(from as usize).cast(),
        base.wrapping_add(to as usize).cast(),
        tail,
    );
    // SAFETY: e est non nul et inscriptible.
    unsafe {
        (*e).caret = from;
        (*e).anchor = from;
    }
    true
}

/// # Safety
/// `c` et `e` sont non nuls. `c.text` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_select_all(c: *mut t_ctl, e: *mut t_edit) {
    let n = slen(text_of(c));
    // SAFETY: e est non nul.
    unsafe {
        (*e).anchor = 0;
        (*e).caret = n;
    }
}

fn take(c: *const t_ctl, e: *const t_edit, s: *const c_char, n: u32) -> u32 {
    let text = text_of(c);
    let len = slen(text);
    let room = (CTL_TEXT_MAX as u32 - 1).wrapping_sub(len);
    // SAFETY: text couvre len octets.
    let mut chars = unsafe { ctl_u8_count(text, len) };
    // SAFETY: e est non nul.
    let max_chars = unsafe { (*e).max_chars };
    let mut pos = 0u32;
    while pos < n && chars < max_chars {
        // SAFETY: pos < n, s couvre n octets.
        let l = unsafe { ctl_u8_len(s.wrapping_add(pos as usize), n - pos) };
        if pos.wrapping_add(l) > room {
            break;
        }
        pos = pos.wrapping_add(l);
        chars = chars.wrapping_add(1);
    }
    pos
}

/// # Safety
/// `c` et `e` sont non nuls. `s` couvre `n` octets si `n` n'est pas nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_insert(
    c: *mut t_ctl,
    e: *mut t_edit,
    s: *const c_char,
    n: u32,
) -> bool {
    // SAFETY: c et e sont non nuls.
    let changed = unsafe { ctl_edt_erase_sel(c, e) };
    let k = take(c, e, s, n);
    if k == 0 {
        return changed;
    }
    // SAFETY: e est non nul.
    let caret = unsafe { (*e).caret };
    let base = text_mut(c);
    let len = slen(base);
    let tail = len.wrapping_sub(caret).wrapping_add(1) as usize;
    let at = caret as usize;
    scopy(
        base.wrapping_add(at + k as usize).cast(),
        base.wrapping_add(at).cast(),
        tail,
    );
    scopy_distinct(base.wrapping_add(at).cast(), s.cast(), k as usize);
    let next = caret.wrapping_add(k);
    // SAFETY: e est inscriptible.
    unsafe {
        (*e).caret = next;
        (*e).anchor = next;
    }
    true
}

/// # Safety
/// `c` et `e` sont non nuls. `c.text` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_erase_prev(c: *mut t_ctl, e: *mut t_edit) -> bool {
    // SAFETY: c et e sont non nuls.
    if unsafe { ctl_edt_erase_sel(c, e) } {
        return true;
    }
    // SAFETY: e est non nul.
    let caret = unsafe { (*e).caret };
    if caret == 0 {
        return false;
    }
    let base = text_mut(c);
    let len = slen(base);
    // SAFETY: caret > 0 et text couvre caret octets.
    let from = unsafe { ctl_u8_prev(base, caret) };
    let tail = len.wrapping_sub(caret).wrapping_add(1) as usize;
    scopy(
        base.wrapping_add(from as usize).cast(),
        base.wrapping_add(caret as usize).cast(),
        tail,
    );
    // SAFETY: e est inscriptible.
    unsafe {
        (*e).caret = from;
        (*e).anchor = from;
    }
    true
}

/// # Safety
/// `c` et `e` sont non nuls. `c.text` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_erase_next(c: *mut t_ctl, e: *mut t_edit) -> bool {
    // SAFETY: c et e sont non nuls.
    if unsafe { ctl_edt_erase_sel(c, e) } {
        return true;
    }
    let base = text_mut(c);
    let len = slen(base);
    // SAFETY: e est non nul.
    let caret = unsafe { (*e).caret };
    if caret >= len {
        return false;
    }
    // SAFETY: caret < len, text couvre len octets.
    let to = unsafe { ctl_u8_next(base, len, caret) };
    let tail = len.wrapping_sub(to).wrapping_add(1) as usize;
    scopy(
        base.wrapping_add(caret as usize).cast(),
        base.wrapping_add(to as usize).cast(),
        tail,
    );
    // SAFETY: e est inscriptible.
    unsafe { (*e).anchor = caret };
    true
}

/// # Safety
/// `c` est non nul. Son privé est un `t_edit` mis à zéro. `c.text` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_init(c: *mut t_ctl) {
    // SAFETY: le privé d'un edit est un t_edit.
    let e = unsafe { (*c).r#priv.cast::<t_edit>() };
    let n = slen(text_of(c));
    // SAFETY: e est le privé, aligné et inscriptible.
    unsafe {
        (*e).max_chars = CTL_TEXT_MAX as u32 - 1;
        (*e).caret = n;
        (*e).anchor = n;
    }
}

fn truncate_chars(c: *mut t_ctl, e: *mut t_edit) {
    let base = text_mut(c);
    let len = slen(base);
    // SAFETY: e est non nul.
    let max_chars = unsafe { (*e).max_chars };
    let mut pos = 0u32;
    let mut n = 0u32;
    while pos < len && n < max_chars {
        // SAFETY: pos < len, text couvre len octets.
        pos = unsafe { ctl_u8_next(base, len, pos) };
        n = n.wrapping_add(1);
    }
    // SAFETY: pos <= len et len < CTL_TEXT_MAX, donc pos est dans text.
    unsafe { base.add(pos as usize).write(0) };
    // SAFETY: e est inscriptible.
    unsafe {
        if (*e).caret > pos {
            (*e).caret = pos;
        }
        if (*e).anchor > pos {
            (*e).anchor = pos;
        }
    }
}

/// # Safety
/// `c` est non nul. Son texte est terminé. `upto` octets sont dans le tampon si le mot de passe est actif.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_dpos(c: *const t_ctl, off: u32) -> u32 {
    // SAFETY: c est non nul.
    if unsafe { (*c).flags } & CTL_PASSWORD != 0 {
        // SAFETY: off octets du texte sont lisibles, ou off vaut 0.
        return unsafe { ctl_u8_count(text_of(c), off) };
    }
    off
}

/// # Safety
/// `c` est non nul. `v` est non nul et inscriptible, `disp` a `CTL_TEXT_MAX` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_view(c: *const t_ctl, v: *mut t_edview) -> bool {
    // SAFETY: c est non nul.
    let font = unsafe { ctl_font_of(c) };
    // SAFETY: v est non nul.
    unsafe { (*v).font = font };
    if font.is_null() {
        return false;
    }
    let text = text_of(c);
    // SAFETY: c est non nul.
    if unsafe { (*c).flags } & CTL_PASSWORD != 0 {
        let n = slen(text);
        // SAFETY: text couvre n octets.
        let n = unsafe { ctl_u8_count(text, n) };
        // SAFETY: v.disp a CTL_TEXT_MAX octets et n y tient.
        unsafe {
            fill_byte((*v).disp.as_mut_ptr().cast(), b'*', n as usize);
            (*v).disp[n as usize] = 0;
            (*v).len = n as i32;
        }
        return true;
    }
    // SAFETY: v.disp a CTL_TEXT_MAX octets, text est terminé.
    unsafe { ctl_copy_text((*v).disp.as_mut_ptr().cast(), text) };
    // SAFETY: disp est terminé par ctl_copy_text.
    let n = slen(unsafe { (*v).disp.as_ptr().cast() });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*v).len = n as i32 };
    true
}

/// # Safety
/// `v` et `c` sont non nuls. `v.font` et `v.disp` sont initialisés.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_wv(v: *const t_edview, c: *const t_ctl, upto: u32) -> i32 {
    // SAFETY: v est non nul.
    let v = unsafe { &*v };
    // SAFETY: c est non nul. upto est un décalage du texte.
    let d = unsafe { ctl_edt_dpos(c, upto) } as i32;
    font_text_width(v.font, v.disp.as_ptr().cast(), d)
}

/// # Safety
/// `c` est non nul. Son texte est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_width(c: *const t_ctl, upto: u32) -> i32 {
    let mut v = empty_view();
    // SAFETY: v est local et inscriptible.
    if unsafe { !ctl_edt_view(c, &mut v) } {
        return 0;
    }
    // SAFETY: la vue est remplie.
    unsafe { ctl_edt_wv(&v, c, upto) }
}

/// # Safety
/// `c` et `e` sont non nuls. Le rectangle de `c` est dans la plage des coordonnées.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_scroll(c: *mut t_ctl, e: *mut t_edit) {
    // SAFETY: c est non nul.
    let w = unsafe { (*c).rect.w };
    let view = w - 2 * CTL_EDIT_INSET;
    if view <= 0 {
        // SAFETY: e est inscriptible.
        unsafe { (*e).scroll = 0 };
        return;
    }
    // SAFETY: e est non nul.
    let caret = unsafe { (*e).caret };
    // SAFETY: c est non nul, le caret est un décalage du texte.
    let cx = unsafe { ctl_edt_width(c, caret) };
    // SAFETY: e est inscriptible.
    unsafe {
        if cx < (*e).scroll {
            (*e).scroll = cx;
        }
        if cx > (*e).scroll + view - 1 {
            (*e).scroll = cx - view + 1;
        }
    }
    let len = slen(text_of(c));
    // SAFETY: c est non nul.
    let mut limit = unsafe { ctl_edt_width(c, len) } - view + 1;
    if limit < 0 {
        limit = 0;
    }
    // SAFETY: e est inscriptible.
    unsafe {
        if (*e).scroll > limit {
            (*e).scroll = limit;
        }
        if (*e).scroll < 0 {
            (*e).scroll = 0;
        }
    }
}

/// # Safety
/// `c` et `e` sont non nuls. `c.text` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_clamp(c: *mut t_ctl, e: *mut t_edit) {
    truncate_chars(c, e);
    let n = slen(text_of(c));
    // SAFETY: e est inscriptible.
    unsafe {
        (*e).caret = n;
        (*e).anchor = n;
        (*e).scroll = 0;
    }
    // SAFETY: c et e restent vivants.
    unsafe { ctl_edt_scroll(c, e) };
}

/// # Safety
/// `r` et `c` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edit_set_limit(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    mut max_chars: u32,
) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || c.is_null() || unsafe { (*r).root.is_null() } {
        return E_INVAL;
    }
    // SAFETY: c est non nul.
    if unsafe { (*c).r#type } != CT_EDIT || max_chars == 0 {
        return E_INVAL;
    }
    // SAFETY: r et c sont non nuls.
    if unsafe { !ctl_in_tree(r, c) } {
        return E_INVAL;
    }
    if max_chars > CTL_TEXT_MAX as u32 - 1 {
        max_chars = CTL_TEXT_MAX as u32 - 1;
    }
    // SAFETY: le privé d'un edit est un t_edit.
    let e = unsafe { (*c).r#priv.cast::<t_edit>() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*e).max_chars = max_chars };
    truncate_chars(c, e);
    // SAFETY: c et e restent vivants.
    unsafe { ctl_edt_scroll(c, e) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, rect) };
    0
}

fn target(c: *const t_ctl, e: *const t_edit, code: u32, shift: bool) -> u32 {
    let text = text_of(c);
    let len = slen(text);
    if code == VK_HOME {
        return 0;
    }
    if code == VK_END {
        return len;
    }
    // SAFETY: e est non nul.
    let caret = unsafe { (*e).caret };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if code == VK_LEFT && unsafe { ctl_edt_has_sel(e) } && !shift {
        // SAFETY: e est non nul.
        return unsafe { ctl_edt_sel_from(e) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if code == VK_RIGHT && unsafe { ctl_edt_has_sel(e) } && !shift {
        // SAFETY: e est non nul.
        return unsafe { ctl_edt_sel_to(e) };
    }
    if code == VK_LEFT {
        // SAFETY: text couvre caret octets.
        return unsafe { ctl_u8_prev(text, caret) };
    }
    // SAFETY: text couvre len octets.
    unsafe { ctl_u8_next(text, len, caret) }
}

/// # Safety
/// `c` et `e` sont non nuls. `c.text` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_nav(
    c: *mut t_ctl,
    e: *mut t_edit,
    code: u32,
    shift: bool,
) -> bool {
    // SAFETY: e est non nul.
    let old_caret = unsafe { (*e).caret };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let old_anchor = unsafe { (*e).anchor };
    let next = target(c, e, code, shift);
    // SAFETY: e est inscriptible.
    unsafe {
        (*e).caret = next;
        if !shift {
            (*e).anchor = next;
        }
        old_caret != (*e).caret || old_anchor != (*e).anchor
    }
}

/// # Safety
/// `c` est non nul. `x` est une abscisse dans la plage des coordonnées.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_index_at(c: *const t_ctl, x: i32) -> u32 {
    let text = text_of(c);
    let len = slen(text);
    let mut v = empty_view();
    // SAFETY: v est local.
    if unsafe { !ctl_edt_view(c, &mut v) } {
        return 0;
    }
    let mut pos = 0u32;
    let mut w0 = 0i32;
    while pos < len {
        // SAFETY: pos < len, text couvre len octets.
        let next = unsafe { ctl_u8_next(text, len, pos) };
        // SAFETY: la vue est remplie.
        let w1 = unsafe { ctl_edt_wv(&v, c, next) };
        if x < (w0 + w1) / 2 {
            return pos;
        }
        pos = next;
        w0 = w1;
    }
    len
}

/// # Safety
/// `c` et `e` sont non nuls. La sélection est dans le texte. Le presse-papiers est le tampon global.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_copy(c: *mut t_ctl, e: *mut t_edit) -> bool {
    // SAFETY: e est non nul.
    if unsafe { !ctl_edt_has_sel(e) } {
        return false;
    }
    // SAFETY: c est non nul.
    if unsafe { (*c).flags } & CTL_PASSWORD != 0 {
        return false;
    }
    // SAFETY: e est non nul.
    let from = unsafe { ctl_edt_sel_from(e) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let to = unsafe { ctl_edt_sel_to(e) };
    let src = text_of(c).wrapping_add(from as usize);
    // SAFETY: src couvre to - from octets.
    unsafe { ctl_clip_set(src, to - from) == 0 }
}

fn sanitize(src: *const c_char, n: u32, dst: *mut c_char) -> u32 {
    let mut pos = 0u32;
    let mut out = 0u32;
    while pos < n {
        // SAFETY: pos < n, src couvre n octets.
        let l = unsafe { ctl_u8_len(src.wrapping_add(pos as usize), n - pos) };
        // SAFETY: pos < n, l'octet est lisible.
        let b = unsafe { src.add(pos as usize).read() } as u8;
        if l > 1 || (0x20..0x7f).contains(&b) {
            scopy_distinct(
                dst.wrapping_add(out as usize).cast(),
                src.wrapping_add(pos as usize).cast(),
                l as usize,
            );
            out = out.wrapping_add(l);
        }
        pos = pos.wrapping_add(l);
    }
    // SAFETY: out < CTL_TEXT_MAX, dst a cette place.
    unsafe { dst.add(out as usize).write(0) };
    out
}

/// # Safety
/// `c` et `e` sont non nuls. Le presse-papiers global est le tampon de `ctl_clip_get`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_paste(c: *mut t_ctl, e: *mut t_edit) -> bool {
    let mut clip = [0u8; CTL_TEXT_MAX];
    let mut clean = [0u8; CTL_TEXT_MAX];
    // SAFETY: clip a CTL_TEXT_MAX octets.
    let n = unsafe { ctl_clip_get(clip.as_mut_ptr().cast(), CTL_TEXT_MAX as u32) };
    if n <= 0 {
        return false;
    }
    let n = sanitize(clip.as_ptr().cast(), n as u32, clean.as_mut_ptr().cast());
    if n == 0 {
        return false;
    }
    // SAFETY: clean couvre n octets, c et e sont vivants.
    unsafe { ctl_edt_insert(c, e, clean.as_ptr().cast(), n) }
}

/// # Safety
/// `c` et `e` sont non nuls. `ev` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_char(c: *mut t_ctl, e: *mut t_edit, ev: *const t_inpevent) -> u32 {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    let alt = ev.mods & INPM_ALT != 0;
    let ctrl = ev.mods & INPM_CTRL != 0;
    if alt != ctrl {
        return 0;
    }
    if ev.code < 0x20 || (ev.code >= 0x7f && ev.code < 0xa0) {
        return 0;
    }
    let mut buf = [0u8; 4];
    // SAFETY: buf a 4 octets. Un refus n'écrit rien.
    let n = unsafe { ctl_u8_encode(ev.code, buf.as_mut_ptr().cast()) };
    if n == 0 {
        return 0;
    }
    // SAFETY: buf couvre n octets.
    if unsafe { ctl_edt_insert(c, e, buf.as_ptr().cast(), n) } {
        return CTL_EDR_USED | CTL_EDR_TEXT;
    }
    CTL_EDR_USED
}

/// # Safety
/// `r` et `c` sont non nuls. `c` est dans l'arbre au début de l'appel.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_finish(r: *mut t_ctlroot, c: *mut t_ctl, res: u32) -> bool {
    if res & (CTL_EDR_TEXT | CTL_EDR_VIEW) != 0 {
        // SAFETY: le privé d'un edit est un t_edit.
        let e = unsafe { (*c).r#priv.cast::<t_edit>() };
        // SAFETY: c et e sont vivants.
        unsafe { ctl_edt_scroll(c, e) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: r a une surface.
        unsafe { ctl_dirty_add(r, rect) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if res & CTL_EDR_TEXT != 0 && unsafe { !ctl_notify(r, c, CN_CHANGED) } {
        return true;
    }
    if res & CTL_EDR_ENTER != 0 {
        // SAFETY: c est encore vivant, le texte n'a pas détruit le contrôle.
        unsafe { ctl_notify(r, c, CN_ENTER) };
    }
    res & CTL_EDR_USED != 0
}

fn flag_if(changed: bool, flags: u32) -> u32 {
    if changed {
        CTL_EDR_USED | flags
    } else {
        CTL_EDR_USED
    }
}

fn down_ctrl(c: *mut t_ctl, e: *mut t_edit, code: u32) -> u32 {
    if code == CTL_VK_A {
        // SAFETY: c et e sont non nuls.
        unsafe { ctl_edt_select_all(c, e) };
        return CTL_EDR_USED | CTL_EDR_VIEW;
    }
    if code == CTL_VK_C {
        // SAFETY: c et e sont non nuls.
        return flag_if(unsafe { ctl_edt_copy(c, e) }, 0);
    }
    if code == CTL_VK_X {
        // SAFETY: c et e sont non nuls. erase n'est appelé que si la copie a réussi.
        let changed = unsafe { ctl_edt_copy(c, e) && ctl_edt_erase_sel(c, e) };
        return flag_if(changed, CTL_EDR_TEXT);
    }
    if code == CTL_VK_V {
        // SAFETY: c et e sont non nuls.
        return flag_if(unsafe { ctl_edt_paste(c, e) }, CTL_EDR_TEXT);
    }
    0
}

fn down_plain(c: *mut t_ctl, e: *mut t_edit, ev: &t_inpevent) -> u32 {
    let shift = ev.mods & INPM_SHIFT != 0;
    if ev.code == VK_LEFT || ev.code == VK_RIGHT || ev.code == VK_HOME || ev.code == VK_END {
        // SAFETY: c et e sont non nuls.
        return flag_if(unsafe { ctl_edt_nav(c, e, ev.code, shift) }, CTL_EDR_VIEW);
    }
    if ev.code == VK_BACK {
        // SAFETY: c et e sont non nuls.
        return flag_if(unsafe { ctl_edt_erase_prev(c, e) }, CTL_EDR_TEXT);
    }
    if ev.code == VK_DELETE {
        // SAFETY: c et e sont non nuls.
        return flag_if(unsafe { ctl_edt_erase_next(c, e) }, CTL_EDR_TEXT);
    }
    if ev.code == VK_RETURN {
        return CTL_EDR_USED | CTL_EDR_ENTER;
    }
    0
}

/// # Safety
/// `r`, `c` et `ev` sont non nuls. `c.priv` est un `t_edit`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_key(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    ev: *const t_inpevent,
) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    let alt = ev.mods & INPM_ALT != 0;
    let ctrl = ev.mods & INPM_CTRL != 0;
    // SAFETY: le privé d'un edit est un t_edit.
    let e = unsafe { (*c).r#priv.cast::<t_edit>() };
    let res = if ev.r#type == INP_CHAR {
        // SAFETY: c, e et ev sont vivants.
        unsafe { ctl_edt_char(c, e, ev) }
    } else if ctrl && !alt {
        down_ctrl(c, e, ev.code)
    } else if !alt && !ctrl {
        down_plain(c, e, ev)
    } else {
        0
    };
    // SAFETY: r et c sont vivants au début.
    unsafe { ctl_edt_finish(r, c, res) }
}

fn index_of(c: *const t_ctl, ed: *const t_edit, e: &t_mouseev) -> u32 {
    // SAFETY: c et ed sont non nuls.
    let x = unsafe { e.at.x - (*c).rect.x - CTL_EDIT_INSET + (*ed).scroll };
    // SAFETY: c est non nul.
    unsafe { ctl_edt_index_at(c, x) }
}

fn edt_down(r: *mut t_ctlroot, c: *mut t_ctl, ed: *mut t_edit, e: &t_mouseev) -> bool {
    // SAFETY: r est non nul.
    unsafe { (*r).capture = c };
    if e.dbl {
        // SAFETY: c et ed sont non nuls.
        unsafe { ctl_edt_select_all(c, ed) };
    } else {
        let idx = index_of(c, ed, e);
        // SAFETY: ed est inscriptible.
        unsafe {
            (*ed).caret = idx;
            (*ed).anchor = idx;
        }
    }
    // SAFETY: c et ed sont vivants.
    unsafe { ctl_edt_scroll(c, ed) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, rect) };
    true
}

/// # Safety
/// `r`, `c` et `e` sont non nuls. `c.priv` est un `t_edit`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_mouse(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    e: *const t_mouseev,
) -> bool {
    // SAFETY: e est non nul. Le privé est un t_edit.
    let e = unsafe { &*e };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let ed = unsafe { (*c).r#priv.cast::<t_edit>() };
    if e.kind == CTL_MEV_DOWN {
        return edt_down(r, c, ed, e);
    }
    // SAFETY: r est non nul.
    if unsafe { (*r).capture } != c {
        return false;
    }
    if e.kind == CTL_MEV_UP {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*r).capture = core::ptr::null_mut() };
    }
    if e.kind == CTL_MEV_MOVE {
        let idx = index_of(c, ed, e);
        // SAFETY: ed est non nul.
        if idx != unsafe { (*ed).caret } {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { (*ed).caret = idx };
            // SAFETY: c et ed sont vivants.
            unsafe { ctl_edt_scroll(c, ed) };
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            let rect = unsafe { (*c).rect };
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { ctl_dirty_add(r, rect) };
        }
    }
    true
}

fn paint_frame(s: *mut t_surface, r: t_rect, en: bool) {
    // SAFETY: s est une surface vivante.
    unsafe { gfx_fill(s, r, ctl_color_back(en)) };
    luna_edit_frame(s, r, en);
    let inn = rect_make(
        r.x + CTL_FRAME,
        r.y + CTL_FRAME,
        r.w - 2 * CTL_FRAME,
        r.h - 2 * CTL_FRAME,
    );
    // SAFETY: s est non nul.
    let clip = unsafe { (*s).clip };
    gfx_set_clip(s, rect_intersect(clip, inn));
}

fn draw_seg(s: *mut t_surface, v: &t_edview, a: i32, b: i32) {
    if b <= a {
        return;
    }
    let rq = t_textreq {
        font: v.font,
        at: t_point {
            x: v.org.x + font_text_width(v.font, v.disp.as_ptr().cast(), a),
            y: v.org.y,
        },
        color: v.col,
        text: v.disp.as_ptr().wrapping_add(a as usize).cast(),
        len: b - a,
    };
    font_draw(s, &rq);
}

fn draw_runs(s: *mut t_surface, v: &mut t_edview, c: *const t_ctl, sel: bool) {
    // SAFETY: c est non nul. Le privé est un t_edit.
    let e = unsafe { (*c).r#priv.cast::<t_edit>() };
    // SAFETY: c est non nul.
    v.col = unsafe { ctl_color_text(ctl_enabled(c)) };
    if !sel {
        draw_seg(s, v, 0, v.len);
        return;
    }
    // SAFETY: e est non nul.
    let a = unsafe { ctl_edt_dpos(c, ctl_edt_sel_from(e)) } as i32;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let b = unsafe { ctl_edt_dpos(c, ctl_edt_sel_to(e)) } as i32;
    draw_seg(s, v, 0, a);
    draw_seg(s, v, b, v.len);
    let x = v.org.x + font_text_width(v.font, v.disp.as_ptr().cast(), a);
    let sel_w = font_text_width(
        v.font,
        v.disp.as_ptr().wrapping_add(a as usize).cast(),
        b - a,
    );
    // SAFETY: la police de la vue n'est pas nulle.
    let h = unsafe { (*v.font).height };
    gfx_fill(s, rect_make(x, v.org.y, sel_w, h), luna_color_selection());
    // SAFETY: pas de pointeur.
    v.col = unsafe { ctl_color_selected_text() };
    draw_seg(s, v, a, b);
}

/// # Safety
/// `r` et `c` sont non nuls. `r.surface` est vivante. `c.priv` est un `t_edit`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_edt_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c et r sont non nuls.
    let e = unsafe { (*c).r#priv.cast::<t_edit>() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let en = unsafe { ctl_enabled(c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let surface = unsafe { (*r).surface };
    paint_frame(surface, rect, en);
    let mut v = empty_view();
    // SAFETY: v est local.
    if unsafe { !ctl_edt_view(c, &mut v) } {
        return;
    }
    // SAFETY: e est non nul, la police n'est pas nulle.
    let scroll = unsafe { (*e).scroll };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let height = unsafe { (*v.font).height };
    v.org.x = rect.x + CTL_EDIT_INSET - scroll;
    v.org.y = rect.y + (rect.h - height) / 2;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let focus = en && unsafe { (*r).focus } == c;
    // SAFETY: e est non nul.
    let sel = focus && unsafe { ctl_edt_has_sel(e) };
    draw_runs(surface, &mut v, c, sel);
    if focus {
        // SAFETY: e est non nul.
        let caret = unsafe { (*e).caret };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let x = v.org.x + unsafe { ctl_edt_wv(&v, c, caret) };
        // SAFETY: pas de pointeur autre que la surface.
        unsafe {
            gfx_vline(
                surface,
                t_point { x, y: v.org.y },
                height,
                ctl_color_text(true),
            )
        };
    }
}
