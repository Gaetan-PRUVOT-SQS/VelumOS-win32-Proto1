#![allow(non_camel_case_types)]

use core::ffi::{c_char, c_int, c_void};

pub const E_OK: c_int = 0;
pub const E_PERM: c_int = -1;
pub const E_NOENT: c_int = -2;
pub const E_IO: c_int = -5;
pub const E_BADF: c_int = -9;
pub const E_CHILD: c_int = -10;
pub const E_AGAIN: c_int = -11;
pub const E_NOMEM: c_int = -12;
pub const E_ACCES: c_int = -13;
pub const E_FAULT: c_int = -14;
pub const E_BUSY: c_int = -16;
pub const E_EXIST: c_int = -17;
pub const E_NODEV: c_int = -19;
pub const E_NOTDIR: c_int = -20;
pub const E_ISDIR: c_int = -21;
pub const E_INVAL: c_int = -22;
pub const E_NFILE: c_int = -23;
pub const E_MFILE: c_int = -24;
pub const E_NOSPC: c_int = -28;
pub const E_PIPE: c_int = -32;
pub const E_RANGE: c_int = -34;
pub const E_DEADLK: c_int = -35;
pub const E_NOSYS: c_int = -38;
pub const E_NOTEMPTY: c_int = -39;
pub const E_PROTO: c_int = -71;
pub const E_OVERFLOW: c_int = -75;
pub const E_NOTSUP: c_int = -95;
pub const E_TIMEOUT: c_int = -110;
pub const E_CANCELED: c_int = -125;

pub const CTL_TEXT_MAX: usize = 256;
pub const CTL_VISIBLE: u32 = 0x01;
pub const CTL_ENABLED: u32 = 0x02;
pub const CTL_FOCUSABLE: u32 = 0x04;
pub const CTL_CHECKED: u32 = 0x08;
pub const CTL_DEFAULT: u32 = 0x10;
pub const CTL_MULTILINE: u32 = 0x20;
pub const CTL_PASSWORD: u32 = 0x40;
pub const CTL_NOBACK: u32 = 0x80;
pub const CTL_ALIGN_CENTER: u32 = 0x100;
pub const CTL_ALIGN_RIGHT: u32 = 0x200;
pub const CTL_ELLIPSIS: u32 = 0x400;
pub const CTL_BOLD: u32 = 0x800;
pub const CTL_ID_OK: u32 = 1;
pub const CTL_ID_CANCEL: u32 = 2;
pub const CN_CLICKED: u32 = 1;
pub const CN_CHANGED: u32 = 2;
pub const CN_SELECT: u32 = 3;
pub const CN_ENTER: u32 = 4;
pub const CN_DBLCLK: u32 = 5;
pub const CN_SCROLL: u32 = 6;
pub const CN_CLOSED: u32 = 7;
pub const CTL_MI_DISABLED: u32 = 0x01;
pub const CTL_MI_CHECKED: u32 = 0x02;
pub const CTL_MI_SEPARATOR: u32 = 0x04;
pub const CTL_MENU_ITEMS_MAX: u32 = 64;
pub const CTL_MENU_LEVELS_MAX: u32 = 4;
pub const CTL_LIST_ITEMS_MAX: u32 = 65536;
pub const CTL_DEPTH_MAX: u32 = 32;
pub const CTL_DBLCLK_NS: i64 = 500_000_000;
pub const CTL_FLAGS_ALL: u32 = 0xfff;
pub const CTL_FLAGS_VISUAL: u32 = 0xffb;
pub const CTL_COORD_MAX: i32 = 1_048_576;
pub const CTL_BOX: i32 = 13;
pub const CTL_RADIO_BOX: i32 = 14;
pub const CTL_GAP: i32 = 4;
pub const CTL_PAD: i32 = 3;
pub const CTL_ROW_PAD: i32 = 2;
pub const CTL_FRAME: i32 = 2;
pub const CTL_RING_W: i32 = 2;
pub const CTL_EDIT_INSET: i32 = 3;
pub const CTL_WHEEL_ROWS: i32 = 3;
pub const CTL_DBLCLK_DIST: i32 = 4;
pub const CTL_ARROW: i32 = 4;
pub const CTL_THUMB_MIN: i32 = 8;
pub const CTL_SCROLL_MAX: i32 = 1_073_741_824;
pub const CTL_MENU_BORDER: i32 = 2;
pub const CTL_MENU_GUTTER: i32 = 22;
pub const CTL_MENU_ARROW: i32 = 16;
pub const CTL_MENU_PAD: i32 = 4;
pub const CTL_ST_HOT: u32 = 0x01;
pub const CTL_ST_PRESSED: u32 = 0x02;
pub const CTL_ST_CLOSED: u32 = 0x04;
pub const CTL_MEV_NONE: u32 = 0;
pub const CTL_MEV_MOVE: u32 = 1;
pub const CTL_MEV_DOWN: u32 = 2;
pub const CTL_MEV_UP: u32 = 3;
pub const CTL_MEV_WHEEL: u32 = 4;
pub const CTL_SBZ_NONE: u32 = 0;
pub const CTL_SBZ_DEC: u32 = 1;
pub const CTL_SBZ_INC: u32 = 2;
pub const CTL_SBZ_PAGE_DEC: u32 = 3;
pub const CTL_SBZ_PAGE_INC: u32 = 4;
pub const CTL_SBZ_THUMB: u32 = 5;
pub const CTL_SBR_NONE: c_int = 0;
pub const CTL_SBR_USED: c_int = 1;
pub const CTL_SBR_MOVED: c_int = 2;
pub const CTL_EDR_USED: u32 = 1;
pub const CTL_EDR_TEXT: u32 = 2;
pub const CTL_EDR_VIEW: u32 = 4;
pub const CTL_EDR_ENTER: u32 = 8;
pub const CTL_VK_A: u32 = 0x41;
pub const CTL_VK_C: u32 = 0x43;
pub const CTL_VK_V: u32 = 0x56;
pub const CTL_VK_X: u32 = 0x58;

