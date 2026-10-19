use core::ffi::c_int;

use crate::abi::{E_INVAL, E_NOENT, NOTIF_MAX, t_notif, t_notifstore};
use crate::wire::parts_ok;

fn id_fit(id: u32) -> u32 {
    if id == 0 || id > i32::MAX as u32 {
        1
    } else {
        id
    }
}

fn id_live(s: &t_notifstore, id: u32) -> bool {
    let mut i = 0u32;
    while i < s.count && (i as usize) < NOTIF_MAX {
        if s.items[i as usize].id == id {
            return true;
        }
        i += 1;
    }
    false
}

fn next_id(s: &mut t_notifstore, slot: usize) {
    let mut id = id_fit(s.next_id);
    let mut steps = 0u32;
    while id_live(s, id) && steps < NOTIF_MAX as u32 {
        if id == i32::MAX as u32 {
            id = 1;
        } else {
            id += 1;
        }
        steps += 1;
    }
    s.items[slot].id = id;
    if id == i32::MAX as u32 {
        s.next_id = 1;
    } else {
        s.next_id = id.wrapping_add(1);
    }
}

fn drop_oldest(s: &mut t_notifstore) {
    s.items.copy_within(1..NOTIF_MAX, 0);
    s.count = s.count.wrapping_sub(1);
    s.dropped = s.dropped.wrapping_add(1);
}

/// # Safety
/// `s` est nul, ou bien il est aligné et exclusif. Un pointeur nul n'est pas écrit.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_store_init(s: *mut t_notifstore) {
    if s.is_null() {
        return;
    }
    let s = unsafe {
        // SAFETY: s n'est pas nul, il est aligné et exclusif.
        &mut *s
    };
    *s = t_notifstore::zero();
    s.next_id = 1;
}

/// # Safety
/// `s` et `n` sont nuls, ou bien alignés. `s` est exclusif. Les deux ne se recouvrent pas.
/// Un pointeur nul donne `E_INVAL`. `count` au-dessus de `NOTIF_MAX` donne `E_INVAL`
/// sans écriture hors du tableau.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_store_post(s: *mut t_notifstore, n: *const t_notif) -> c_int {
    if s.is_null() || n.is_null() {
        return E_INVAL;
    }
    let src = unsafe {
        // SAFETY: n n'est pas nul et il est aligné. La copie précède toute écriture de s.
        *n
    };
    if !parts_ok(&src.app, &src.title, &src.text) {
        return E_INVAL;
    }
    let s = unsafe {
        // SAFETY: s n'est pas nul, aligné et exclusif. src est une copie locale.
        &mut *s
    };
    if s.count > NOTIF_MAX as u32 {
        return E_INVAL;
    }
    if s.count == NOTIF_MAX as u32 {
        drop_oldest(s);
    }
    let idx = s.count as usize;
    s.items[idx] = src;
    next_id(s, idx);
    s.count += 1;
    s.items[idx].id as c_int
}

/// # Safety
/// `s` est nul, ou bien il est aligné et exclusif. Un pointeur nul ou un id nul donne
/// `E_INVAL`. `count` au-dessus de `NOTIF_MAX` donne `E_INVAL` sans lecture hors tableau.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_store_dismiss(s: *mut t_notifstore, id: u32) -> c_int {
    if s.is_null() || id == 0 {
        return E_INVAL;
    }
    let s = unsafe {
        // SAFETY: s n'est pas nul, il est aligné et exclusif.
        &mut *s
    };
    if s.count > NOTIF_MAX as u32 {
        return E_INVAL;
    }
    let mut i = 0u32;
    while i < s.count && s.items[i as usize].id != id {
        i += 1;
    }
    if i == s.count {
        return E_NOENT;
    }
    let from = match (i as usize).checked_add(1) {
        Some(v) => v,
        None => return E_INVAL,
    };
    let count = s.count as usize;
    if from < count {
        s.items.copy_within(from..count, i as usize);
    }
    match s.count.checked_sub(1) {
        Some(n) => s.count = n,
        None => return E_INVAL,
    }
    s.items[s.count as usize] = t_notif::zero();
    0
}

/// # Safety
/// `s` est nul, ou bien il est aligné et exclusif. Un pointeur nul n'est pas écrit.
/// `next_id` et `dropped` sont conservés.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_store_clear(s: *mut t_notifstore) {
    if s.is_null() {
        return;
    }
    let s = unsafe {
        // SAFETY: s n'est pas nul, il est aligné et exclusif.
        &mut *s
    };
    let keep = s.next_id;
    s.items = [t_notif::zero(); NOTIF_MAX];
    s.count = 0;
    s.next_id = keep;
}

/// # Safety
/// `s` est nul, ou bien il est aligné. Un pointeur nul donne 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_store_count(s: *const t_notifstore) -> u32 {
    if s.is_null() {
        return 0;
    }
    unsafe {
        // SAFETY: s n'est pas nul et il est aligné.
        (*s).count
    }
}

/// # Safety
/// `s` est nul, ou bien il est aligné et reste vivant tant que le pointeur rendu sert.
/// Hors borne, compteur trop grand ou pointeur nul : retour nul, sans déréférencement.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_store_get(s: *const t_notifstore, index: u32) -> *const t_notif {
    if s.is_null() {
        return core::ptr::null();
    }
    let s = unsafe {
        // SAFETY: s n'est pas nul et il est aligné.
        &*s
    };
    if s.count > NOTIF_MAX as u32 || index >= s.count {
        return core::ptr::null();
    }
    let Some(pos) = s.count.checked_sub(1).and_then(|v| v.checked_sub(index)) else {
        return core::ptr::null();
    };
    let Some(pos) = usize::try_from(pos).ok() else {
        return core::ptr::null();
    };
    if pos >= NOTIF_MAX {
        return core::ptr::null();
    }
    &s.items[pos]
}
