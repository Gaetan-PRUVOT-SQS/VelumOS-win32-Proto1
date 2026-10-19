use core::ffi::c_int;

use crate::abi::{
    CT_EDIT, CT_GROUP, CT_MENU, CT_PANEL, CT_RADIO, CT_TYPES, CTL_CHECKED, CTL_COORD_MAX,
    CTL_DEPTH_MAX, CTL_ENABLED, CTL_FLAGS_ALL, CTL_FLAGS_VISUAL, CTL_FONT_AUTO, CTL_ST_CLOSED,
    CTL_ST_HOT, CTL_ST_PRESSED, CTL_VISIBLE, E_INVAL, E_NOMEM, FONT_IDS, t_ctl, t_ctlcb,
    t_ctlclock, t_ctlroot, t_ctlspec, t_point, t_rect, t_rootpriv, t_surface,
};
use crate::edit::{ctl_edt_clamp, ctl_edt_scroll};
use crate::ffi::{rect_contains, rect_empty, rect_intersect, rect_make, region_add};
use crate::mem::{ctl_alloc, ctl_free};
use crate::ops::ctl_ops_get;
use crate::radio::ctl_radio_select;
use crate::raw::{scmp, scopy_distinct, slen, szero};
use crate::text::ctl_copy_text;

/// # Safety
/// `c` est non nul. La chaîne des parents est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_walk_next(c: *const t_ctl) -> *mut t_ctl {
    // SAFETY: c est non nul.
    let start = unsafe { &*c };
    if !start.first.is_null() {
        return start.first;
    }
    let mut cur = c;
    while !cur.is_null() {
        // SAFETY: cur n'est pas nul dans la boucle.
        let node = unsafe { &*cur };
        if !node.next.is_null() {
            return node.next;
        }
        cur = node.parent;
    }
    core::ptr::null_mut()
}

/// # Safety
/// `c` et `top` sont nuls ou dans un arbre fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_is_within(c: *const t_ctl, top: *const t_ctl) -> bool {
    let mut cur = c;
    while !cur.is_null() {
        if cur == top {
            return true;
        }
        // SAFETY: cur n'est pas nul.
        cur = unsafe { &*cur }.parent;
    }
    false
}

/// # Safety
/// `c` est nul ou la chaîne des parents est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_shown(c: *const t_ctl) -> bool {
    let mut cur = c;
    while !cur.is_null() {
        // SAFETY: cur n'est pas nul.
        let node = unsafe { &*cur };
        if node.flags & CTL_VISIBLE == 0 {
            return false;
        }
        cur = node.parent;
    }
    true
}

/// # Safety
/// `c` est nul ou la chaîne des parents est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_enabled(c: *const t_ctl) -> bool {
    let mut cur = c;
    while !cur.is_null() {
        // SAFETY: cur n'est pas nul.
        let node = unsafe { &*cur };
        if node.flags & CTL_ENABLED == 0 {
            return false;
        }
        cur = node.parent;
    }
    true
}

/// # Safety
/// `c` est nul ou la chaîne des parents est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_usable(c: *const t_ctl) -> bool {
    // SAFETY: même contrat que ctl_shown et ctl_enabled.
    unsafe { ctl_shown(c) && ctl_enabled(c) }
}

/// # Safety
/// `r` est non nul. `r.root` est nul ou l'arbre est fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_find_id(r: *const t_ctlroot, id: u32) -> *mut t_ctl {
    if id == 0 {
        return core::ptr::null_mut();
    }
    // SAFETY: r est non nul.
    let mut cur = unsafe { &*r }.root;
    while !cur.is_null() {
        // SAFETY: cur n'est pas nul.
        let node = unsafe { &*cur };
        if node.id == id && node.state & CTL_ST_CLOSED == 0 {
            return cur;
        }
        // SAFETY: cur est dans l'arbre.
        cur = unsafe { ctl_walk_next(cur) };
    }
    core::ptr::null_mut()
}

/// # Safety
/// `r` est nul, ou non nul avec un arbre fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_find(r: *mut t_ctlroot, id: u32) -> *mut t_ctl {
    if r.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: r est vivant.
    unsafe { ctl_find_id(r, id) }
}

