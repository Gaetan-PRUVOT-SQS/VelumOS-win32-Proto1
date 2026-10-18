use core::ffi::{c_char, c_int, c_void};

use crate::abi::{pod_size, state};
use crate::geom::{rect_empty, rect_intersect, rect_make};
use crate::low::read_at;
use crate::sys::surface_pages;
use crate::{
    CUR_HAND, E_BUSY, E_INVAL, E_IO, E_NOMEM, E_NOSPC, E_OK, E_PROTO, E_RANGE, ICON_IDS,
    WM_MSG_MAX, WM_PORT_NAME, WM_RECTS_MAX, WM_VERSION, WM_WIN_PER_CLIENT, WMC_ACTIVATE,
    WMC_CAPTURE, WMC_CREATE, WMC_DESTROY, WMC_DIM_MAX, WMC_HELLO, WMC_PRESENT, WMC_SET_CURSOR,
    WMC_SET_ICON, WMC_SET_RECT, WMC_SET_STATE, WMC_SET_THEME, WMC_SET_TITLE, WMC_SET_WORKAREA,
    WMC_SUBSCRIBE, WMS_CREATED, WMS_HELLO_OK, WMS_RESIZED, WMS_THEME, WSTATE_HIDDEN, t_rect,
    t_surface, t_wmcmsg, t_wmcreate, t_wmcreated, t_wmhdr, t_wmhello, t_wmpresent, t_wmrect,
    t_wmtheme, t_wmtitle, t_wmwin, wmc_detach, wmc_hdr, wmc_recv, wmc_send, wmc_simple, wmc_slot,
    wmc_stash_pop, wmc_theme_apply, wmc_title_copy, wmc_wait, wmsys_close, wmsys_connect,
    wmsys_map,
};

fn created_ok(handle: u32, c: &t_wmcreated) -> bool {
    handle != 0
        && c.width != 0
        && c.height != 0
        && c.width <= c.stride
        && c.stride <= WMC_DIM_MAX
        && c.height <= WMC_DIM_MAX
}

fn set_surface(s: &mut t_surface, c: &t_wmcreated, px: *mut c_void) {
    let w = i32::try_from(c.width).unwrap_or(0);
    let h = i32::try_from(c.height).unwrap_or(0);
    let stride = i32::try_from(c.stride).unwrap_or(0);
    s.px = px.cast();
    s.w = w;
    s.h = h;
    s.stride = stride;
    s.clip = rect_make(0, 0, w, h);
}

fn hello() -> c_int {
    let mut rq = t_wmhello::ZERO;
    unsafe {
        // SAFETY: rq est local et exclusif.
        wmc_hdr(&mut rq.h, WMC_HELLO, pod_size::<t_wmhello>(), 0);
        rq.version = WM_VERSION;
        let mut r = wmc_send(core::ptr::from_mut(&mut rq).cast());
        if r >= 0 {
            let rx = core::ptr::addr_of_mut!((*state()).rx);
            r = wmc_wait(r as u32, WMS_HELLO_OK, rx);
        }
        r
    }
}

fn bind(w: *mut t_wmwin, style: u32) -> c_int {
    unsafe {
        // SAFETY: w a été vérifié non nul et aligné par wmc_create.
        core::ptr::write(w, t_wmwin::ZERO);
        let st = state();
        let rx = core::ptr::addr_of!((*st).rx);
        let hdr = read_at::<t_wmhdr>(&(*rx).buf);
        (*w).id = hdr.window;
        (*w).chan = (*st).chan;
        (*w).style = style;
        let r = wmc_attach(w, rx);
        if r < 0 {
            let id = (*w).id;
            wmc_simple(WMC_DESTROY, id, 0);
        }
        r
    }
}

fn fill_rects(m: &mut t_wmpresent, bound: t_rect, rects: *const t_rect, n: u32) -> u32 {
    m.nrects = 0;
    let mut i = 0u32;
    while i < n && m.nrects < WM_RECTS_MAX {
        let raw = unsafe {
            // SAFETY: rects a n éléments, i < n, l'appelant a vérifié rects non nul.
            rects.add(i as usize).read_unaligned()
        };
        let hit = rect_intersect(raw, bound);
        if !rect_empty(hit) {
            let slot = m.nrects as usize;
            m.rects[slot] = hit;
            m.nrects += 1;
        }
        i += 1;
    }
    i
}

