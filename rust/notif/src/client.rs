use core::ffi::{c_char, c_int};
use core::sync::atomic::{AtomicI64, AtomicU32, Ordering};

use crate::abi::{
    E_INVAL, E_OVERFLOW, NOTIF_MAGIC, NOTIFC_CLEAR, NOTIFC_COUNT, NOTIFC_DISMISS, NOTIFC_TOGGLE,
    t_chanrecv, t_chansend, t_notifmsg,
};
use crate::wire::{notif_msg_init, notif_msg_post};

const NOTIF_REPLY_NS: u64 = 30_000_000;
const NOTIF_CACHE_NS: i64 = 250_000_000;

static CHAN: AtomicU32 = AtomicU32::new(0);
static SEQ: AtomicU32 = AtomicU32::new(0);
static CACHED: AtomicU32 = AtomicU32::new(0);
static CACHED_AT: AtomicI64 = AtomicI64::new(0);

mod raw {
    #[cfg(not(test))]
    use super::{t_chanrecv, t_chansend};
    #[cfg(not(test))]
    use core::ffi::{c_char, c_int};

    #[cfg(not(test))]
    unsafe extern "C" {
        pub fn v_close(h: u32) -> c_int;
        pub fn v_port_connect(name: *const c_char, flags: u32) -> i64;
        pub fn v_chan_send(chan: u32, msg: *const t_chansend) -> c_int;
        pub fn v_chan_recv(chan: u32, msg: *mut t_chanrecv) -> c_int;
        pub fn v_time_mono() -> i64;
    }

    #[cfg(test)]
    pub use super::harness::{v_chan_recv, v_chan_send, v_close, v_port_connect, v_time_mono};
}

fn sys_close(h: u32) -> c_int {
    unsafe {
        // SAFETY: h est un t_handle. Signature de v_close dans vobj.h.
        raw::v_close(h)
    }
}

fn sys_connect(name: *const c_char, flags: u32) -> i64 {
    unsafe {
        // SAFETY: name est le CStr statique « notif », flags est un u32. Signature de v_port_connect.
        raw::v_port_connect(name, flags)
    }
}

fn sys_send(chan: u32, msg: &t_chansend) -> c_int {
    unsafe {
        // SAFETY: msg est aligné et vivant le temps de l'appel. Signature de v_chan_send.
        raw::v_chan_send(chan, msg)
    }
}

fn sys_recv(chan: u32, msg: &mut t_chanrecv) -> c_int {
    unsafe {
        // SAFETY: msg est aligné et exclusif le temps de l'appel. Signature de v_chan_recv.
        raw::v_chan_recv(chan, msg)
    }
}

fn sys_time() -> i64 {
    unsafe {
        // SAFETY: v_time_mono n'a pas d'argument. Signature de vtime.h.
        raw::v_time_mono()
    }
}

fn as_addr<T>(p: *const T) -> Result<u64, c_int> {
    u64::try_from(p as usize).map_err(|_| E_OVERFLOW)
}

fn msg_len() -> Result<u64, c_int> {
    u64::try_from(core::mem::size_of::<t_notifmsg>()).map_err(|_| E_OVERFLOW)
}

/// # Safety
/// Aucun pointeur. Ferme le canal s'il est ouvert. `v_close` suit vobj.h.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_client_close() {
    let h = CHAN.load(Ordering::Relaxed);
    if h != 0 {
        sys_close(h);
    }
    CHAN.store(0, Ordering::Relaxed);
    CACHED_AT.store(0, Ordering::Relaxed);
}

fn ensure_channel() -> Result<u32, c_int> {
    let cur = CHAN.load(Ordering::Relaxed);
    if cur != 0 {
        return Ok(cur);
    }
    let h = sys_connect(c"notif".as_ptr(), 0);
    if h < 0 {
        return Err(h as c_int);
    }
    let handle = h as u32;
    CHAN.store(handle, Ordering::Relaxed);
    Ok(handle)
}