/// # Safety
/// `c` est non nul. La chaîne des parents est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_depth(c: *const t_ctl) -> u32 {
    let mut d = 0u32;
    // SAFETY: c est non nul.
    let mut cur = unsafe { &*c };
    while !cur.parent.is_null() {
        d += 1;
        // SAFETY: parent n'est pas nul.
        cur = unsafe { &*cur.parent };
    }
    d
}

/// # Safety
/// `r` est non nul. `c` est nul ou dans un arbre fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_in_tree(r: *const t_ctlroot, c: *const t_ctl) -> bool {
    // SAFETY: r est non nul.
    let root = unsafe { &*r }.root;
    if root.is_null() || c.is_null() {
        return false;
    }
    let mut cur = c;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    while !unsafe { &*cur }.parent.is_null() {
        // SAFETY: cur n'est pas nul, parent non plus.
        cur = unsafe { &*cur }.parent;
    }
    cur == root
}

/// # Safety
/// `r` est non nul, son arbre est fini. `c` est un pointeur de contrôle ou nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_alive(r: *const t_ctlroot, c: *const t_ctl) -> bool {
    // SAFETY: r est non nul.
    let mut cur = unsafe { &*r }.root;
    while !cur.is_null() {
        if core::ptr::eq(cur, c) {
            return true;
        }
        // SAFETY: cur est dans l'arbre.
        cur = unsafe { ctl_walk_next(cur) };
    }
    false
}

/// # Safety
/// `parent` et `c` sont non nuls. `parent` n'a pas de cycle de frères.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_link_last(parent: *mut t_ctl, c: *mut t_ctl) {
    // SAFETY: les deux pointeurs sont non nuls.
    let parent = unsafe { &mut *parent };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let c = unsafe { &mut *c };
    c.parent = parent;
    c.next = core::ptr::null_mut();
    if parent.first.is_null() {
        parent.first = c;
        return;
    }
    let mut last = parent.first;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    while !unsafe { &*last }.next.is_null() {
        // SAFETY: last n'est pas nul.
        last = unsafe { &*last }.next;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { &mut *last }.next = c;
}

/// # Safety
/// `c` est non nul. S'il a un parent, la liste des frères est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_unlink(c: *mut t_ctl) {
    // SAFETY: c est non nul.
    let node = unsafe { &*c };
    if node.parent.is_null() {
        return;
    }
    // SAFETY: parent n'est pas nul.
    let parent = unsafe { &mut *node.parent };
    if parent.first == c {
        parent.first = node.next;
        return;
    }
    let mut prev = parent.first;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    while !prev.is_null() && unsafe { &*prev }.next != c {
        // SAFETY: prev n'est pas nul.
        prev = unsafe { &*prev }.next;
    }
    if !prev.is_null() {
        // SAFETY: prev n'est pas nul.
        unsafe { &mut *prev }.next = node.next;
    }
}

/// # Safety
/// `c` est non nul et a été alloué par `ctl_alloc`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_free_one(c: *mut t_ctl) {
    // SAFETY: c est non nul.
    let ty = unsafe { &*c }.r#type;
    // SAFETY: ty est l'entier du contrôle.
    let ops = unsafe { ctl_ops_get(ty) };
    if !ops.is_null() {
        // SAFETY: ops n'est pas nul.
        if let Some(destroy) = unsafe { &*ops }.destroy {
            // SAFETY: c est le contrôle dont le type a ce destructeur.
            unsafe { destroy(c) };
        }
    }
    // SAFETY: c vient de ctl_alloc.
    unsafe { ctl_free(c.cast()) };
}

/// # Safety
/// `top` est non nul. Son sous-arbre est fini et chaque nœud vient de `ctl_alloc`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_free_subtree(top: *mut t_ctl) {
    let mut cur = top;
    while !cur.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        while !unsafe { &*cur }.first.is_null() {
            // SAFETY: cur n'est pas nul.
            cur = unsafe { &*cur }.first;
        }
        if cur == top {
            // SAFETY: cur vient de ctl_alloc.
            unsafe { ctl_free_one(cur) };
            return;
        }
        // SAFETY: cur a un parent, sinon il serait top.
        let up = unsafe { &*cur }.parent;
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { &mut *up }.first = unsafe { &*cur }.next;
        // SAFETY: cur vient de ctl_alloc.
        unsafe { ctl_free_one(cur) };
        cur = up;
    }
}

