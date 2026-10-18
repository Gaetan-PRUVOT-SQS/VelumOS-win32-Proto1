use core::ffi::{c_char, c_int};

use crate::abi::{
    E_INVAL, MSG_BYTES, NOTIF_APP_MAX, NOTIF_MAGIC, NOTIF_TEXT_MAX, NOTIF_TITLE_MAX, NOTIFC_CLEAR,
    NOTIFC_DISMISS, NOTIFC_POST, t_notifmsg,
};

fn field_buf(field: &[u8], empty_ok: bool) -> bool {
    let mut n = 0usize;
    while n < field.len() && field[n] != 0 {
        n += 1;
    }
    if n == field.len() {
        return false;
    }
    if n == 0 {
        return empty_ok;
    }
    let mut i = 0usize;
    while i < n {
        if field[i] < 0x20 || field[i] == 0x7f {
            return false;
        }
        i += 1;
    }
    true
}

pub(crate) fn parts_ok(app: &[u8], title: &[u8], text: &[u8]) -> bool {
    field_buf(app, false) && field_buf(title, false) && field_buf(text, true)
}

fn field_ptr(p: *const c_char, max: usize, empty_ok: bool) -> bool {
    if p.is_null() || max > NOTIF_TEXT_MAX {
        return false;
    }
    let mut buf = [0u8; NOTIF_TEXT_MAX];
    let mut i = 0usize;
    while i < max {
        let b = unsafe {
            // SAFETY: p n'est pas nul et i < max. Lecture bornée comme strnlen.
            p.add(i).read() as u8
        };
        buf[i] = b;
        i += 1;
        if b == 0 {
            break;
        }
    }
    field_buf(&buf[..max], empty_ok)
}

fn copy_cstr(dst: &mut [u8], src: *const c_char) {
    if src.is_null() {
        if let Some(slot) = dst.first_mut() {
            *slot = 0;
        }
        return;
    }
    let mut tmp = [0u8; NOTIF_TEXT_MAX];
    let cap = dst.len().min(tmp.len());
    let mut i = 0usize;
    while i < cap {
        let b = unsafe {
            // SAFETY: src n'est pas nul et i < cap. Arrêt au premier NUL.
            src.add(i).read() as u8
        };
        tmp[i] = b;
        i += 1;
        if b == 0 {
            break;
        }
    }
    let mut j = 0usize;
    while j < i && j + 1 < dst.len() {
        dst[j] = tmp[j];
        if tmp[j] == 0 {
            return;
        }
        j += 1;
    }
    if j < dst.len() {
        dst[j] = 0;
    }
}

fn type_known(ty: u32) -> bool {
    (NOTIFC_POST..=NOTIFC_CLEAR).contains(&ty)
}

/// # Safety
/// `app`, `title` et `text` sont nuls, ou bien chacun est lisible jusqu'au premier NUL
/// ou sur `NOTIF_*_MAX` octets. Un pointeur nul n'est pas déréférencé et donne 0.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_fields_valid(
    app: *const c_char,
    title: *const c_char,
    text: *const c_char,
) -> c_int {
    if app.is_null() || title.is_null() || text.is_null() {
        return 0;
    }
    if field_ptr(app, NOTIF_APP_MAX, false)
        && field_ptr(title, NOTIF_TITLE_MAX, false)
        && field_ptr(text, NOTIF_TEXT_MAX, true)
    {
        1
    } else {
        0
    }
}

/// # Safety
/// `buf` est nul, ou bien il pointe `len` octets lisibles quand `len` vaut la taille
/// de `t_notifmsg`. `out` est nul, ou bien il est aligné et exclusif. Un pointeur nul
/// donne `E_INVAL` sans écriture. Si `len` est bon, `out` est recopié avant les contrôles.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_msg_decode(
    buf: *const core::ffi::c_void,
    len: u32,
    out: *mut t_notifmsg,
) -> c_int {
    if buf.is_null() || out.is_null() || len != MSG_BYTES {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: buf et out ne sont pas nuls, len couvre exactement une t_notifmsg.
        core::ptr::copy_nonoverlapping(buf.cast::<u8>(), out.cast::<u8>(), MSG_BYTES as usize);
    }
    let msg = unsafe {
        // SAFETY: out vient d'être entièrement écrit et reste exclusif.
        &*out
    };
    if msg.magic != NOTIF_MAGIC || msg.size != MSG_BYTES || !type_known(msg.r#type) {
        return E_INVAL;
    }
    if msg.r#type == NOTIFC_POST && !parts_ok(&msg.app, &msg.title, &msg.text) {
        return E_INVAL;
    }
    if msg.r#type == NOTIFC_DISMISS && msg.id == 0 {
        return E_INVAL;
    }
    0
}

/// # Safety
/// `m` est nul, ou bien il est aligné et exclusif. Un pointeur nul n'est pas écrit.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_msg_init(m: *mut t_notifmsg, ty: u32) {
    if m.is_null() {
        return;
    }
    let m = unsafe {
        // SAFETY: m n'est pas nul, il est aligné et exclusif.
        &mut *m
    };
    *m = t_notifmsg::zero();
    m.magic = NOTIF_MAGIC;
    m.r#type = ty;
    m.size = MSG_BYTES;
}

/// # Safety
/// `m` est nul, ou bien il est aligné et exclusif. `app`, `title` et `text` sont nuls,
/// ou bien lisibles comme pour `notif_fields_valid`. Un pointeur nul donne `E_INVAL`
/// et `m` n'est pas modifié. En échec de champ, `m` reste intact.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_msg_post(
    m: *mut t_notifmsg,
    app: *const c_char,
    title: *const c_char,
    text: *const c_char,
    icon: u32,
) -> c_int {
    if m.is_null()
        || unsafe {
            // SAFETY: les trois pointeurs sont nuls ou lisibles sur leur plafond.
            notif_fields_valid(app, title, text)
        } == 0
    {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: m n'est pas nul, aligné et exclusif.
        notif_msg_init(m, NOTIFC_POST);
    }
    let msg = unsafe {
        // SAFETY: m vient d'être initialisé et reste exclusif.
        &mut *m
    };
    copy_cstr(&mut msg.app, app);
    copy_cstr(&mut msg.title, title);
    copy_cstr(&mut msg.text, text);
    msg.icon = icon;
    0
}
