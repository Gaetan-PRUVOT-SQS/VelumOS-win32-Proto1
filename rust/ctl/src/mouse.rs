use crate::abi::{
    BTN_LEFT, CTL_DBLCLK_DIST, CTL_DBLCLK_NS, CTL_MEV_DOWN, CTL_MEV_MOVE, CTL_MEV_NONE, CTL_MEV_UP,
    CTL_MEV_WHEEL, CTL_WHEEL_ROWS, INP_MOUSE_DOWN, INP_MOUSE_MOVE, INP_MOUSE_UP, INP_WHEEL, t_ctl,
    t_ctlroot, t_mouseev, t_point, t_wmmouse,
};
use crate::focus::ctl_focus;
use crate::notify::ctl_hot_set;
use crate::ops::ctl_ops_get;
use crate::tree::{ctl_hit, ctl_reap, ctl_rootpriv, ctl_usable};

fn kind_of(m: &t_wmmouse, was: u32) -> u32 {
    let left = 1u32 << BTN_LEFT;
    if m.r#type == INP_MOUSE_MOVE {
        return CTL_MEV_MOVE;
    }
    if m.r#type == INP_WHEEL {
        return CTL_MEV_WHEEL;
    }
    if m.r#type == INP_MOUSE_DOWN && m.buttons & !was & left != 0 {
        return CTL_MEV_DOWN;
    }
    if m.r#type == INP_MOUSE_UP && was & !m.buttons & left != 0 {
        return CTL_MEV_UP;
    }
    CTL_MEV_NONE
}

/// # Safety
/// `wheel` est le delta brut de la molette.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_wheel_delta(mut wheel: i32) -> i32 {
    wheel = wheel.clamp(-1024, 1024);
    wheel * CTL_WHEEL_ROWS
}

/// # Safety
/// `r` est non nul, initialisé, avec un `t_rootpriv`. `m` est non nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mouse_event(r: *mut t_ctlroot, m: *const t_wmmouse) -> t_mouseev {
    // SAFETY: r a un privé de racine, m est non nul.
    let rp = unsafe { &mut *ctl_rootpriv(r) };
    let was = rp.buttons;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let m = unsafe { &*m };
    rp.buttons = m.buttons;
    t_mouseev {
        kind: kind_of(m, was),
        at: t_point { x: m.x, y: m.y },
        wheel: m.wheel,
        dbl: false,
    }
}

fn near_click(rp: &crate::abi::t_rootpriv, e: &t_mouseev) -> bool {
    let mut dx = e.at.x - rp.click_at.x;
    let mut dy = e.at.y - rp.click_at.y;
    if dx < 0 {
        dx = -dx;
    }
    if dy < 0 {
        dy = -dy;
    }
    dx <= CTL_DBLCLK_DIST && dy <= CTL_DBLCLK_DIST
}

/// # Safety
/// `r` est initialisé. `c` et `e` sont non nuls. `c` reste vivant.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mouse_dbl(
    r: *mut t_ctlroot,
    c: *const t_ctl,
    e: *const t_mouseev,
) -> bool {
    // SAFETY: le privé de racine est un t_rootpriv.
    let clock = unsafe { (*ctl_rootpriv(r)).clock };
    let Some(clock) = clock else {
        return false;
    };
    // SAFETY: clock est le rappel posé par ctl_set_clock.
    let now = unsafe { clock() };
    // SAFETY: r, c et e sont non nuls.
    let rp = unsafe { &mut *ctl_rootpriv(r) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let e = unsafe { &*e };
    let dbl = rp.click_ctl == (c as usize)
        && near_click(rp, e)
        && now >= rp.click_ns
        && now - rp.click_ns <= CTL_DBLCLK_NS;
    rp.click_ctl = c as usize;
    rp.click_ns = now;
    rp.click_at = e.at;
    if dbl {
        rp.click_ctl = 0;
    }
    dbl
}

fn hot_of(t: *mut t_ctl) -> *mut t_ctl {
    if t.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: t n'est pas nul.
    let ty = unsafe { (*t).r#type };
    // SAFETY: ty est l'entier du contrôle.
    let ops = unsafe { ctl_ops_get(ty) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if ops.is_null() || !unsafe { (*ops).hot } || !unsafe { ctl_usable(t) } {
        return core::ptr::null_mut();
    }
    t
}

fn pick_target(r: *mut t_ctlroot, e: &t_mouseev) -> *mut t_ctl {
    // SAFETY: r n'est pas nul.
    let capture = unsafe { (*r).capture };
    if !capture.is_null() {
        return capture;
    }
    // SAFETY: r est initialisé.
    let t = unsafe { ctl_hit(r, e.at) };
    // SAFETY: r a une surface.
    unsafe { ctl_hot_set(r, hot_of(t)) };
    t
}

fn route(r: *mut t_ctlroot, m: *const t_wmmouse) -> bool {
    // SAFETY: r et m suivent ctl_mouse_event.
    let mut e = unsafe { ctl_mouse_event(r, m) };
    let t = pick_target(r, &e);
    if t.is_null() {
        return false;
    }
    // SAFETY: t est dans l'arbre.
    if unsafe { !ctl_usable(t) } {
        return true;
    }
    if e.kind == CTL_MEV_DOWN {
        // SAFETY: r, t et e sont vivants.
        e.dbl = unsafe { ctl_mouse_dbl(r, t, &e) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_focus(r, t) };
    }
    // SAFETY: t est non nul.
    let ops = unsafe { ctl_ops_get((*t).r#type) };
    // SAFETY: le type d'un contrôle créé a une entrée.
    let Some(mouse) = (unsafe { (*ops).mouse }) else {
        return false;
    };
    // SAFETY: mouse est l'entrée de la table.
    unsafe { mouse(r, t, &e) }
}

/// # Safety
/// `r` et `m` sont nuls ou vivants. L'arbre est fini.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mouse(r: *mut t_ctlroot, m: *const t_wmmouse) -> bool {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || m.is_null() || unsafe { (*r).root.is_null() } {
        return false;
    }
    let used = route(r, m);
    // SAFETY: r est initialisé.
    unsafe { ctl_reap(r) };
    used
}