/// # Safety
/// `r` est un rectangle passé par valeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_rect_ok(r: t_rect) -> bool {
    if r.x < -CTL_COORD_MAX || r.x > CTL_COORD_MAX {
        return false;
    }
    if r.y < -CTL_COORD_MAX || r.y > CTL_COORD_MAX {
        return false;
    }
    r.w >= 0 && r.w <= CTL_COORD_MAX && r.h >= 0 && r.h <= CTL_COORD_MAX
}

/// # Safety
/// `r` est non nul, `r.surface` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_dirty_add(r: *mut t_ctlroot, rect: t_rect) {
    // SAFETY: r et sa surface sont non nuls.
    let root = unsafe { &mut *r };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let surf = unsafe { &*root.surface };
    let bounds = rect_make(0, 0, surf.w, surf.h);
    let rect = rect_intersect(rect, bounds);
    if !rect_empty(rect) {
        region_add(&mut root.dirty, rect);
    }
}

/// # Safety
/// `r` et `c` sont nuls ou vivants. L'arbre est fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_invalidate(r: *mut t_ctlroot, c: *mut t_ctl) {
    if r.is_null() || c.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() {
        return;
    }
    // SAFETY: r et c sont vivants.
    if !unsafe { ctl_in_tree(r, c) } {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { &*c }.rect;
    // SAFETY: r a une surface, posée par ctl_root_init.
    unsafe { ctl_dirty_add(r, rect) };
}

/// # Safety
/// `r` est non nul et `r.root` n'est pas nul. `r.root.priv` est un `t_rootpriv`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_rootpriv(r: *const t_ctlroot) -> *mut t_rootpriv {
    // SAFETY: r et root sont non nuls.
    unsafe { &*(*r).root }.r#priv.cast()
}

fn make_ok(spec: &t_ctlspec) -> bool {
    // SAFETY: spec.type est un entier.
    let ops = unsafe { ctl_ops_get(spec.r#type) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if ops.is_null() || !unsafe { &*ops }.supported {
        return false;
    }
    // SAFETY: rect est une valeur.
    unsafe { ctl_rect_ok(spec.rect) }
}

/// # Safety
/// `spec` est non nul. `extra` est la taille ajoutée au privé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_make(spec: *const t_ctlspec, extra: usize) -> *mut t_ctl {
    // SAFETY: spec est non nul.
    let spec = unsafe { &*spec };
    if !make_ok(spec) {
        return core::ptr::null_mut();
    }
    // SAFETY: type valide, ops non nul.
    let ops = unsafe { &*ctl_ops_get(spec.r#type) };
    let priv_n = ops.priv_size + extra;
    // SAFETY: taille du bloc demandée à l'allocateur.
    let c = unsafe { ctl_alloc(core::mem::size_of::<t_ctl>() + priv_n) }.cast::<t_ctl>();
    if c.is_null() {
        return core::ptr::null_mut();
    }
    let init = ops.init;
    {
        // SAFETY: c vient de ctl_alloc et couvre t_ctl plus priv_n.
        let node = unsafe { &mut *c };
        node.r#type = spec.r#type;
        node.id = spec.id;
        node.flags = spec.flags & CTL_FLAGS_ALL;
        node.rect = spec.rect;
        if priv_n != 0 {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            node.r#priv = unsafe { c.cast::<u8>().add(core::mem::size_of::<t_ctl>()) }.cast();
        }
        // SAFETY: text a CTL_TEXT_MAX octets, spec.text est nul ou terminé.
        unsafe { ctl_copy_text(node.text_c_mut(), spec.text) };
    }
    if let Some(init) = init {
        // SAFETY: c est construit. Le prêt du contrôle est fini avant init.
        unsafe { init(c) };
    }
    c
}

fn parent_ok(r: *const t_ctlroot, parent: *const t_ctl) -> bool {
    // SAFETY: r et parent sont non nuls ici.
    if unsafe { !ctl_in_tree(r, parent) } {
        return false;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let ty = unsafe { &*parent }.r#type;
    if ty != CT_PANEL && ty != CT_GROUP {
        return false;
    }
    // SAFETY: parent est dans l'arbre.
    unsafe { ctl_depth(parent) + 1 < CTL_DEPTH_MAX }
}

fn spec_ok(r: *const t_ctlroot, spec: &t_ctlspec) -> bool {
    if spec.r#type >= CT_TYPES || spec.r#type == CT_MENU {
        return false;
    }
    if spec.id != 0 {
        // SAFETY: r est non nul.
        if !unsafe { ctl_find_id(r, spec.id) }.is_null() {
            return false;
        }
    }
    true
}

/// # Safety
/// `r`, `parent` et `spec` sont nuls ou vivants. L'arbre est fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_add(
    r: *mut t_ctlroot,
    parent: *mut t_ctl,
    spec: *const t_ctlspec,
) -> *mut t_ctl {
    if r.is_null() || spec.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() {
        return core::ptr::null_mut();
    }
    let mut parent = parent;
    if parent.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        parent = unsafe { &*r }.root;
    }
    // SAFETY: spec n'est pas nul.
    let spec_r = unsafe { &*spec };
    if !parent_ok(r, parent) || !spec_ok(r, spec_r) {
        return core::ptr::null_mut();
    }
    // SAFETY: spec est vivant.
    let c = unsafe { ctl_make(spec, 0) };
    if c.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: parent et c sont non nuls.
    unsafe { ctl_link_last(parent, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { &*c }.rect;
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, rect) };
    c
}

