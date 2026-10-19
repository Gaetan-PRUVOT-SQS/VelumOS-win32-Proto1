use crate::abi::{CT_RADIO, CTL_CHECKED, CTL_ENABLED, CTL_VISIBLE, t_ctl, t_ctlroot, t_focusscan};
use crate::raw::szero;
use crate::tree::ctl_dirty_add;

fn radio_ok(s: &t_ctl) -> bool {
    s.r#type == CT_RADIO && s.flags & CTL_VISIBLE != 0 && s.flags & CTL_ENABLED != 0
}

fn radio_scan(c: &t_ctl, sc: &mut t_focusscan) {
    szero(
        (sc as *mut t_focusscan).cast(),
        core::mem::size_of::<t_focusscan>(),
    );
    let mut past = false;
    let mut s = c.parent;
    if s.is_null() {
        return;
    }
    // SAFETY: parent n'est pas nul.
    s = unsafe { &*s }.first;
    while !s.is_null() {
        // SAFETY: s n'est pas nul.
        let node = unsafe { &*s };
        if core::ptr::eq(s, c) {
            past = true;
        }
        if radio_ok(node) {
            if sc.first.is_null() {
                sc.first = s;
            }
            sc.last = s;
            if !core::ptr::eq(s, c) && !past {
                sc.before = s;
            } else if !core::ptr::eq(s, c) && sc.after.is_null() {
                sc.after = s;
            }
        }
        s = node.next;
    }
}

/// # Safety
/// `r` et `c` sont non nuls. `c` a un parent. Les frères sont finis.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_radio_select(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c et son parent sont non nuls.
    let mut sib = unsafe { &*(*c).parent }.first;
    while !sib.is_null() {
        // SAFETY: sib n'est pas nul.
        let node = unsafe { &mut *sib };
        let has = node.flags & CTL_CHECKED != 0;
        if node.r#type == CT_RADIO && has != core::ptr::eq(sib, c) {
            node.flags ^= CTL_CHECKED;
            let rect = node.rect;
            // SAFETY: r a une surface.
            unsafe { ctl_dirty_add(r, rect) };
        }
        sib = node.next;
    }
}

/// # Safety
/// `c` est non nul et a un parent.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_radio_stop(c: *const t_ctl) -> bool {
    // SAFETY: c et son parent sont non nuls.
    let c = unsafe { &*c };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mut sib = unsafe { &*c.parent }.first;
    let mut first: *mut t_ctl = core::ptr::null_mut();
    while !sib.is_null() {
        // SAFETY: sib n'est pas nul.
        let node = unsafe { &*sib };
        if radio_ok(node) && node.flags & CTL_CHECKED != 0 {
            return core::ptr::eq(sib, c);
        }
        if radio_ok(node) && first.is_null() {
            first = sib;
        }
        sib = node.next;
    }
    core::ptr::eq(first, c)
}

/// # Safety
/// `c` est non nul et a un parent. Les frères sont finis.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_radio_step(c: *const t_ctl, forward: bool) -> *mut t_ctl {
    let mut sc = t_focusscan {
        first: core::ptr::null_mut(),
        last: core::ptr::null_mut(),
        before: core::ptr::null_mut(),
        after: core::ptr::null_mut(),
    };
    // SAFETY: c est non nul.
    radio_scan(unsafe { &*c }, &mut sc);
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
/// `r` et `c` sont non nuls. `c` est un radio dans l'arbre.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_rad_activate(r: *mut t_ctlroot, c: *mut t_ctl) -> bool {
    // SAFETY: c est un radio avec parent.
    unsafe { ctl_radio_select(r, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { &*c }.rect;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { crate::notify::ctl_notify(r, c, crate::abi::CN_CLICKED) };
    true
}

fn rad_move(r: *mut t_ctlroot, c: *mut t_ctl, forward: bool) -> bool {
    // SAFETY: c est un radio.
    let to = unsafe { ctl_radio_step(c, forward) };
    if to.is_null() || to == c {
        return true;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { crate::focus::ctl_focus(r, to) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_rad_activate(r, to) };
    true
}

/// # Safety
/// `r`, `c` et `ev` sont non nuls. `c` est un radio.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_rad_key(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    ev: *const crate::abi::t_inpevent,
) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    if ev.r#type != crate::abi::INP_KEY_DOWN
        || ev.mods & (crate::abi::INPM_ALT | crate::abi::INPM_CTRL) != 0
    {
        return false;
    }
    if ev.code == crate::abi::VK_SPACE {
        if ev.mods & crate::abi::INPM_REPEAT == 0 {
            // SAFETY: c est activable.
            unsafe { crate::notify::ctl_activate(r, c) };
        }
        return true;
    }
    if ev.code == crate::abi::VK_LEFT || ev.code == crate::abi::VK_UP {
        return rad_move(r, c, false);
    }
    if ev.code == crate::abi::VK_RIGHT || ev.code == crate::abi::VK_DOWN {
        return rad_move(r, c, true);
    }
    false
}
