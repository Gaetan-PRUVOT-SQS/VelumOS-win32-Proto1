use crate::abi::{CTL_VISIBLE, GFX_REGION_MAX, t_ctl, t_ctlroot, t_rect};
use crate::ffi::{
    gfx_set_clip, rect_empty, rect_intersect, rect_make, region_bounds, region_clear, region_empty,
};
use crate::ops::ctl_ops_get;

/// # Safety
/// `r` est non nul et a une surface. `c` est non nul. `clip` est un rectangle. L'arbre sous `c` est fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_paint_tree(r: *mut t_ctlroot, c: *mut t_ctl, clip: t_rect) {
    // SAFETY: c est non nul.
    let flags = unsafe { (*c).flags };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let ty = unsafe { (*c).r#type };
    if flags & CTL_VISIBLE == 0 {
        return;
    }
    let vis = rect_intersect(clip, rect);
    if rect_empty(vis) {
        return;
    }
    // SAFETY: r a une surface.
    gfx_set_clip(unsafe { (*r).surface }, vis);
    // SAFETY: ty est l'entier du contrôle.
    let ops = unsafe { ctl_ops_get(ty) };
    if !ops.is_null()
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        && let Some(paint) = unsafe { (*ops).paint }
    {
        // SAFETY: paint est l'entrée de la table. c est toujours là pour un type affichable.
        unsafe { paint(r, c) };
    }
    // SAFETY: c est toujours le contrôle peint.
    let mut kid = unsafe { (*c).first };
    while !kid.is_null() {
        // SAFETY: kid est un fils, l'arbre est fini.
        unsafe { ctl_paint_tree(r, kid, vis) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        kid = unsafe { (*kid).next };
    }
}

/// # Safety
/// `r` est nul ou initialisé, avec une surface si la racine existe.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_paint(r: *mut t_ctlroot) -> t_rect {
    let empty = rect_make(0, 0, 0, 0);
    if r.is_null() {
        return empty;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { (*r).root.is_null() } || unsafe { (*r).surface.is_null() } {
        return empty;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if region_empty(unsafe { &raw const (*r).dirty }) {
        return empty;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let work = unsafe { (*r).dirty };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    region_clear(unsafe { &raw mut (*r).dirty });
    let bounds = region_bounds(&raw const work);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let saved = unsafe { (*(*r).surface).clip };
    let mut i = 0u32;
    while i < work.n && (i as usize) < GFX_REGION_MAX {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let top = unsafe { (*r).root };
        // SAFETY: top est la racine, work.r[i] est dans la copie.
        unsafe { ctl_paint_tree(r, top, work.r[i as usize]) };
        i += 1;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    gfx_set_clip(unsafe { (*r).surface }, saved);
    bounds
}