fn ok_or(r: c_int) -> c_int {
    if r < 0 { r } else { E_OK }
}

/// # Safety
/// Aucun pointeur. Ferme les surfaces, les descripteurs de la file, puis le canal.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_disconnect() {
    let st = state();
    unsafe {
        // SAFETY: st est g_wmc. Les fenêtres non nulles sont celles enregistrées par wmc_create.
        let mut k = 0usize;
        while k < WM_WIN_PER_CLIENT {
            let w = (*st).wins[k];
            if !w.is_null() {
                wmc_detach(w);
            }
            k += 1;
        }
        loop {
            let mut m = t_wmcmsg::ZERO;
            if !wmc_stash_pop(&mut m) {
                break;
            }
            if m.handle != 0 {
                wmsys_close(m.handle);
            }
        }
        if (*st).chan != 0 {
            wmsys_close((*st).chan);
        }
        st.write(crate::t_wmcstate::ZERO);
    }
}

/// # Safety
/// `info` est nul, ou bien il a la place d'un `t_wmhello`. Un pointeur nul est accepté.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_connect(info: *mut t_wmhello) -> c_int {
    let st = state();
    unsafe {
        // SAFETY: st est g_wmc. WM_PORT_NAME est une chaîne C statique.
        if (*st).chan != 0 {
            return E_BUSY;
        }
        let h = wmsys_connect(WM_PORT_NAME.as_ptr().cast());
        if h == 0 {
            return E_IO;
        }
        if h < 0 {
            return h as c_int;
        }
        st.write(crate::t_wmcstate::ZERO);
        (*st).chan = h as u32;
        let r = hello();
        if r < 0 {
            wmc_disconnect();
            return r;
        }
        let rx = core::ptr::addr_of!((*st).rx);
        let copied = read_at::<t_wmhello>(&(*rx).buf);
        (*st).info = copied;
        if !info.is_null() {
            info.write_unaligned(copied);
        }
        E_OK
    }
}

/// # Safety
/// Aucun pointeur. `window` nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_activate(window: u32) -> c_int {
    if window == 0 {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: pas de pointeur. wmc_simple construit son message.
        wmc_simple(WMC_ACTIVATE, window, 0)
    }
}

/// # Safety
/// Aucun pointeur. `on` nul envoie 0, toute autre valeur envoie 1.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_subscribe(on: u32) -> c_int {
    let value = u32::from(on != 0);
    unsafe {
        // SAFETY: pas de pointeur. wmc_simple construit son message.
        wmc_simple(WMC_SUBSCRIBE, 0, value)
    }
}