fn send_msg(m: &mut t_notifmsg) -> c_int {
    let len = match msg_len() {
        Ok(n) => n,
        Err(e) => return e,
    };
    let addr = match as_addr(m as *const t_notifmsg) {
        Ok(a) => a,
        Err(e) => return e,
    };
    let chan = match ensure_channel() {
        Ok(h) => h,
        Err(e) => return e,
    };
    let seq = SEQ.fetch_add(1, Ordering::Relaxed).wrapping_add(1);
    m.seq = seq;
    let mut s = t_chansend::zero();
    s.msg = addr;
    s.len = len;
    let r = sys_send(chan, &s);
    if r < 0 {
        unsafe {
            // SAFETY: notif_client_close ne prend pas de pointeur. Même ordre que le C.
            notif_client_close();
        }
    }
    r
}

fn recv_msg(m: &mut t_notifmsg) -> c_int {
    let len = match msg_len() {
        Ok(n) => n,
        Err(e) => return e,
    };
    let addr = match as_addr(m as *const t_notifmsg) {
        Ok(a) => a,
        Err(e) => return e,
    };
    let mut slot = t_chanrecv::zero();
    slot.buf = addr;
    slot.buf_len = len;
    slot.timeout_ns = NOTIF_REPLY_NS;
    let ret = sys_recv(CHAN.load(Ordering::Relaxed), &mut slot);
    if ret < 0 {
        return ret;
    }
    if slot.out_len != len || m.magic != NOTIF_MAGIC {
        return E_INVAL;
    }
    0
}

fn call(m: &mut t_notifmsg) -> c_int {
    let r = send_msg(m);
    if r < 0 {
        return r;
    }
    let r = recv_msg(m);
    if r < 0 {
        return r;
    }
    m.status
}

fn simple(ty: u32, id: u32) -> c_int {
    let mut m = t_notifmsg::zero();
    unsafe {
        // SAFETY: m est aligné et exclusif.
        notif_msg_init(&mut m, ty);
    }
    m.id = id;
    CACHED_AT.store(0, Ordering::Relaxed);
    call(&mut m)
}

/// # Safety
/// `app`, `title` et `text` sont nuls, ou bien lisibles comme pour `notif_msg_post`.
/// Un champ invalide donne `E_INVAL` sans appel IPC. Le canal vient de `v_port_connect`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_post(
    app: *const c_char,
    title: *const c_char,
    text: *const c_char,
    icon: u32,
) -> c_int {
    let mut m = t_notifmsg::zero();
    let r = unsafe {
        // SAFETY: m est exclusif. Les chaînes sont celles de l'appelant.
        notif_msg_post(&mut m, app, title, text, icon)
    };
    if r < 0 {
        return r;
    }
    let r = call(&mut m);
    if r < 0 {
        return r;
    }
    m.id as c_int
}

/// # Safety
/// `out` est nul, ou bien il est aligné pour `u32` et exclusif. Nul : `E_INVAL`.
/// `v_time_mono` suit vtime.h. Le cache suit le C : fenêtre 250 ms, horloge signée.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_client_count(out: *mut u32) -> c_int {
    if out.is_null() {
        return E_INVAL;
    }
    let now = sys_time();
    let at = CACHED_AT.load(Ordering::Relaxed);
    if at != 0
        && let Some(delta) = now.checked_sub(at)
        && delta < NOTIF_CACHE_NS
    {
        let n = CACHED.load(Ordering::Relaxed);
        unsafe {
            // SAFETY: out n'est pas nul, il est aligné et exclusif.
            out.write(n);
        }
        return 0;
    }
    let mut m = t_notifmsg::zero();
    unsafe {
        // SAFETY: m est aligné et exclusif.
        notif_msg_init(&mut m, NOTIFC_COUNT);
    }
    let r = call(&mut m);
    if r < 0 {
        return r;
    }
    CACHED.store(m.count, Ordering::Relaxed);
    CACHED_AT.store(now, Ordering::Relaxed);
    unsafe {
        // SAFETY: out n'est pas nul, il est aligné et exclusif.
        out.write(m.count);
    }
    0
}