/// # Safety
/// `r` est non nul et `r.root` n'est pas nul. Les menus sont des fils directs de la racine.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_forget(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: r est non nul.
    let root = unsafe { &mut *r };
    // SAFETY: focus, hot, capture sont nuls ou dans l'arbre.
    if !root.focus.is_null() && unsafe { ctl_is_within(root.focus, c) } {
        root.focus = core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !root.hot.is_null() && unsafe { ctl_is_within(root.hot, c) } {
        root.hot = core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !root.capture.is_null() && unsafe { ctl_is_within(root.capture, c) } {
        root.capture = core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mut kid = unsafe { &*root.root }.first;
    while !kid.is_null() {
        // SAFETY: kid n'est pas nul.
        let node = unsafe { &mut *kid };
        if node.r#type == CT_MENU && !node.r#priv.is_null() {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            let menu = unsafe { &mut *node.r#priv.cast::<crate::abi::t_menu>() };
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            if !menu.prev.is_null() && unsafe { ctl_is_within(menu.prev, c) } {
                menu.prev = core::ptr::null_mut();
            }
        }
        kid = node.next;
    }
}

/// # Safety
/// `r` est non nul. Les pointeurs focus, hot et capture sont nuls ou dans l'arbre.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_refs_check(r: *mut t_ctlroot) {
    // SAFETY: r est non nul.
    let root = unsafe { &mut *r };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !root.focus.is_null() && unsafe { !ctl_usable(root.focus) } {
        root.focus = core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !root.hot.is_null() && unsafe { !ctl_usable(root.hot) } {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { &mut *root.hot }.state &= !CTL_ST_HOT;
        root.hot = core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if !root.capture.is_null() && unsafe { !ctl_usable(root.capture) } {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { &mut *root.capture }.state &= !CTL_ST_PRESSED;
        root.capture = core::ptr::null_mut();
    }
}

/// # Safety
/// `r` est non nul. Les menus fermés sont des fils directs alloués par `ctl_alloc`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_reap(r: *mut t_ctlroot) {
    // SAFETY: r est non nul.
    if unsafe { &*r }.root.is_null() {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mut kid = unsafe { &*(*r).root }.first;
    while !kid.is_null() {
        // SAFETY: kid n'est pas nul.
        let node = unsafe { &*kid };
        let next = node.next;
        if node.r#type == CT_MENU && node.state & CTL_ST_CLOSED != 0 {
            // SAFETY: kid est dans l'arbre.
            unsafe { ctl_remove(r, kid) };
        }
        kid = next;
    }
}

/// # Safety
/// `r` et `c` sont nuls ou vivants. `c` n'est pas la racine si on le retire.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_remove(r: *mut t_ctlroot, c: *mut t_ctl) {
    if r.is_null() || c.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    let rootp = unsafe { &*r }.root;
    if rootp.is_null() || c == rootp {
        return;
    }
    // SAFETY: r et c sont vivants.
    if unsafe { !ctl_in_tree(r, c) } {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { &*c }.rect;
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, rect) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_forget(r, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_unlink(c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_free_subtree(c) };
}

/// # Safety
/// `r` et `surface` sont nuls ou vivants. `command` est un rappel C ou nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_root_init(
    r: *mut t_ctlroot,
    surface: *mut t_surface,
    command: t_ctlcb,
) -> c_int {
    if r.is_null() || surface.is_null() {
        return E_INVAL;
    }
    szero(r.cast(), core::mem::size_of::<t_ctlroot>());
    // SAFETY: surface n'est pas nul.
    let surf = unsafe { &*surface };
    let spec = t_ctlspec {
        r#type: CT_PANEL,
        id: 0,
        flags: CTL_VISIBLE | CTL_ENABLED,
        rect: rect_make(0, 0, surf.w, surf.h),
        text: core::ptr::null(),
    };
    // SAFETY: rect est une valeur.
    if unsafe { !ctl_rect_ok(spec.rect) } {
        return E_INVAL;
    }
    // SAFETY: spec est local et vivant.
    let made = unsafe { ctl_make(&spec, core::mem::size_of::<t_rootpriv>()) };
    if made.is_null() {
        return E_NOMEM;
    }
    // SAFETY: r a été mis à zéro et reste vivant.
    let root = unsafe { &mut *r };
    root.root = made;
    root.surface = surface;
    root.command = command;
    // SAFETY: surface est posée.
    unsafe { ctl_dirty_add(r, spec.rect) };
    0
}

