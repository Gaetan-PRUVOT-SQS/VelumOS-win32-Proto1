use core::ffi::{c_char, c_int, c_void};
#[cfg(test)]
use core::sync::atomic::{AtomicI32, AtomicU32, Ordering};

#[cfg(not(all(not(test), velum_host)))]
use crate::abi::{E_INVAL, E_PROTO, IPC_HANDLES_MAX, WM_MSG_MAX, t_chanrecv, t_chansend};
use crate::abi::{WM_BUF_WORDS, WMC_PAGE, pod_size, t_handle, t_wmcmsg};

#[cfg(test)]
const E_NOSYS: c_int = -38;

#[cfg(test)]
pub(crate) static SEND_RET: AtomicI32 = AtomicI32::new(E_NOSYS);
#[cfg(test)]
pub(crate) static CONNECT_RET: AtomicI32 = AtomicI32::new(E_IO_SPY);
#[cfg(test)]
pub(crate) static SENT_N: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static SENT_LEN: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static SENT_MAGIC: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static SENT_TYPE: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static SENT_SIZE: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static SENT_WIN: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static SENT_SEQ: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static SENT_VALUE: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static NAME_WM: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static LUNA_N: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static LUNA_DARK: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static LUNA_RED: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static CLOSE_N: AtomicU32 = AtomicU32::new(0);
#[cfg(test)]
pub(crate) static CLOSE_H: AtomicU32 = AtomicU32::new(0);

#[cfg(test)]
const E_IO_SPY: i32 = -5;

#[cfg(all(not(test), not(velum_host)))]
unsafe extern "C" {
    fn v_port_connect(name: *const c_char, flags: u32) -> i64;
    fn v_chan_send(chan: t_handle, msg: *const t_chansend) -> c_int;
    fn v_chan_recv(chan: t_handle, msg: *mut t_chanrecv) -> c_int;
    fn v_close(h: t_handle) -> c_int;
    fn v_time_mono() -> i64;
    fn v_section_map(section: t_handle, offset: u64, len: u64, prot: u32) -> i64;
    fn v_vfree(va: u64, len: u64) -> c_int;
}

#[cfg(not(test))]
unsafe extern "C" {
    fn wm_luna_apply(dark: u32, reduced: u32);
}

#[cfg(all(not(test), velum_host))]
unsafe extern "C" {
    pub fn wmsys_connect(name: *const c_char) -> i64;
    pub fn wmsys_send(ch: t_handle, msg: *const c_void, len: u32) -> c_int;
    pub fn wmsys_recv(ch: t_handle, m: *mut t_wmcmsg, timeout_ns: u64) -> c_int;
    pub fn wmsys_map(section: t_handle, len: u64) -> *mut c_void;
    pub fn wmsys_unmap(va: *mut c_void, len: u64);
    pub fn wmsys_close(h: t_handle);
    pub fn wmsys_now() -> u64;
}

#[cfg(test)]
fn name_is_wm(name: *const c_char) -> bool {
    if name.is_null() {
        return false;
    }
    unsafe {
        // SAFETY: name n'est pas nul. On s'arrête au premier NUL, au plus 3 octets.
        let a = name.read();
        if a == 0 {
            return false;
        }
        let b = name.add(1).read();
        if b == 0 {
            return false;
        }
        let c = name.add(2).read();
        a == b'w' as c_char && b == b'm' as c_char && c == 0
    }
}

#[cfg(not(all(not(test), velum_host)))]
fn raw_connect(name: *const c_char) -> i64 {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: name est nul ou une chaîne C. flags vaut 0, comme wmsys_connect.
        v_port_connect(name, 0)
    }
    #[cfg(test)]
    {
        NAME_WM.store(u32::from(name_is_wm(name)), Ordering::Relaxed);
        i64::from(CONNECT_RET.load(Ordering::Relaxed))
    }
}

