use crate::abi::{
    CT_BUTTON, CT_CHECK, CT_GROUP, CT_LABEL, CT_MENU, CT_RADIO, CTL_DEFAULT, CTL_ID_CANCEL,
    INP_CHAR, INP_KEY_DOWN, INPM_ALT, INPM_CTRL, INPM_REPEAT, INPM_SHIFT, VK_ESCAPE, VK_RETURN,
    VK_TAB, t_ctl, t_ctlroot, t_inpevent,
};
use crate::focus::ctl_focus;
use crate::focus::ctl_focus_move;
use crate::notify::ctl_activate;
use crate::ops::ctl_ops_get;
use crate::text::ctl_mnemonic;
use crate::tree::{ctl_find_id, ctl_reap, ctl_usable, ctl_walk_next};

fn key_target(r: &t_ctlroot) -> *mut t_ctl {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !r.capture.is_null() && unsafe { (*r.capture).r#type } == CT_MENU {
        return r.capture;
    }
    r.focus
}

fn has_text(c: &t_ctl) -> bool {
    c.r#type == CT_BUTTON
        || c.r#type == CT_CHECK
        || c.r#type == CT_RADIO
        || c.r#type == CT_LABEL
        || c.r#type == CT_GROUP
}

fn mnemo_match(c: *const t_ctl, code: u32) -> bool {
    // SAFETY: c n'est pas nul dans le parcours.
    let node = unsafe { &*c };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !has_text(node) || unsafe { !ctl_usable(c) } {
        return false;
    }
    // SAFETY: node.text est terminé.
    unsafe { ctl_mnemonic(node.text_c()) as u32 == code }
}

fn search(r: &t_ctlroot, code: u32) -> *mut t_ctl {
    let mut first: *mut t_ctl = core::ptr::null_mut();
    let mut past = false;
    let mut cur = r.root;
    while !cur.is_null() {
        if past && cur != r.focus && mnemo_match(cur, code) {
            return cur;
        }
        if first.is_null() && mnemo_match(cur, code) {
            first = cur;
        }
        if cur == r.focus {
            past = true;
        }
        // SAFETY: cur est dans l'arbre fini.
        cur = unsafe { ctl_walk_next(cur) };
    }
    first
}

/// # Safety
/// `r` est non nul et initialisé. L'arbre est fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnemonic_fire(r: *mut t_ctlroot, mut code: u32) -> bool {
    if (b'a' as u32..=b'z' as u32).contains(&code) {
        code = code - b'a' as u32 + b'A' as u32;
    }
    // SAFETY: r n'est pas nul.
    let to = search(unsafe { &*r }, code);
    if to.is_null() {
        return false;
    }
    // SAFETY: to est dans l'arbre.
    unsafe { ctl_focus(r, to) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_activate(r, to) };
    true
}

fn default_button(r: &t_ctlroot) -> *mut t_ctl {
    let mut cur = r.root;
    while !cur.is_null() {
        // SAFETY: cur est dans l'arbre.
        let node = unsafe { &*cur };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        if node.r#type == CT_BUTTON && node.flags & CTL_DEFAULT != 0 && unsafe { ctl_usable(cur) } {
            return cur;
        }
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        cur = unsafe { ctl_walk_next(cur) };
    }
    core::ptr::null_mut()
}

fn fire(r: *mut t_ctlroot, c: *mut t_ctl) -> bool {
    if c.is_null() {
        return false;
    }
    // SAFETY: c n'est pas nul.
    if unsafe { (*c).r#type } != CT_BUTTON || unsafe { !ctl_usable(c) } {
        return false;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_activate(r, c) }
}

/// # Safety
/// `r` est non nul et initialisé. `ev` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_key_global(r: *mut t_ctlroot, ev: *const t_inpevent) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    let alt = ev.mods & INPM_ALT != 0;
    let ctrl = ev.mods & INPM_CTRL != 0;
    if ev.code == VK_TAB && !alt && !ctrl {
        // SAFETY: r est initialisé.
        return unsafe { ctl_focus_move(r, ev.mods & INPM_SHIFT == 0) };
    }
    if alt && !ctrl {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        return unsafe { ctl_mnemonic_fire(r, ev.code) };
    }
    if alt || ctrl || ev.mods & INPM_REPEAT != 0 {
        return false;
    }
    if ev.code == VK_RETURN {
        // SAFETY: r est initialisé.
        let btn = default_button(unsafe { &*r });
        return fire(r, btn);
    }
    if ev.code == VK_ESCAPE {
        // SAFETY: r est initialisé.
        return fire(r, unsafe { ctl_find_id(r, CTL_ID_CANCEL) });
    }
    false
}

fn route(r: *mut t_ctlroot, ev: &t_inpevent) -> bool {
    if ev.r#type != INP_KEY_DOWN && ev.r#type != INP_CHAR {
        return false;
    }
    // SAFETY: r n'est pas nul.
    let t = key_target(unsafe { &*r });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !t.is_null() && unsafe { ctl_usable(t) } {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let ops = unsafe { ctl_ops_get((*t).r#type) };
        if !ops.is_null()
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            && let Some(key) = unsafe { (*ops).key }
        {
            // SAFETY: key est l'entrée de la table, ev est vivant.
            if unsafe { key(r, t, ev) } {
                return true;
            }
        }
    }
    if ev.r#type != INP_KEY_DOWN {
        return false;
    }
    // SAFETY: r et ev sont vivants.
    unsafe { ctl_key_global(r, ev) }
}

/// # Safety
/// `r` et `ev` sont nuls ou vivants. L'arbre est fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_key(r: *mut t_ctlroot, ev: *const t_inpevent) -> bool {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || ev.is_null() || unsafe { (*r).root.is_null() } {
        return false;
    }
    // SAFETY: ev n'est pas nul.
    let used = route(r, unsafe { &*ev });
    // SAFETY: r est initialisé.
    unsafe { ctl_reap(r) };
    used
}
