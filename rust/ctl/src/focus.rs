use crate::abi::{CT_EDIT, CT_MENU, CT_RADIO, CTL_FOCUSABLE, t_ctl, t_ctlroot, t_focusscan};
use crate::edit::ctl_edt_select_all;
use crate::radio::ctl_radio_stop;
use crate::raw::szero;
use crate::tree::{ctl_dirty_add, ctl_in_tree, ctl_usable, ctl_walk_next};

/// # Safety
/// `r` est nul ou initialisé. `c` est nul ou dans un arbre fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_focus(r: *mut t_ctlroot, c: *mut t_ctl) {
    if r.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    let root = unsafe { &mut *r };
    if root.root.is_null() {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !root.capture.is_null() && unsafe { &*root.capture }.r#type == CT_MENU {
        return;
    }
    if !c.is_null()
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        && (unsafe { !ctl_in_tree(r, c) }
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            || unsafe { &*c }.flags & CTL_FOCUSABLE == 0
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            || unsafe { !ctl_usable(c) })
    {
        return;
    }
    if root.focus == c {
        return;
    }
    if !root.focus.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { &*root.focus }.rect;
        // SAFETY: r a une surface.
        unsafe { ctl_dirty_add(r, rect) };
    }
    root.focus = c;
    if !c.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { &*c }.rect;
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
}

/// # Safety
/// `c` est non nul. La chaîne des parents est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_tabstop(c: *const t_ctl) -> bool {
    // SAFETY: c est non nul.
    let node = unsafe { &*c };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if node.flags & CTL_FOCUSABLE == 0 || unsafe { !ctl_usable(c) } {
        return false;
    }
    if node.r#type == CT_RADIO {
        // SAFETY: c est un radio avec un parent.
        return unsafe { ctl_radio_stop(c) };
    }
    true
}

fn tally(sc: &mut t_focusscan, cur: *mut t_ctl, focus: *mut t_ctl, past: bool) {
    if sc.first.is_null() {
        sc.first = cur;
    }
    sc.last = cur;
    if cur == focus {
        return;
    }
    if !past {
        sc.before = cur;
    } else if sc.after.is_null() {
        sc.after = cur;
    }
}

/// # Safety
/// `r` et `sc` sont non nuls. L'arbre est fini. `sc` est inscriptible.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_focus_scan(r: *const t_ctlroot, sc: *mut t_focusscan) {
    szero(sc.cast(), core::mem::size_of::<t_focusscan>());
    // SAFETY: r et sc sont non nuls.
    let root = unsafe { &*r };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let sc = unsafe { &mut *sc };
    let mut past = false;
    let mut cur = root.root;
    while !cur.is_null() {
        if cur == root.focus {
            past = true;
        }
        // SAFETY: cur est dans l'arbre.
        if unsafe { ctl_tabstop(cur) } {
            tally(sc, cur, root.focus, past);
        }
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        cur = unsafe { ctl_walk_next(cur) };
    }
}

/// # Safety
/// `r` est non nul et initialisé. `c` est dans l'arbre.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_focus_after(r: *mut t_ctlroot, c: *const t_ctl) -> bool {
    // SAFETY: c est dans l'arbre.
    let mut cur = unsafe { ctl_walk_next(c) };
    while !cur.is_null() {
        // SAFETY: cur est dans l'arbre.
        if unsafe { ctl_tabstop(cur) } {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { ctl_focus(r, cur) };
            return true;
        }
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        cur = unsafe { ctl_walk_next(cur) };
    }
    false
}

fn pick(sc: &t_focusscan, forward: bool) -> *mut t_ctl {
    if forward && !sc.after.is_null() {
        return sc.after;
    }
    if forward {
        return sc.first;
    }
    if !sc.before.is_null() {
        return sc.before;
    }
    sc.last
}

/// # Safety
/// `r` est non nul et initialisé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_focus_move(r: *mut t_ctlroot, forward: bool) -> bool {
    let mut sc = t_focusscan {
        first: core::ptr::null_mut(),
        last: core::ptr::null_mut(),
        before: core::ptr::null_mut(),
        after: core::ptr::null_mut(),
    };
    // SAFETY: r et sc sont vivants.
    unsafe { ctl_focus_scan(r, &mut sc) };
    let to = pick(&sc, forward);
    if to.is_null() {
        return false;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_focus(r, to) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { &*to }.r#type == CT_EDIT {
        // SAFETY: un edit a un t_edit.
        unsafe { ctl_edt_select_all(to, (*to).r#priv.cast()) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { &*to }.rect;
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
    true
}
