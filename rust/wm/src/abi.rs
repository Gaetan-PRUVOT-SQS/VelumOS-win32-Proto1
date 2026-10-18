#![allow(non_camel_case_types)]

use core::cell::UnsafeCell;
use core::ffi::c_int;

pub const E_OK: c_int = 0;
pub const E_IO: c_int = -5;
pub const E_BADF: c_int = -9;
pub const E_NOMEM: c_int = -12;
pub const E_BUSY: c_int = -16;
pub const E_INVAL: c_int = -22;
pub const E_NOSPC: c_int = -28;
pub const E_PIPE: c_int = -32;
pub const E_RANGE: c_int = -34;
pub const E_PROTO: c_int = -71;
pub const E_TIMEOUT: c_int = -110;

pub const WM_PORT_NAME: &[u8] = b"wm\0";
pub const WM_MAGIC: u32 = 0x314d_5756;
pub const WM_VERSION: u32 = 1;
pub const WM_TITLE_MAX: usize = 64;
pub const WM_RECTS_MAX: u32 = 16;
pub const WM_MSG_MAX: usize = 320;
pub const WM_BUF_WORDS: usize = WM_MSG_MAX / 8;
pub const WM_WIN_PER_CLIENT: usize = 64;
pub const WM_REPLY_NS: u64 = 2_000_000_000;
pub const WMC_STASH: u32 = 32;
pub const WMC_PAGE: u64 = 4096;
pub const WMC_DIM_MAX: u32 = 8192;

pub const WMC_HELLO: u32 = 0x0001;
pub const WMC_CREATE: u32 = 0x0002;
pub const WMC_DESTROY: u32 = 0x0003;
pub const WMC_SET_TITLE: u32 = 0x0004;
pub const WMC_SET_RECT: u32 = 0x0005;
pub const WMC_SET_STATE: u32 = 0x0006;
pub const WMC_PRESENT: u32 = 0x0007;
pub const WMC_SET_CURSOR: u32 = 0x0008;
pub const WMC_ACTIVATE: u32 = 0x0009;
pub const WMC_SET_WORKAREA: u32 = 0x000a;
pub const WMC_SUBSCRIBE: u32 = 0x000b;
pub const WMC_SET_ICON: u32 = 0x000c;
pub const WMC_CAPTURE: u32 = 0x000d;
pub const WMC_SET_THEME: u32 = 0x000e;

pub const WMS_HELLO_OK: u32 = 0x1001;
pub const WMS_CREATED: u32 = 0x1002;
pub const WMS_KEY: u32 = 0x1003;
pub const WMS_MOUSE: u32 = 0x1004;
pub const WMS_CLOSE_REQ: u32 = 0x1005;
pub const WMS_ACTIVATE: u32 = 0x1006;
pub const WMS_RESIZED: u32 = 0x1007;
pub const WMS_PAINT: u32 = 0x1008;
pub const WMS_STATE: u32 = 0x1009;
pub const WMS_WIN_ADD: u32 = 0x100a;
pub const WMS_WIN_DEL: u32 = 0x100b;
pub const WMS_WIN_UPD: u32 = 0x100c;
pub const WMS_THEME: u32 = 0x100d;
pub const WMS_ERROR: u32 = 0x10ff;

pub const WSTATE_NORMAL: u32 = 0;
pub const WSTATE_MIN: u32 = 1;
pub const WSTATE_MAX: u32 = 2;
pub const WSTATE_HIDDEN: u32 = 3;
pub const CUR_ARROW: u32 = 0;
pub const CUR_HAND: u32 = 7;
pub const ICON_IDS: u32 = 14;

pub const PROT_R: u32 = 0x01;
pub const PROT_W: u32 = 0x02;
pub const IPC_HANDLES_MAX: usize = 8;

pub type t_handle = u32;

pub(crate) const fn pod_size<T>() -> u32 {
    let n = core::mem::size_of::<T>();
    assert!(n <= u32::MAX as usize);
    n as u32
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_rect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
}

