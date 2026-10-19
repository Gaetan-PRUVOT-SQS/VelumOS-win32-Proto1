use core::ffi::{c_char, c_int, c_void};

use crate::abi::{pod_size, state};
use crate::sys::{c_len, luna_apply};
use crate::{
    E_BADF, E_INVAL, E_OK, E_PIPE, E_PROTO, E_TIMEOUT, WM_BUF_WORDS, WM_MAGIC, WM_MSG_MAX,
    WM_REPLY_NS, WM_TITLE_MAX, WM_WIN_PER_CLIENT, WMC_ACTIVATE, WMC_DESTROY, WMC_STASH,
    WMS_ACTIVATE, WMS_CLOSE_REQ, WMS_CREATED, WMS_ERROR, WMS_HELLO_OK, WMS_KEY, WMS_MOUSE,
    WMS_PAINT, WMS_RESIZED, WMS_STATE, WMS_THEME, WMS_WIN_ADD, WMS_WIN_DEL, WMS_WIN_UPD, t_wmarg,
    t_wmcmsg, t_wmcreated, t_wmhdr, t_wmhello, t_wminfo, t_wmkey, t_wmmouse, t_wmtheme, t_wmwin,
    wmsys_close, wmsys_now, wmsys_recv, wmsys_send, wmsys_unmap,
};

pub(crate) fn read_at<T: Copy>(buf: &[u64; WM_BUF_WORDS]) -> T {
    const { assert!(core::mem::size_of::<T>() <= WM_MSG_MAX) };
    let mut out = core::mem::MaybeUninit::<T>::uninit();
    unsafe {
        // SAFETY: T tient dans les 320 octets du message. out est initialisé avant lecture.
        core::ptr::copy_nonoverlapping(
            buf.as_ptr().cast::<u8>(),
            out.as_mut_ptr().cast::<u8>(),
            core::mem::size_of::<T>(),
        );
        out.assume_init()
    }
}

fn expected(ty: u32) -> u32 {
    match ty {
        WMS_ERROR => pod_size::<t_wmhdr>(),
        WMS_THEME => pod_size::<t_wmtheme>(),
        WMS_HELLO_OK => pod_size::<t_wmhello>(),
        WMS_CREATED | WMS_RESIZED => pod_size::<t_wmcreated>(),
        WMS_KEY => pod_size::<t_wmkey>(),
        WMS_MOUSE => pod_size::<t_wmmouse>(),
        WMS_CLOSE_REQ | WMS_PAINT => pod_size::<t_wmhdr>(),
        WMS_ACTIVATE | WMS_STATE => pod_size::<t_wmarg>(),
        WMS_WIN_ADD | WMS_WIN_DEL | WMS_WIN_UPD => pod_size::<t_wminfo>(),
        _ => 0,
    }
}

fn reply_code(h: &t_wmhdr) -> c_int {
    if h.r#type != WMS_ERROR {
        return E_OK;
    }
    if h.status < 0 {
        return h.status;
    }
    E_PROTO
}

fn ring(a: u32, b: u32) -> u32 {
    match a.checked_add(b) {
        Some(v) => v % WMC_STASH,
        None => a.wrapping_add(b) % WMC_STASH,
    }
}

/// # Safety
/// `h` est nul, ou bien il a la place d'un `t_wmhdr`. Un pointeur nul n'est pas écrit.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_hdr(h: *mut t_wmhdr, ty: u32, size: u32, window: u32) {
    if h.is_null() {
        return;
    }
    let hdr = t_wmhdr {
        magic: WM_MAGIC,
        r#type: ty,
        size,
        window,
        seq: 0,
        status: 0,
    };
    unsafe {
        // SAFETY: h n'est pas nul. L'écriture couvre un t_wmhdr, même non aligné.
        h.write_unaligned(hdr);
    }
}

/// # Safety
/// `msg` est nul, ou bien il pointe un message dont le premier champ est un `t_wmhdr`
/// et dont `size` octets sont lisibles. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_send(msg: *mut c_void) -> c_int {
    if msg.is_null() {
        return E_INVAL;
    }
    let st = state();
    unsafe {
        // SAFETY: st est g_wmc, exclusif le temps de l'appel. msg n'est pas nul.
        if (*st).chan == 0 {
            return E_BADF;
        }
        if (*st).lost {
            return E_PIPE;
        }
        let mut seq = (*st).seq.wrapping_add(1);
        if seq == 0 || seq > 0x7fff_ffff {
            seq = 1;
        }
        (*st).seq = seq;
        let hdr_ptr = msg.cast::<t_wmhdr>();
        let mut hdr = hdr_ptr.read_unaligned();
        hdr.seq = seq;
        let size = hdr.size;
        hdr_ptr.write_unaligned(hdr);
        let r = wmsys_send((*st).chan, msg, size);
        if r == E_PIPE {
            (*st).lost = true;
        }
        if r < 0 {
            return r;
        }
        seq as c_int
    }
}