/// # Safety
/// Aucun pointeur. Renvoie le canal courant, ou 0 s'il n'y en a pas.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_channel() -> u32 {
    unsafe {
        // SAFETY: st est g_wmc, lecture d'un u32.
        (*state()).chan
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et exclusif. Nul ou `id` nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_destroy(w: *mut t_wmwin) -> c_int {
    if w.is_null() || !w.is_aligned() {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: w n'est pas nul et il est aligné.
        if (*w).id == 0 {
            return E_INVAL;
        }
        let id = (*w).id;
        let r = wmc_simple(WMC_DESTROY, id, 0);
        let k = wmc_slot(id);
        if k >= 0 {
            (*state()).wins[k as usize] = core::ptr::null_mut();
        }
        wmc_detach(w);
        (*w).id = 0;
        r
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et lisible. Nul ou `state` > `WSTATE_HIDDEN` donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_state(w: *mut t_wmwin, state: u32) -> c_int {
    if w.is_null() || !w.is_aligned() || state > WSTATE_HIDDEN {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: w n'est pas nul et il est aligné.
        wmc_simple(WMC_SET_STATE, (*w).id, state)
    }
}

/// # Safety
/// Aucun pointeur. `window` nul ou `state` > `WSTATE_HIDDEN` donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_state_id(window: u32, state: u32) -> c_int {
    if window == 0 || state > WSTATE_HIDDEN {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: pas de pointeur.
        wmc_simple(WMC_SET_STATE, window, state)
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et lisible. Nul ou `cursor` > `CUR_HAND` donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_cursor(w: *mut t_wmwin, cursor: u32) -> c_int {
    if w.is_null() || !w.is_aligned() || cursor > CUR_HAND {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: w n'est pas nul et il est aligné.
        wmc_simple(WMC_SET_CURSOR, (*w).id, cursor)
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et lisible. Nul ou `icon` >= `ICON_IDS` donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_icon(w: *mut t_wmwin, icon: u32) -> c_int {
    if w.is_null() || !w.is_aligned() || icon >= ICON_IDS {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: w n'est pas nul et il est aligné.
        wmc_simple(WMC_SET_ICON, (*w).id, icon)
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et lisible. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_capture(w: *mut t_wmwin, on: u32) -> c_int {
    if w.is_null() || !w.is_aligned() {
        return E_INVAL;
    }
    let value = u32::from(on != 0);
    unsafe {
        // SAFETY: w n'est pas nul et il est aligné.
        wmc_simple(WMC_CAPTURE, (*w).id, value)
    }
}

/// # Safety
/// `w` et `title` sont nuls, ou bien `w` est aligné et `title` est une chaîne C.
/// L'un ou l'autre nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_title(w: *mut t_wmwin, title: *const c_char) -> c_int {
    if w.is_null() || !w.is_aligned() || title.is_null() {
        return E_INVAL;
    }
    let mut m = t_wmtitle::ZERO;
    unsafe {
        // SAFETY: w et m sont alignés. title n'est pas nul.
        wmc_hdr(&mut m.h, WMC_SET_TITLE, pod_size::<t_wmtitle>(), (*w).id);
        wmc_title_copy(m.title.as_mut_ptr().cast(), title);
        ok_or(wmc_send(core::ptr::from_mut(&mut m).cast()))
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et lisible. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_rect(w: *mut t_wmwin, rect: t_rect) -> c_int {
    if w.is_null() || !w.is_aligned() {
        return E_INVAL;
    }
    let mut m = t_wmrect::ZERO;
    unsafe {
        // SAFETY: w et m sont alignés.
        wmc_hdr(&mut m.h, WMC_SET_RECT, pod_size::<t_wmrect>(), (*w).id);
        m.rect = rect;
        ok_or(wmc_send(core::ptr::from_mut(&mut m).cast()))
    }
}

/// # Safety
/// Aucun pointeur. Le rectangle part tel quel.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_workarea(rect: t_rect) -> c_int {
    let mut m = t_wmrect::ZERO;
    unsafe {
        // SAFETY: m est local et exclusif.
        wmc_hdr(&mut m.h, WMC_SET_WORKAREA, pod_size::<t_wmrect>(), 0);
        m.rect = rect;
        ok_or(wmc_send(core::ptr::from_mut(&mut m).cast()))
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et lisible. Nul, `id` nul, `dark` > 1 ou `reduced` > 1
/// donne `E_INVAL` sans envoi.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_set_theme(w: *const t_wmwin, dark: u32, reduced: u32) -> c_int {
    if w.is_null() || !w.is_aligned() {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: w n'est pas nul et il est aligné.
        if (*w).id == 0 || dark > 1 || reduced > 1 {
            return E_INVAL;
        }
        let mut t = t_wmtheme::ZERO;
        wmc_hdr(&mut t.h, WMC_SET_THEME, pod_size::<t_wmtheme>(), (*w).id);
        t.dark = dark;
        t.reduced = reduced;
        ok_or(wmc_send(core::ptr::from_mut(&mut t).cast()))
    }
}

/// # Safety
/// `w` est nul, ou bien aligné et lisible. Si `n` > 0, `rects` a `n` éléments.
/// `w` nul, ou `n` > 0 avec `rects` nul, donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_present(w: *mut t_wmwin, rects: *const t_rect, n: u32) -> c_int {
    if w.is_null() || !w.is_aligned() || (n > 0 && rects.is_null()) {
        return E_INVAL;
    }
    let mut done = 0u32;
    let mut r = E_OK;
    while r >= 0 && (done < n || (n == 0 && done == 0)) {
        let mut msg = t_wmpresent::ZERO;
        unsafe {
            // SAFETY: w n'est pas nul et il est aligné. msg est local.
            wmc_hdr(&mut msg.h, WMC_PRESENT, pod_size::<t_wmpresent>(), (*w).id);
            if n > 0 {
                let bound = rect_make(0, 0, (*w).surface.w, (*w).surface.h);
                let Some(left) = n.checked_sub(done) else {
                    return E_INVAL;
                };
                let advanced = fill_rects(&mut msg, bound, rects.add(done as usize), left);
                let Some(next) = done.checked_add(advanced) else {
                    return E_INVAL;
                };
                done = next;
            } else {
                done = 1;
            }
            if n == 0 || msg.nrects > 0 {
                r = wmc_send(core::ptr::from_mut(&mut msg).cast());
            }
        }
    }
    ok_or(r)
}

/// # Safety
/// `buf` est nul, ou bien il a `size` octets exclusifs. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_next(buf: *mut c_void, size: u32, timeout_ns: u64) -> c_int {
    if buf.is_null() {
        return E_INVAL;
    }
    let st = state();
    unsafe {
        // SAFETY: st est g_wmc. buf n'est pas nul et a `size` octets.
        let m = core::ptr::addr_of_mut!((*st).rx);
        if !wmc_stash_pop(m) {
            let r = wmc_recv(m, timeout_ns);
            if r < 0 {
                return r;
            }
        }
        wmc_on_message(m);
        let len = (*m).len;
        if size < len {
            return E_RANGE;
        }
        let n = len as usize;
        if n > WM_MSG_MAX {
            return E_PROTO;
        }
        core::ptr::copy_nonoverlapping((*m).buf.as_ptr().cast::<u8>(), buf.cast::<u8>(), n);
        len as c_int
    }
}

/// # Safety
/// `m` est nul, ou bien aligné et exclusif. Un pointeur nul est ignoré.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_on_message(m: *mut t_wmcmsg) {
    if m.is_null() || !m.is_aligned() {
        return;
    }
    unsafe {
        // SAFETY: m n'est pas nul et il est aligné.
        let h = read_at::<t_wmhdr>(&(*m).buf);
        if h.r#type == WMS_THEME {
            wmc_theme_apply(m);
        }
        if (*m).handle == 0 {
            return;
        }
        let mut k = -1i32;
        if h.r#type == WMS_RESIZED && h.window != 0 {
            k = wmc_slot(h.window);
        }
        if k >= 0 {
            let win = (*state()).wins[k as usize];
            wmc_attach(win, m);
        } else {
            wmsys_close((*m).handle);
        }
        (*m).handle = 0;
    }
}

/// # Safety
/// `w` et `m` sont nuls, ou bien alignés et exclusifs. L'un ou l'autre nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_attach(w: *mut t_wmwin, m: *const t_wmcmsg) -> c_int {
    if w.is_null() || !w.is_aligned() || m.is_null() || !m.is_aligned() {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: w et m ne sont pas nuls et ils sont alignés.
        let c = read_at::<t_wmcreated>(&(*m).buf);
        let handle = (*m).handle;
        if !created_ok(handle, &c) {
            wmsys_close(handle);
            return E_PROTO;
        }
        let Some(len) = surface_pages(c.stride, c.height) else {
            wmsys_close(handle);
            return E_NOMEM;
        };
        let px = wmsys_map(handle, len);
        if px.is_null() {
            wmsys_close(handle);
            return E_NOMEM;
        }
        wmc_detach(w);
        (*w).section = handle;
        set_surface(&mut (*w).surface, &c, px);
        E_OK
    }
}

/// # Safety
/// `w` et `rq` sont nuls, ou bien `w` est aligné et exclusif et `rq` est lisible.
/// L'un ou l'autre nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_create(w: *mut t_wmwin, rq: *const t_wmcreate) -> c_int {
    if w.is_null() || !w.is_aligned() || rq.is_null() {
        return E_INVAL;
    }
    unsafe {
        // SAFETY: w est aligné. rq n'est pas nul, son titre occupe 64 octets.
        let k = wmc_slot(0);
        if k < 0 {
            return E_NOSPC;
        }
        let mut msg = rq.read_unaligned();
        let style = msg.style;
        wmc_hdr(&mut msg.h, WMC_CREATE, pod_size::<t_wmcreate>(), 0);
        let title = (rq as *const u8)
            .add(core::mem::offset_of!(t_wmcreate, title))
            .cast::<c_char>();
        wmc_title_copy(msg.title.as_mut_ptr().cast(), title);
        let mut r = wmc_send(core::ptr::from_mut(&mut msg).cast());
        if r >= 0 {
            let rx = core::ptr::addr_of_mut!((*state()).rx);
            r = wmc_wait(r as u32, WMS_CREATED, rx);
        }
        if r < 0 {
            return r;
        }
        r = bind(w, style);
        if r == E_OK {
            (*state()).wins[k as usize] = w;
        }
        r
    }
}