#[cfg(not(all(not(test), velum_host)))]
fn raw_send(chan: t_handle, msg: *const t_chansend) -> c_int {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: msg est aligné et vivant. Le tampon qu'il désigne reste vivant le temps de l'appel.
        v_chan_send(chan, msg)
    }
    #[cfg(test)]
    {
        let _ = chan;
        if !msg.is_null() && msg.is_aligned() {
            let send = unsafe {
                // SAFETY: msg n'est pas nul et il est aligné sur t_chansend.
                msg.read()
            };
            SENT_N.fetch_add(1, Ordering::Relaxed);
            SENT_LEN.store(
                u32::try_from(send.len).unwrap_or(u32::MAX),
                Ordering::Relaxed,
            );
            SENT_VALUE.store(0, Ordering::Relaxed);
            if send.msg != 0 && send.len >= 24 {
                let mut hdr = crate::t_wmhdr::ZERO;
                unsafe {
                    // SAFETY: l'émetteur a fourni au moins send.len octets. 24 tient dans un en-tête.
                    core::ptr::copy_nonoverlapping(
                        send.msg as *const u8,
                        core::ptr::from_mut(&mut hdr).cast::<u8>(),
                        24,
                    );
                }
                SENT_MAGIC.store(hdr.magic, Ordering::Relaxed);
                SENT_TYPE.store(hdr.r#type, Ordering::Relaxed);
                SENT_SIZE.store(hdr.size, Ordering::Relaxed);
                SENT_WIN.store(hdr.window, Ordering::Relaxed);
                SENT_SEQ.store(hdr.seq, Ordering::Relaxed);
            }
            if send.msg != 0 && send.len >= 28 {
                let mut value = 0u32;
                unsafe {
                    // SAFETY: send.len couvre l'octet 24, valeur de t_wmarg.
                    core::ptr::copy_nonoverlapping(
                        (send.msg as *const u8).add(24),
                        core::ptr::from_mut(&mut value).cast::<u8>(),
                        4,
                    );
                }
                SENT_VALUE.store(value, Ordering::Relaxed);
            }
        }
        SEND_RET.load(Ordering::Relaxed)
    }
}

#[cfg(not(all(not(test), velum_host)))]
fn raw_recv(chan: t_handle, msg: *mut t_chanrecv) -> c_int {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: msg est aligné et exclusif. Ses tampons restent exclusifs le temps de l'appel.
        v_chan_recv(chan, msg)
    }
    #[cfg(test)]
    {
        let _ = (chan, msg);
        E_NOSYS
    }
}

#[cfg(not(all(not(test), velum_host)))]
fn raw_close(h: t_handle) -> c_int {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: h est un descripteur obtenu du noyau, ou zéro. v_close suit vobj.h.
        v_close(h)
    }
    #[cfg(test)]
    {
        CLOSE_N.fetch_add(1, Ordering::Relaxed);
        CLOSE_H.store(h, Ordering::Relaxed);
        0
    }
}

#[cfg(not(all(not(test), velum_host)))]
fn raw_now() -> i64 {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: v_time_mono n'a pas d'argument. Signature de vtime.h.
        v_time_mono()
    }
    #[cfg(test)]
    {
        0
    }
}

#[cfg(not(all(not(test), velum_host)))]
fn raw_map(section: t_handle, len: u64) -> i64 {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: section est un descripteur, len est la taille déjà bornée par l'appelant.
        // offset vaut 0 et prot vaut PROT_R | PROT_W, comme wmsys_map.
        v_section_map(section, 0, len, crate::PROT_R | crate::PROT_W)
    }
    #[cfg(test)]
    {
        let _ = (section, len);
        -1
    }
}

#[cfg(not(all(not(test), velum_host)))]
fn raw_vfree(va: u64, len: u64) -> c_int {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: va est une adresse rendue par v_section_map, len est alignée sur la page.
        v_vfree(va, len)
    }
    #[cfg(test)]
    {
        let _ = (va, len);
        0
    }
}

pub(crate) fn luna_apply(dark: u32, reduced: u32) {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: dark et reduced valent 0 ou 1. wm_luna_apply est fourni par rust/support/wm_luna.c.
        wm_luna_apply(dark, reduced);
    }
    #[cfg(test)]
    {
        LUNA_N.fetch_add(1, Ordering::Relaxed);
        LUNA_DARK.store(dark, Ordering::Relaxed);
        LUNA_RED.store(reduced, Ordering::Relaxed);
    }
}

fn page_align(v: u64) -> u64 {
    const PAGE: u64 = 4096;
    match v.checked_add(PAGE - 1) {
        Some(sum) => sum & !(PAGE - 1),
        None => v.wrapping_add(PAGE - 1) & !(PAGE - 1),
    }
}

/// # Safety
/// `name` est nul, ou bien une chaîne C. Le retour est celui de `v_port_connect`.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmsys_connect(name: *const c_char) -> i64 {
    raw_connect(name)
}