pub const CT_PANEL: u32 = 0;
pub const CT_LABEL: u32 = 1;
pub const CT_BUTTON: u32 = 2;
pub const CT_CHECK: u32 = 3;
pub const CT_RADIO: u32 = 4;
pub const CT_GROUP: u32 = 5;
pub const CT_EDIT: u32 = 6;
pub const CT_LIST: u32 = 7;
pub const CT_SCROLL: u32 = 8;
pub const CT_PROGRESS: u32 = 9;
pub const CT_MENU: u32 = 10;
pub const CT_IMAGE: u32 = 11;
pub const CT_TYPES: u32 = 12;

pub const INP_KEY_DOWN: u32 = 1;
pub const INP_KEY_UP: u32 = 2;
pub const INP_CHAR: u32 = 3;
pub const INP_MOUSE_MOVE: u32 = 4;
pub const INP_MOUSE_DOWN: u32 = 5;
pub const INP_MOUSE_UP: u32 = 6;
pub const INP_WHEEL: u32 = 7;
pub const INPM_SHIFT: u32 = 0x01;
pub const INPM_CTRL: u32 = 0x02;
pub const INPM_ALT: u32 = 0x04;
pub const INPM_REPEAT: u32 = 0x40;
pub const BTN_LEFT: u32 = 0;
pub const VK_BACK: u32 = 0x08;
pub const VK_TAB: u32 = 0x09;
pub const VK_RETURN: u32 = 0x0d;
pub const VK_ESCAPE: u32 = 0x1b;
pub const VK_SPACE: u32 = 0x20;
pub const VK_PRIOR: u32 = 0x21;
pub const VK_NEXT: u32 = 0x22;
pub const VK_END: u32 = 0x23;
pub const VK_HOME: u32 = 0x24;
pub const VK_LEFT: u32 = 0x25;
pub const VK_UP: u32 = 0x26;
pub const VK_RIGHT: u32 = 0x27;
pub const VK_DOWN: u32 = 0x28;
pub const VK_DELETE: u32 = 0x2e;