/// # Safety
/// `r` est nul ou a été initialisé. L'arbre vient de `ctl_alloc`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_root_destroy(r: *mut t_ctlroot) {
    if r.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let top = unsafe { &*r }.root;
    // SAFETY: top est la racine allouée.
    unsafe { ctl_free_subtree(top) };
    szero(r.cast(), core::mem::size_of::<t_ctlroot>());
}

/// # Safety
/// `r` est nul ou initialisé. `now` est un rappel C ou nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_set_clock(r: *mut t_ctlroot, now: t_ctlclock) {
    if r.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() {
        return;
    }
    // SAFETY: le privé de la racine est un t_rootpriv.
    unsafe { &mut *ctl_rootpriv(r) }.clock = now;
}

fn same_rect(a: t_rect, b: t_rect) -> bool {
    a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h
}

/// # Safety
/// `r` et `c` sont nuls ou vivants. `text` est nul ou terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_set_text(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    text: *const core::ffi::c_char,
) {
    if r.is_null() || c.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() || unsafe { !ctl_in_tree(r, c) } {
        return;
    }
    let mut tmp = [0u8; crate::abi::CTL_TEXT_MAX];
    // SAFETY: tmp a CTL_TEXT_MAX octets, text est nul ou terminé.
    unsafe { ctl_copy_text(tmp.as_mut_ptr().cast(), text) };
    // SAFETY: c est non nul, son texte est terminé.
    if scmp(tmp.as_ptr().cast(), unsafe { &*c }.text_c()) == 0 {
        return;
    }
    let n = slen(tmp.as_ptr().cast()) as usize + 1;
    // SAFETY: c.text a la place de la copie, bornée par ctl_copy_text.
    scopy_distinct(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { &mut *c }.text.as_mut_ptr().cast(),
        tmp.as_ptr().cast(),
        n,
    );
    // SAFETY: c est non nul.
    if unsafe { &*c }.r#type == CT_EDIT {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let priv_ = unsafe { &*c }.r#priv;
        // SAFETY: un edit a un t_edit en privé.
        unsafe { ctl_edt_clamp(c, priv_.cast()) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { &*c }.rect;
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, rect) };
}