/// # Safety
/// Aucun pointeur. Invalide le cache puis envoie `NOTIFC_TOGGLE`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_client_toggle() -> c_int {
    simple(NOTIFC_TOGGLE, 0)
}

/// # Safety
/// Aucun pointeur appelant. `id` est recopié dans le message. Le cache est invalidé avant l'envoi.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_client_dismiss(id: u32) -> c_int {
    simple(NOTIFC_DISMISS, id)
}

/// # Safety
/// Aucun pointeur. Invalide le cache puis envoie `NOTIFC_CLEAR`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn notif_client_clear() -> c_int {
    simple(NOTIFC_CLEAR, 0)
}

#[cfg(test)]
pub(crate) fn reset_internals() {
    CHAN.store(0, Ordering::Relaxed);
    SEQ.store(0, Ordering::Relaxed);
    CACHED.store(0, Ordering::Relaxed);
    CACHED_AT.store(0, Ordering::Relaxed);
}

#[cfg(test)]
pub(crate) mod harness {
    use super::{t_chanrecv, t_chansend, t_notifmsg};
    use crate::abi::{E_INVAL, NOTIF_MAGIC};
    use core::ffi::{c_char, c_int};
    use std::sync::Mutex;

    #[derive(Clone)]
    pub struct Script {
        pub mono: i64,
        pub connect: i64,
        pub send: c_int,
        pub recv: c_int,
        pub out_len: u64,
        pub reply: t_notifmsg,
    }

    impl Default for Script {
        fn default() -> Self {
            let mut reply = t_notifmsg::zero();
            reply.magic = NOTIF_MAGIC;
            reply.id = 1;
            Self {
                mono: 0,
                connect: 4,
                send: 0,
                recv: 0,
                out_len: core::mem::size_of::<t_notifmsg>() as u64,
                reply,
            }
        }
    }

    #[derive(Default)]
    pub struct Trace {
        pub names: Vec<Vec<u8>>,
        pub flags: Vec<u32>,
        pub send_chan: Vec<u32>,
        pub sends: Vec<t_chansend>,
        pub msgs: Vec<t_notifmsg>,
        pub recv_chan: Vec<u32>,
        pub recvs: Vec<t_chanrecv>,
        pub closed: Vec<u32>,
    }

    #[derive(Default)]
    pub struct State {
        pub script: Script,
        pub trace: Trace,
    }

    static STATE: Mutex<State> = Mutex::new(State {
        script: Script {
            mono: 0,
            connect: 4,
            send: 0,
            recv: 0,
            out_len: 0,
            reply: t_notifmsg {
                magic: 0,
                r#type: 0,
                size: 0,
                seq: 0,
                status: 0,
                id: 0,
                count: 0,
                icon: 0,
                stamp_ns: 0,
                app: [0; crate::abi::NOTIF_APP_MAX],
                title: [0; crate::abi::NOTIF_TITLE_MAX],
                text: [0; crate::abi::NOTIF_TEXT_MAX],
            },
        },
        trace: Trace {
            names: Vec::new(),
            flags: Vec::new(),
            send_chan: Vec::new(),
            sends: Vec::new(),
            msgs: Vec::new(),
            recv_chan: Vec::new(),
            recvs: Vec::new(),
            closed: Vec::new(),
        },
    });

    fn with<R>(f: impl FnOnce(&mut State) -> R) -> R {
        let mut guard = STATE.lock().unwrap_or_else(|e| e.into_inner());
        f(&mut guard)
    }

    pub(crate) fn reset() {
        with(|st| *st = State::default());
        super::reset_internals();
    }

