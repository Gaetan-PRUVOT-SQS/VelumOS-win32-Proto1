use core::ffi::c_int;

pub const E_OK: c_int = 0;
pub const E_IO: c_int = -5;
pub const E_NOENT: c_int = -2;
pub const E_INVAL: c_int = -22;
pub const E_OVERFLOW: c_int = -75;
pub const E_TIMEOUT: c_int = -110;

pub const NOTIF_PORT_NAME: &[u8] = b"notif\0";
pub const NOTIF_MAGIC: u32 = 0x314e_5456;
pub const NOTIF_MAX: usize = 16;
pub const NOTIF_APP_MAX: usize = 24;
pub const NOTIF_TITLE_MAX: usize = 48;
pub const NOTIF_TEXT_MAX: usize = 120;
pub const NOTIF_BADGE_MAX: usize = 4;
pub const NOTIF_AGE_MAX: usize = 24;
pub const NOTIF_CAP_BADGE: u32 = 99;

pub const NOTIFC_POST: u32 = 1;
pub const NOTIFC_COUNT: u32 = 2;
pub const NOTIFC_TOGGLE: u32 = 3;
pub const NOTIFC_DISMISS: u32 = 4;
pub const NOTIFC_CLEAR: u32 = 5;

pub const NOTIFS_OK: u32 = 0x101;
pub const NOTIFS_COUNT: u32 = 0x102;
pub const NOTIFS_ERROR: u32 = 0x1ff;

pub(crate) const MSG_BYTES: u32 = {
    let n = core::mem::size_of::<t_notifmsg>();
    assert!(n <= u32::MAX as usize);
    n as u32
};

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_notif {
    pub id: u32,
    pub icon: u32,
    pub stamp_ns: u64,
    pub app: [u8; NOTIF_APP_MAX],
    pub title: [u8; NOTIF_TITLE_MAX],
    pub text: [u8; NOTIF_TEXT_MAX],
}

impl t_notif {
    pub const fn zero() -> Self {
        Self {
            id: 0,
            icon: 0,
            stamp_ns: 0,
            app: [0; NOTIF_APP_MAX],
            title: [0; NOTIF_TITLE_MAX],
            text: [0; NOTIF_TEXT_MAX],
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_notifstore {
    pub items: [t_notif; NOTIF_MAX],
    pub count: u32,
    pub next_id: u32,
    pub dropped: u32,
}

impl t_notifstore {
    pub const fn zero() -> Self {
        Self {
            items: [t_notif::zero(); NOTIF_MAX],
            count: 0,
            next_id: 0,
            dropped: 0,
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_notifmsg {
    pub magic: u32,
    pub r#type: u32,
    pub size: u32,
    pub seq: u32,
    pub status: i32,
    pub id: u32,
    pub count: u32,
    pub icon: u32,
    pub stamp_ns: u64,
    pub app: [u8; NOTIF_APP_MAX],
    pub title: [u8; NOTIF_TITLE_MAX],
    pub text: [u8; NOTIF_TEXT_MAX],
}

impl t_notifmsg {
    pub const fn zero() -> Self {
        Self {
            magic: 0,
            r#type: 0,
            size: 0,
            seq: 0,
            status: 0,
            id: 0,
            count: 0,
            icon: 0,
            stamp_ns: 0,
            app: [0; NOTIF_APP_MAX],
            title: [0; NOTIF_TITLE_MAX],
            text: [0; NOTIF_TEXT_MAX],
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_chansend {
    pub msg: u64,
    pub len: u64,
    pub handles: u64,
    pub nhandles: u32,
    pub reserved: u32,
}

impl t_chansend {
    pub const fn zero() -> Self {
        Self {
            msg: 0,
            len: 0,
            handles: 0,
            nhandles: 0,
            reserved: 0,
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_chanrecv {
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

impl t_chanrecv {
    pub const fn zero() -> Self {
        Self {
            buf: 0,
            buf_len: 0,
            handles: 0,
            handles_max: 0,
            reserved: 0,
            timeout_ns: 0,
            out_len: 0,
            out_nhandles: 0,
            reserved2: 0,
        }
    }
}