/// # Safety
/// `m` est nul, ou bien aligné et lisible. Un pointeur nul donne faux.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_valid(m: *const t_wmcmsg) -> bool {
    if m.is_null() || !m.is_aligned() {
        return false;
    }
    let m = unsafe {
        // SAFETY: m n'est pas nul et il est aligné.
        &*m
    };
    if m.len < pod_size::<t_wmhdr>() || m.len > WM_MSG_MAX as u32 {
        return false;
    }
    let h = read_at::<t_wmhdr>(&m.buf);
    if h.magic != WM_MAGIC || h.size != m.len || expected(h.r#type) != m.len {
        return false;
    }
    let with_handle = h.r#type == WMS_CREATED || h.r#type == WMS_RESIZED;
    with_handle == (m.handle != 0)
}

/// # Safety
/// `m` est nul, ou bien aligné et exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_recv(m: *mut t_wmcmsg, timeout_ns: u64) -> c_int {
    if m.is_null() || !m.is_aligned() {
        return E_INVAL;
    }
    let st = state();
    unsafe {
        // SAFETY: st est g_wmc. m n'est pas nul et il est aligné.
        if (*st).chan == 0 {
            return E_BADF;
        }
        if (*st).lost {
            return E_PIPE;
        }
        let r = wmsys_recv((*st).chan, m, timeout_ns);
        if r == E_PIPE {
            (*st).lost = true;
        }
        if r < 0 {
            return r;
        }
        if !wmc_valid(m) {
            if (*m).handle != 0 {
                wmsys_close((*m).handle);
            }
            (*m).handle = 0;
            return E_PROTO;
        }
        (*m).len as c_int
    }
}

/// # Safety
/// `out` est nul, ou bien aligné et exclusif. Un pointeur nul donne `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_wait(seq: u32, ty: u32, out: *mut t_wmcmsg) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    let first = unsafe {
        // SAFETY: wmsys_now n'a pas d'argument.
        wmsys_now()
    };
    let deadline = match first.checked_add(WM_REPLY_NS) {
        Some(v) => v,
        None => first.wrapping_add(WM_REPLY_NS),
    };
    let mut now = unsafe {
        // SAFETY: deuxième lecture, comme le C qui appelle wmsys_now deux fois.
        wmsys_now()
    };
    while now < deadline {
        let Some(left) = deadline.checked_sub(now) else {
            return E_TIMEOUT;
        };
        let st = state();
        let rx = unsafe {
            // SAFETY: st est g_wmc, rx en est un champ aligné.
            core::ptr::addr_of_mut!((*st).rx)
        };
        let r = unsafe {
            // SAFETY: rx est g_wmc.rx, aligné et exclusif le temps de l'appel.
            wmc_recv(rx, left)
        };
        if r < 0 && r != E_PROTO {
            return r;
        }
        let h = unsafe {
            // SAFETY: rx reste valide. buf contient 320 octets.
            read_at::<t_wmhdr>(&(*rx).buf)
        };
        if r >= 0 && h.seq == seq && (h.r#type == ty || h.r#type == WMS_ERROR) {
            unsafe {
                // SAFETY: rx et out sont alignés, non nuls, et la lecture précède l'écriture.
                let tmp = rx.read();
                out.write(tmp);
            }
            return reply_code(&h);
        }
        if r >= 0 {
            unsafe {
                // SAFETY: rx est aligné. La copie locale ne chevauche pas la file.
                let tmp = rx.read();
                wmc_stash_push(&tmp);
            }
        }
        now = unsafe {
            // SAFETY: wmsys_now n'a pas d'argument.
            wmsys_now()
        };
    }
    E_TIMEOUT
}

/// # Safety
/// `dst` est nul, ou bien il a `WM_TITLE_MAX` octets exclusifs. `src` est nul, ou bien
/// une chaîne C. Si les 63 premiers octets n'ont pas de NUL, l'octet 63 est lisible.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_title_copy(dst: *mut c_char, src: *const c_char) {
    if dst.is_null() {
        return;
    }
    unsafe {
        // SAFETY: dst a 64 octets exclusifs. src suit le contrat ci-dessus.
        core::ptr::write_bytes(dst, 0, WM_TITLE_MAX);
        if src.is_null() {
            return;
        }
        let mut n = 0usize;
        while n < WM_TITLE_MAX - 1 {
            let b = src.add(n).read();
            if b == 0 {
                break;
            }
            n += 1;
        }
        if n == WM_TITLE_MAX - 1 {
            while n > 0 && (src.add(n).read() as u8 & 0xc0) == 0x80 {
                n -= 1;
            }
        }
        if n > 0 {
            core::ptr::copy_nonoverlapping(src, dst, n);
        }
    }
}

