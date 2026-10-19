use core::ffi::c_void;

use crate::abi::{CTL_ST_HOT, t_ctl, t_ctlroot};
use crate::ops::ctl_ops_get;
use crate::tree::{ctl_alive, ctl_dirty_add};

/// # Safety
/// `r` et `c` sont non nuls. `c` est dans l'arbre, ou le rappel peut l'avoir retiré.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_notify(r: *mut t_ctlroot, c: *mut t_ctl, code: u32) -> bool {
    // SAFETY: c est non nul.
    let cb = unsafe { (*c).cb };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let user = unsafe { (*c).user };
    if let Some(cb) = cb {
        // SAFETY: cb est le rappel du contrôle, vivant pour cet appel.
        unsafe { cb(c.cast::<c_void>(), code, user) };
    }
    // SAFETY: r et c sont ceux du contrat, l'arbre est fini.
    if unsafe { !ctl_alive(r, c) } {
        return false;
    }
    // SAFETY: r n'est pas nul.
    let command = unsafe { (*r).command };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let root_user = unsafe { (*r).user };
    if let Some(command) = command {
        // SAFETY: command est le rappel de la racine.
        unsafe { command(c.cast::<c_void>(), code, root_user) };
    }
    // SAFETY: même contrat qu'au-dessus.
    unsafe { ctl_alive(r, c) }
}

/// # Safety
/// `r` et `c` sont non nuls. `c` a un type connu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_activate(r: *mut t_ctlroot, c: *mut t_ctl) -> bool {
    // SAFETY: c est non nul.
    let ty = unsafe { (*c).r#type };
    // SAFETY: ty est l'entier du contrôle.
    let ops = unsafe { ctl_ops_get(ty) };
    if ops.is_null() {
        return false;
    }
    // SAFETY: ops n'est pas nul.
    let Some(activate) = (unsafe { (*ops).activate }) else {
        return false;
    };
    // SAFETY: activate est l'entrée de la table pour ce type.
    unsafe { activate(r, c) }
}

/// # Safety
/// `r` est non nul et a une surface. `c` est nul ou vivant.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_hot_set(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: r n'est pas nul.
    if unsafe { (*r).hot } == c {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let old = unsafe { (*r).hot };
    if !old.is_null() {
        // SAFETY: old est le contrôle chaud précédent.
        unsafe { (*old).state &= !CTL_ST_HOT };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*old).rect };
        // SAFETY: r a une surface.
        unsafe { ctl_dirty_add(r, rect) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*r).hot = c };
    if !c.is_null() {
        // SAFETY: c est vivant.
        unsafe { (*c).state |= CTL_ST_HOT };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
}