impl t_rect {
    pub const ZERO: Self = Self {
        x: 0,
        y: 0,
        w: 0,
        h: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_inpevent {
    pub r#type: u32,
    pub code: u32,
    pub x: i32,
    pub y: i32,
    pub mods: u32,
    pub scancode: u32,
    pub time_ns: u64,
}

impl t_inpevent {
    pub const ZERO: Self = Self {
        r#type: 0,
        code: 0,
        x: 0,
        y: 0,
        mods: 0,
        scancode: 0,
        time_ns: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_surface {
    pub px: *mut u32,
    pub w: i32,
    pub h: i32,
    pub stride: i32,
    pub clip: t_rect,
}

impl t_surface {
    pub const ZERO: Self = Self {
        px: core::ptr::null_mut(),
        w: 0,
        h: 0,
        stride: 0,
        clip: t_rect::ZERO,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmhdr {
    pub magic: u32,
    pub r#type: u32,
    pub size: u32,
    pub window: u32,
    pub seq: u32,
    pub status: i32,
}

impl t_wmhdr {
    pub const ZERO: Self = Self {
        magic: 0,
        r#type: 0,
        size: 0,
        window: 0,
        seq: 0,
        status: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmhello {
    pub h: t_wmhdr,
    pub version: u32,
    pub screen_w: u32,
    pub screen_h: u32,
    pub workarea: t_rect,
}

impl t_wmhello {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        version: 0,
        screen_w: 0,
        screen_h: 0,
        workarea: t_rect::ZERO,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmcreate {
    pub h: t_wmhdr,
    pub rect: t_rect,
    pub style: u32,
    pub state: u32,
    pub title: [u8; WM_TITLE_MAX],
}

impl t_wmcreate {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        rect: t_rect::ZERO,
        style: 0,
        state: 0,
        title: [0; WM_TITLE_MAX],
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmcreated {
    pub h: t_wmhdr,
    pub stride: u32,
    pub width: u32,
    pub height: u32,
    pub reserved: u32,
}

impl t_wmcreated {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        stride: 0,
        width: 0,
        height: 0,
        reserved: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmpresent {
    pub h: t_wmhdr,
    pub nrects: u32,
    pub rects: [t_rect; WM_RECTS_MAX as usize],
}

impl t_wmpresent {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        nrects: 0,
        rects: [t_rect::ZERO; WM_RECTS_MAX as usize],
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmkey {
    pub h: t_wmhdr,
    pub ev: t_inpevent,
}

impl t_wmkey {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        ev: t_inpevent::ZERO,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmmouse {
    pub h: t_wmhdr,
    pub r#type: u32,
    pub x: i32,
    pub y: i32,
    pub buttons: u32,
    pub wheel: i32,
    pub hit: u32,
}

impl t_wmmouse {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        r#type: 0,
        x: 0,
        y: 0,
        buttons: 0,
        wheel: 0,
        hit: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wminfo {
    pub h: t_wmhdr,
    pub pid: u32,
    pub style: u32,
    pub state: u32,
    pub icon: u32,
    pub active: u32,
    pub title: [u8; WM_TITLE_MAX],
}

impl t_wminfo {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        pid: 0,
        style: 0,
        state: 0,
        icon: 0,
        active: 0,
        title: [0; WM_TITLE_MAX],
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmtheme {
    pub h: t_wmhdr,
    pub dark: u32,
    pub reduced: u32,
    pub reserved: u32,
}

impl t_wmtheme {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        dark: 0,
        reduced: 0,
        reserved: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmwin {
    pub id: u32,
    pub chan: t_handle,
    pub section: t_handle,
    pub surface: t_surface,
    pub style: u32,
}

impl t_wmwin {
    pub const ZERO: Self = Self {
        id: 0,
        chan: 0,
        section: 0,
        surface: t_surface::ZERO,
        style: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmarg {
    pub h: t_wmhdr,
    pub value: u32,
    pub reserved: u32,
}

impl t_wmarg {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        value: 0,
        reserved: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmrect {
    pub h: t_wmhdr,
    pub rect: t_rect,
}

impl t_wmrect {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        rect: t_rect::ZERO,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmtitle {
    pub h: t_wmhdr,
    pub title: [u8; WM_TITLE_MAX],
}

impl t_wmtitle {
    pub const ZERO: Self = Self {
        h: t_wmhdr::ZERO,
        title: [0; WM_TITLE_MAX],
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmcmsg {
    pub buf: [u64; WM_BUF_WORDS],
    pub len: u32,
    pub handle: t_handle,
}

impl t_wmcmsg {
    pub const ZERO: Self = Self {
        buf: [0; WM_BUF_WORDS],
        len: 0,
        handle: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub struct t_wmcstate {
    pub chan: t_handle,
    pub seq: u32,
    pub lost: bool,
    pub info: t_wmhello,
    pub head: u32,
    pub n: u32,
    pub stash: [t_wmcmsg; WMC_STASH as usize],
    pub rx: t_wmcmsg,
    pub wins: [*mut t_wmwin; WM_WIN_PER_CLIENT],
}

impl t_wmcstate {
    pub const ZERO: Self = Self {
        chan: 0,
        seq: 0,
        lost: false,
        info: t_wmhello::ZERO,
        head: 0,
        n: 0,
        stash: [t_wmcmsg::ZERO; WMC_STASH as usize],
        rx: t_wmcmsg::ZERO,
        wins: [core::ptr::null_mut(); WM_WIN_PER_CLIENT],
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub(crate) struct t_chansend {
    pub msg: u64,
    pub len: u64,
    pub handles: u64,
    pub nhandles: u32,
    pub reserved: u32,
}

#[cfg(not(all(not(test), velum_host)))]
impl t_chansend {
    pub const ZERO: Self = Self {
        msg: 0,
        len: 0,
        handles: 0,
        nhandles: 0,
        reserved: 0,
    };
}

#[repr(C)]
#[derive(Clone, Copy, Debug)]
#[allow(non_camel_case_types)]
pub(crate) struct t_chanrecv {
    pub buf: u64,
    pub buf_len: u64,
    pub handles: u64,
    pub handles_max: u32,
    pub reserved: u32,
    pub timeout_ns: u64,
    pub out_len: u64,
    pub out_nhandles: u32,
    pub reserved2: u32,
}

#[cfg(not(all(not(test), velum_host)))]
impl t_chanrecv {
    pub const ZERO: Self = Self {
        buf: 0,
        buf_len: 0,
        handles: 0,
        handles_max: 0,
        reserved: 0,
        timeout_ns: 0,
        out_len: 0,
        out_nhandles: 0,
        reserved2: 0,
    };
}

#[repr(transparent)]
pub struct WmCell<T> {
    inner: UnsafeCell<T>,
}

impl<T> WmCell<T> {
    pub const fn new(value: T) -> Self {
        Self {
            inner: UnsafeCell::new(value),
        }
    }

    pub const fn get(&self) -> *mut T {
        self.inner.get()
    }
}

// SAFETY: même rôle que le static C g_wmc. Les pointeurs de fenêtre
// ne sont pas Sync. Le client ne partage pas cet état entre fils.
unsafe impl Sync for WmCell<t_wmcstate> {}

#[unsafe(no_mangle)]
pub static g_wmc: WmCell<t_wmcstate> = WmCell::new(t_wmcstate::ZERO);

pub(crate) fn state() -> *mut t_wmcstate {
    g_wmc.get()
}