/// # Safety
/// Aucun pointeur de l'appelant. Le message part sur le canal courant, ou `E_BADF` s'il est fermé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_simple(ty: u32, window: u32, value: u32) -> c_int {
    let mut m = t_wmarg::ZERO;
    let size = if ty == WMC_DESTROY || ty == WMC_ACTIVATE {
        pod_size::<t_wmhdr>()
    } else {
        pod_size::<t_wmarg>()
    };
    unsafe {
        // SAFETY: m est local, aligné et exclusif.
        wmc_hdr(&mut m.h, ty, size, window);
        m.value = value;
        let r = wmc_send(core::ptr::from_mut(&mut m).cast());
        if r < 0 {
            return r;
        }
    }
    E_OK
}

/// # Safety
/// `m` est nul, ou bien aligné et lisible. Un pointeur nul est ignoré.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_stash_push(m: *const t_wmcmsg) {
    if m.is_null() || !m.is_aligned() {
        return;
    }
    let st = state();
    unsafe {
        // SAFETY: st est g_wmc. m n'est pas nul et il est aligné.
        if (*st).n == WMC_STASH {
            let ty = read_at::<t_wmhdr>(&(*m).buf).r#type;
            if ty == WMS_MOUSE || ty == WMS_PAINT {
                return;
            }
            let idx = ((*st).head % WMC_STASH) as usize;
            let old = (*st).stash[idx].handle;
            if old != 0 {
                wmsys_close(old);
            }
            (*st).head = ring((*st).head, 1);
            (*st).n = (*st).n.saturating_sub(1);
        }
        let idx = ring((*st).head, (*st).n) as usize;
        (*st).stash[idx] = *m;
        (*st).n = match (*st).n.checked_add(1) {
            Some(v) => v,
            None => (*st).n.wrapping_add(1),
        };
    }
}

/// # Safety
/// `m` est nul, ou bien aligné et exclusif. Un pointeur nul donne faux.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_stash_pop(m: *mut t_wmcmsg) -> bool {
    if m.is_null() || !m.is_aligned() {
        return false;
    }
    let st = state();
    unsafe {
        // SAFETY: st est g_wmc. m n'est pas nul et il est aligné.
        if (*st).n == 0 {
            return false;
        }
        let idx = ((*st).head % WMC_STASH) as usize;
        *m = (*st).stash[idx];
        (*st).head = ring((*st).head, 1);
        (*st).n = (*st).n.saturating_sub(1);
        true
    }
}

/// # Safety
/// Aucun pointeur de l'appelant. Les fenêtres lues sont celles de `g_wmc` ou nulles.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_slot(id: u32) -> c_int {
    let st = state();
    let mut k = 0i32;
    while (k as usize) < WM_WIN_PER_CLIENT {
        let w = unsafe {
            // SAFETY: st est g_wmc et k est dans le tableau.
            (*st).wins[k as usize]
        };
        if id == 0 && w.is_null() {
            return k;
        }
        if id != 0 && !w.is_null() && !w.is_aligned() {
            k += 1;
            continue;
        }
        if id != 0 && !w.is_null() {
            let wid = unsafe {
                // SAFETY: w vient de g_wmc.wins et n'est pas nul. L'appelant le garde vivant.
                (*w).id
            };
            if wid == id {
                return k;
            }
        }
        k += 1;
    }
    -1
}

/// # Safety
/// `w` est nul, ou bien aligné et exclusif. Un pointeur nul est ignoré.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_detach(w: *mut t_wmwin) {
    if w.is_null() || !w.is_aligned() {
        return;
    }
    unsafe {
        // SAFETY: w n'est pas nul et il est aligné.
        let len = c_len((*w).surface.stride, (*w).surface.h);
        if !(*w).surface.px.is_null() {
            wmsys_unmap((*w).surface.px.cast(), len);
        }
        if (*w).section != 0 {
            wmsys_close((*w).section);
        }
        (*w).surface = crate::t_surface::ZERO;
        (*w).section = 0;
    }
}

/// # Safety
/// `m` est nul, ou bien aligné et lisible. Un pointeur nul est ignoré.
/// `dark` et `reduced` hors 0 ou 1, ou `reserved` non nul, n'appellent pas `wm_luna_apply`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmc_theme_apply(m: *const t_wmcmsg) {
    if m.is_null() || !m.is_aligned() {
        return;
    }
    unsafe {
        // SAFETY: m n'est pas nul et il est aligné. Le tampon fait 320 octets.
        if (*m).len != pod_size::<t_wmtheme>() {
            return;
        }
        let t = read_at::<t_wmtheme>(&(*m).buf);
        if t.dark > 1 || t.reduced > 1 || t.reserved != 0 {
            return;
        }
        luna_apply(t.dark, t.reduced);
    }
}