/// # Safety
/// `r` et `c` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_set_flag(r: *mut t_ctlroot, c: *mut t_ctl, flag: u32, on: bool) {
    if r.is_null() || c.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() || unsafe { !ctl_in_tree(r, c) } {
        return;
    }
    // SAFETY: c est non nul.
    let node = unsafe { &mut *c };
    let old = node.flags;
    let flag = flag & CTL_FLAGS_ALL;
    if on {
        node.flags |= flag;
    } else {
        node.flags &= !flag;
    }
    if node.r#type == CT_RADIO && on && flag & CTL_CHECKED != 0 {
        // SAFETY: c est un radio dans l'arbre.
        unsafe { ctl_radio_select(r, c) };
    }
    if (old ^ node.flags) & CTL_FLAGS_VISUAL != 0 {
        let rect = node.rect;
        // SAFETY: r a une surface.
        unsafe { ctl_dirty_add(r, rect) };
    }
    // SAFETY: r est vivant.
    unsafe { ctl_refs_check(r) };
}

/// # Safety
/// `r` et `c` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_set_rect(r: *mut t_ctlroot, c: *mut t_ctl, rect: t_rect) -> c_int {
    if r.is_null() || c.is_null() {
        return E_INVAL;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() || unsafe { !ctl_in_tree(r, c) } {
        return E_INVAL;
    }
    // SAFETY: rect est une valeur.
    if unsafe { !ctl_rect_ok(rect) } {
        return E_INVAL;
    }
    // SAFETY: c est non nul.
    if same_rect(rect, unsafe { &*c }.rect) {
        return 0;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let old = unsafe { &*c }.rect;
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, old) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { &mut *c }.rect = rect;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { &*c }.r#type == CT_EDIT {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let priv_ = unsafe { &*c }.r#priv;
        // SAFETY: un edit a un t_edit.
        unsafe { ctl_edt_scroll(c, priv_.cast()) };
    }
    0
}

/// # Safety
/// `r` et `c` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_set_font(r: *mut t_ctlroot, c: *mut t_ctl, font: c_int) {
    if r.is_null() || c.is_null() {
        return;
    }
    // SAFETY: r n'est pas nul.
    if unsafe { &*r }.root.is_null() || unsafe { !ctl_in_tree(r, c) } {
        return;
    }
    if font as u32 > CTL_FONT_AUTO {
        return;
    }
    let mut next = 0u32;
    if (font as u32) < FONT_IDS {
        next = font as u32 + 1;
    }
    // SAFETY: c est non nul. Le prêt s'arrête avant le défilement.
    let (ty, priv_, rect) = unsafe {
        let node = &mut *c;
        if next == node.font {
            return;
        }
        node.font = next;
        (node.r#type, node.r#priv, node.rect)
    };
    if ty == CT_EDIT {
        // SAFETY: un edit a un t_edit.
        unsafe { ctl_edt_scroll(c, priv_.cast()) };
    }
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, rect) };
}

/// # Safety
/// `r` est non nul, sa racine est visible ou non. `p` est un point.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_hit(r: *const t_ctlroot, p: t_point) -> *mut t_ctl {
    // SAFETY: r est non nul.
    let mut cur = unsafe { &*r }.root;
    if cur.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: cur n'est pas nul.
    let node = unsafe { &*cur };
    if node.flags & CTL_VISIBLE == 0 || !rect_contains(node.rect, p) {
        return core::ptr::null_mut();
    }
    loop {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let mut kid = unsafe { &*cur }.first;
        let mut found = core::ptr::null_mut();
        while !kid.is_null() {
            // SAFETY: kid n'est pas nul.
            let k = unsafe { &*kid };
            if k.flags & CTL_VISIBLE != 0 && rect_contains(k.rect, p) {
                found = kid;
            }
            kid = k.next;
        }
        if found.is_null() {
            return cur;
        }
        cur = found;
    }
}