    pub(crate) fn update(f: impl FnOnce(&mut State)) {
        with(f);
    }

    pub(crate) fn read<R>(f: impl FnOnce(&State) -> R) -> R {
        with(|st| f(st))
    }

    /// # Safety
    /// Aucun pointeur. Enregistre le descripteur.
    #[unsafe(no_mangle)]
    pub unsafe extern "C" fn v_close(h: u32) -> c_int {
        with(|st| st.trace.closed.push(h));
        0
    }

    /// # Safety
    /// `name` est nul, ou bien lisible jusqu'au NUL ou sur 64 octets.
    #[unsafe(no_mangle)]
    pub unsafe extern "C" fn v_port_connect(name: *const c_char, flags: u32) -> i64 {
        let mut bytes = Vec::new();
        if !name.is_null() {
            let mut i = 0usize;
            while i < 64 {
                let b = unsafe {
                    // SAFETY: name n'est pas nul et i < 64. Arrêt au NUL.
                    name.add(i).read() as u8
                };
                bytes.push(b);
                i += 1;
                if b == 0 {
                    break;
                }
            }
        }
        with(|st| {
            st.trace.names.push(bytes);
            st.trace.flags.push(flags);
            st.script.connect
        })
    }

    /// # Safety
    /// `msg` est nul, ou bien il est aligné et lisible. Le message pointé, s'il y en a un,
    /// reste vivant le temps de l'appel.
    #[unsafe(no_mangle)]
    pub unsafe extern "C" fn v_chan_send(chan: u32, msg: *const t_chansend) -> c_int {
        if msg.is_null() {
            return E_INVAL;
        }
        let send = unsafe {
            // SAFETY: msg n'est pas nul et il est aligné.
            *msg
        };
        let size = core::mem::size_of::<t_notifmsg>() as u64;
        let copied = if send.msg != 0 && send.len == size {
            let p = send.msg as *const t_notifmsg;
            if p.is_aligned() {
                Some(unsafe {
                    // SAFETY: le client a passé l'adresse d'une t_notifmsg vivante.
                    p.read()
                })
            } else {
                None
            }
        } else {
            None
        };
        with(|st| {
            st.trace.send_chan.push(chan);
            st.trace.sends.push(send);
            if let Some(m) = copied {
                st.trace.msgs.push(m);
            }
            st.script.send
        })
    }

    /// # Safety
    /// `msg` est nul, ou bien il est aligné et exclusif. Le tampon `buf`, s'il est assez grand,
    /// est celui du client et reste exclusif le temps de l'appel.
    #[unsafe(no_mangle)]
    pub unsafe extern "C" fn v_chan_recv(chan: u32, msg: *mut t_chanrecv) -> c_int {
        if msg.is_null() {
            return E_INVAL;
        }
        let incoming = unsafe {
            // SAFETY: msg n'est pas nul et il est aligné.
            *msg
        };
        let (ret, reply, out_len) = with(|st| {
            st.trace.recv_chan.push(chan);
            st.trace.recvs.push(incoming);
            (st.script.recv, st.script.reply, st.script.out_len)
        });
        if ret < 0 {
            return ret;
        }
        let slot = unsafe {
            // SAFETY: msg n'est pas nul, aligné et exclusif.
            &mut *msg
        };
        slot.out_len = out_len;
        let size = core::mem::size_of::<t_notifmsg>() as u64;
        if out_len == size && slot.buf != 0 {
            let p = slot.buf as *mut t_notifmsg;
            if p.is_aligned() {
                unsafe {
                    // SAFETY: buf est l'adresse d'une t_notifmsg exclusive du client.
                    p.write(reply);
                }
            }
        }
        ret
    }

    /// # Safety
    /// Aucun pointeur.
    #[unsafe(no_mangle)]
    pub unsafe extern "C" fn v_time_mono() -> i64 {
        with(|st| st.script.mono)
    }
}