/// # Safety
/// `msg` est nul, ou bien il reste lisible sur `len` octets le temps de l'appel.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmsys_send(ch: t_handle, msg: *const c_void, len: u32) -> c_int {
    let send = t_chansend {
        msg: msg as usize as u64,
        len: u64::from(len),
        ..t_chansend::ZERO
    };
    raw_send(ch, &send)
}

#[cfg(not(all(not(test), velum_host)))]
fn finish(m: *mut t_wmcmsg, got: &t_chanrecv, hs: &[t_handle; IPC_HANDLES_MAX]) -> c_int {
    let mut n = got.out_nhandles;
    if n > IPC_HANDLES_MAX as u32 {
        n = IPC_HANDLES_MAX as u32;
    }
    let mut i = 1u32;
    while i < n {
        raw_close(hs[i as usize]);
        i += 1;
    }
    unsafe {
        // SAFETY: m n'est pas nul, aligné, et exclusif le temps de finish.
        (*m).handle = 0;
        if n > 0 {
            (*m).handle = hs[0];
        }
        if got.out_len > WM_MSG_MAX as u64 {
            if (*m).handle != 0 {
                raw_close((*m).handle);
            }
            (*m).handle = 0;
            return E_PROTO;
        }
        match u32::try_from(got.out_len) {
            Ok(len) => {
                (*m).len = len;
                c_int::try_from(len).unwrap_or(E_PROTO)
            }
            Err(_) => E_PROTO,
        }
    }
}

/// # Safety
/// `m` est nul, ou bien aligné et exclusif. Le tampon `m.buf` reçoit la charge utile.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmsys_recv(ch: t_handle, m: *mut t_wmcmsg, timeout_ns: u64) -> c_int {
    if m.is_null() || !m.is_aligned() {
        return E_INVAL;
    }
    let mut hs = [0u32; IPC_HANDLES_MAX];
    let mut got = t_chanrecv::ZERO;
    unsafe {
        // SAFETY: m n'est pas nul et il est aligné. buf a WM_MSG_MAX octets.
        got.buf = (*m).buf.as_mut_ptr() as usize as u64;
        (*m).handle = 0;
        (*m).len = 0;
    }
    got.buf_len = WM_MSG_MAX as u64;
    got.handles = hs.as_mut_ptr() as usize as u64;
    got.handles_max = IPC_HANDLES_MAX as u32;
    got.timeout_ns = timeout_ns;
    let ret = raw_recv(ch, &mut got);
    if ret < 0 {
        return ret;
    }
    finish(m, &got, &hs)
}

/// # Safety
/// `section` est un descripteur de section. `len` est la taille à mapper.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmsys_map(section: t_handle, len: u64) -> *mut c_void {
    let va = raw_map(section, len);
    if va <= 0 {
        return core::ptr::null_mut();
    }
    va as u64 as *mut c_void
}

/// # Safety
/// `va` est nul, ou bien une adresse rendue par `wmsys_map`. `len` est la taille demandée.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmsys_unmap(va: *mut c_void, len: u64) {
    if va.is_null() {
        return;
    }
    raw_vfree(va as usize as u64, page_align(len));
}

/// # Safety
/// `h` est un descripteur, ou zéro. Zéro n'est pas fermé.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmsys_close(h: t_handle) {
    if h != 0 {
        raw_close(h);
    }
}

/// # Safety
/// Aucun pointeur. Une horloge négative donne 0.
#[cfg(not(all(not(test), velum_host)))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn wmsys_now() -> u64 {
    let t = raw_now();
    if t < 0 {
        return 0;
    }
    t as u64
}

pub(crate) fn surface_pages(stride: u32, height: u32) -> Option<u64> {
    let bytes = u64::from(stride)
        .checked_mul(u64::from(height))?
        .checked_mul(4)?;
    bytes.checked_add(WMC_PAGE - 1).map(|v| v & !(WMC_PAGE - 1))
}

pub(crate) fn c_len(stride: i32, h: i32) -> u64 {
    let s = stride as u64;
    let hh = h as u64;
    let prod = match s.checked_mul(hh) {
        Some(v) => match v.checked_mul(4) {
            Some(n) => n,
            None => v.wrapping_mul(4),
        },
        None => s.wrapping_mul(hh).wrapping_mul(4),
    };
    page_align(prod)
}

const _: () = assert!(WM_BUF_WORDS == 40);
const _: () = assert!(pod_size::<crate::t_wmhdr>() == 24);