pub const FONT_UI: c_int = 0;
pub const FONT_UI_BOLD: c_int = 1;
pub const FONT_INS_BOLD_24: c_int = 16;
pub const FONT_IDS: u32 = 21;
pub const CTL_FONT_AUTO: u32 = FONT_IDS;
pub const LUNA_FOCUS_MARGIN: i32 = 4;
pub const LS_NORMAL: i32 = 0;
pub const LS_HOT: i32 = 1;
pub const LS_PRESSED: i32 = 2;
pub const LS_DISABLED: i32 = 3;
pub const LS_DEFAULT: i32 = 4;
pub const LS_FOCUSED: i32 = 5;
pub const LUNA_THEME_LIGHT: i32 = 0;
pub const LUNA_THEME_DARK: i32 = 1;
pub const GFX_REGION_MAX: usize = 32;

pub const VT_ACCENT: u32 = 0xff0f_7d83;
pub const VT_ACCENT_HI: u32 = 0xff2b_b3b1;
pub const VT_OPAQUE_CONTENT: u32 = 0xfffb_fbfc;
pub const VT_OPAQUE_CONTENT_D: u32 = 0xff14_1824;

pub type t_color = u32;
pub type t_ctlcb = Option<unsafe extern "C" fn(*mut c_void, u32, *mut c_void)>;
pub type t_ctlclock = Option<unsafe extern "C" fn() -> i64>;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_point {
    pub x: i32,
    pub y: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_rect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_glyph {
    pub cp: u32,
    pub w: u8,
    pub h: u8,
    pub xoff: i8,
    pub yoff: i8,
    pub advance: u8,
    pub off: u16,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_font {
    pub ascent: i32,
    pub descent: i32,
    pub height: i32,
    pub nglyphs: i32,
    pub glyphs: *const t_glyph,
    pub bits: *const u8,
    pub nbits: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_surface {
    pub px: *mut u32,
    pub w: i32,
    pub h: i32,
    pub stride: i32,
    pub clip: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_region {
    pub r: [t_rect; GFX_REGION_MAX],
    pub n: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_textreq {
    pub font: *const t_font,
    pub at: t_point,
    pub color: t_color,
    pub text: *const c_char,
    pub len: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_inpevent {
    pub r#type: u32,
    pub code: u32,
    pub x: i32,
    pub y: i32,
    pub mods: u32,
    pub scancode: u32,
    pub time_ns: u64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_wmhdr {
    pub magic: u32,
    pub r#type: u32,
    pub size: u32,
    pub window: u32,
    pub seq: u32,
    pub status: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_wmmouse {
    pub h: t_wmhdr,
    pub r#type: u32,
    pub x: i32,
    pub y: i32,
    pub buttons: u32,
    pub wheel: i32,
    pub hit: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lunametrics {
    pub caption_h: i32,
    pub frame_w: i32,
    pub frame_bottom: i32,
    pub btn_w: i32,
    pub btn_h: i32,
    pub taskbar_h: i32,
    pub start_w: i32,
    pub tray_w: i32,
    pub scroll_w: i32,
    pub menu_item_h: i32,
    pub icon_small: i32,
    pub icon_large: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lunabtn {
    pub r: t_rect,
    pub state: i32,
    pub is_default: bool,
    pub focus: bool,
}

#[repr(C)]
pub struct t_ctl {
    pub r#type: u32,
    pub id: u32,
    pub flags: u32,
    pub state: u32,
    pub rect: t_rect,
    pub text: [u8; CTL_TEXT_MAX],
    pub parent: *mut t_ctl,
    pub first: *mut t_ctl,
    pub next: *mut t_ctl,
    pub cb: t_ctlcb,
    pub user: *mut c_void,
    pub r#priv: *mut c_void,
    pub font: u32,
}

#[repr(C)]
pub struct t_ctlroot {
    pub root: *mut t_ctl,
    pub focus: *mut t_ctl,
    pub hot: *mut t_ctl,
    pub capture: *mut t_ctl,
    pub surface: *mut t_surface,
    pub dirty: t_region,
    pub command: t_ctlcb,
    pub user: *mut c_void,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ctlspec {
    pub r#type: u32,
    pub id: u32,
    pub flags: u32,
    pub rect: t_rect,
    pub text: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ctlmenuitem {
    pub text: *const c_char,
    pub id: u32,
    pub flags: u32,
    pub sub: *const t_ctlmenuitem,
    pub nsub: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ctlmenuspec {
    pub id: u32,
    pub at: t_point,
    pub items: *const t_ctlmenuitem,
    pub count: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_mouseev {
    pub kind: u32,
    pub at: t_point,
    pub wheel: i32,
    pub dbl: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_scrst {
    pub pos: i32,
    pub max: i32,
    pub page: i32,
    pub grab: i32,
    pub zone: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_sblo {
    pub dec: t_rect,
    pub inc: t_rect,
    pub track: t_rect,
    pub thumb: t_rect,
    pub vertical: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_sbctx {
    pub r: *mut t_ctlroot,
    pub c: *mut t_ctl,
    pub st: *mut t_scrst,
    pub bar: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ctltext {
    pub r#box: t_rect,
    pub text: *const c_char,
    pub flags: u32,
    pub color: t_color,
    pub enabled: bool,
    pub mnemonic: bool,
    pub font: *const t_font,
}

#[repr(C)]
pub struct t_textrun {
    pub font: *const t_font,
    pub buf: [u8; CTL_TEXT_MAX],
    pub len: i32,
    pub cut: i32,
    pub mn: i32,
    pub width: i32,
    pub ellipsis: bool,
    pub at: t_point,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_edit {
    pub caret: u32,
    pub anchor: u32,
    pub scroll: i32,
    pub max_chars: u32,
}

#[repr(C)]
pub struct t_edview {
    pub font: *const t_font,
    pub disp: [u8; CTL_TEXT_MAX],
    pub len: i32,
    pub org: t_point,
    pub col: t_color,
}

#[repr(C)]
pub struct t_clip {
    pub data: [u8; CTL_TEXT_MAX],
    pub len: u32,
}

#[repr(C)]
pub struct t_list {
    pub items: *mut *mut c_char,
    pub count: u32,
    pub cap: u32,
    pub sel: i32,
    pub sb: t_scrst,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lstgeom {
    pub items: t_rect,
    pub bar: t_rect,
    pub rowh: i32,
    pub rows: i32,
    pub bar_on: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_menulevel {
    pub items: *const t_ctlmenuitem,
    pub count: u32,
    pub sel: i32,
    pub rect: t_rect,
}

#[repr(C)]
pub struct t_menu {
    pub depth: u32,
    pub picked: u32,
    pub prev: *mut t_ctl,
    pub lv: [t_menulevel; CTL_MENU_LEVELS_MAX as usize],
}

#[repr(C)]
pub struct t_pool {
    pub cur: *mut c_char,
    pub left: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_progress {
    pub pct: u32,
    pub reserved: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_rootpriv {
    pub clock: t_ctlclock,
    pub click_ns: i64,
    pub click_at: t_point,
    pub click_ctl: usize,
    pub buttons: u32,
    pub reserved: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_focusscan {
    pub first: *mut t_ctl,
    pub last: *mut t_ctl,
    pub before: *mut t_ctl,
    pub after: *mut t_ctl,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ctlops {
    pub priv_size: usize,
    pub supported: bool,
    pub hot: bool,
    pub init: Option<unsafe extern "C" fn(*mut t_ctl)>,
    pub destroy: Option<unsafe extern "C" fn(*mut t_ctl)>,
    pub paint: Option<unsafe extern "C" fn(*mut t_ctlroot, *mut t_ctl)>,
    pub mouse: Option<unsafe extern "C" fn(*mut t_ctlroot, *mut t_ctl, *const t_mouseev) -> bool>,
    pub key: Option<unsafe extern "C" fn(*mut t_ctlroot, *mut t_ctl, *const t_inpevent) -> bool>,
    pub activate: Option<unsafe extern "C" fn(*mut t_ctlroot, *mut t_ctl) -> bool>,
}

impl t_ctl {
    pub(crate) fn text_c(&self) -> *const c_char {
        self.text.as_ptr().cast()
    }

    pub(crate) fn text_c_mut(&mut self) -> *mut c_char {
        self.text.as_mut_ptr().cast()
    }
}
