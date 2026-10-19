#![allow(non_camel_case_types)]

use core::ffi::c_char;

pub const E_OK: i32 = 0;
pub const E_PERM: i32 = -1;
pub const E_NOENT: i32 = -2;
pub const E_IO: i32 = -5;
pub const E_NOMEM: i32 = -12;
pub const E_ACCES: i32 = -13;
pub const E_EXIST: i32 = -17;
pub const E_INVAL: i32 = -22;
pub const E_NOSPC: i32 = -28;
pub const E_RANGE: i32 = -34;
pub const E_OVERFLOW: i32 = -75;
pub const E_NOTSUP: i32 = -95;

pub const HT_NOWHERE: u32 = 0;
pub const HT_CLIENT: u32 = 1;
pub const HT_CAPTION: u32 = 2;
pub const HT_SYSMENU: u32 = 3;
pub const HT_MINBUTTON: u32 = 8;
pub const HT_MAXBUTTON: u32 = 9;
pub const HT_LEFT: u32 = 10;
pub const HT_RIGHT: u32 = 11;
pub const HT_TOP: u32 = 12;
pub const HT_TOPLEFT: u32 = 13;
pub const HT_TOPRIGHT: u32 = 14;
pub const HT_BOTTOM: u32 = 15;
pub const HT_BOTTOMLEFT: u32 = 16;
pub const HT_BOTTOMRIGHT: u32 = 17;
pub const HT_CLOSE: u32 = 20;

pub const WS_CAPTION: u32 = 0x0001;
pub const WS_SYSMENU: u32 = 0x0002;
pub const WS_MINBOX: u32 = 0x0004;
pub const WS_MAXBOX: u32 = 0x0008;
pub const WS_SIZEBOX: u32 = 0x0010;
pub const WS_POPUP: u32 = 0x0020;
pub const WS_TOOLWINDOW: u32 = 0x0040;
pub const WS_NOACTIVATE: u32 = 0x0080;
pub const WS_TOPMOST: u32 = 0x0100;
pub const WS_DESKTOP: u32 = 0x0200;
pub const WS_APPBAR: u32 = 0x0400;
pub const WS_FULLSCREEN: u32 = 0x0800;
pub const WS_DEFAULT: u32 = 0x001f;

pub const LS_NORMAL: i32 = 0;
pub const LS_HOT: i32 = 1;
pub const LS_PRESSED: i32 = 2;
pub const LS_DISABLED: i32 = 3;
pub const LS_DEFAULT: i32 = 4;
pub const LS_FOCUSED: i32 = 5;

pub const ICON_NONE: i32 = 0;
pub const ICON_LOGO: i32 = 1;
pub const ICON_COMPUTER: i32 = 2;
pub const ICON_FOLDER: i32 = 3;
pub const ICON_DOCUMENT: i32 = 4;
pub const ICON_RECYCLE: i32 = 5;
pub const ICON_DRIVE: i32 = 6;
pub const ICON_SETTINGS: i32 = 7;
pub const ICON_RUN: i32 = 8;
pub const ICON_SEARCH: i32 = 9;
pub const ICON_HELP: i32 = 10;
pub const ICON_POWER: i32 = 11;
pub const ICON_USER: i32 = 12;
pub const ICON_PROGRAM: i32 = 13;
pub const ICON_IDS: i32 = 14;

pub const LUNA_THEME_LIGHT: i32 = 0;
pub const LUNA_THEME_DARK: i32 = 1;

pub const LUNA_GLASS_DOCK: i32 = 0;
pub const LUNA_GLASS_LAUNCHER: i32 = 1;
pub const LUNA_GLASS_PANEL: i32 = 2;
pub const LUNA_GLASS_CARD: i32 = 3;
pub const LUNA_GLASS_CHIP: i32 = 4;
pub const LUNA_GLASS_FRAME: i32 = 5;
pub const LUNA_GLASS_WINDOW: i32 = 6;
pub const LUNA_GLASS_CONSOLE: i32 = 7;
pub const LUNA_GLASS_FLOAT: i32 = 8;
pub const LUNA_GLASS_KINDS: i32 = 9;

pub const LUNA_COLOR_TEXT: i32 = 0;
pub const LUNA_COLOR_TEXT_2: i32 = 1;
pub const LUNA_COLOR_MUTED: i32 = 2;
pub const LUNA_COLOR_DIM: i32 = 3;
pub const LUNA_COLOR_ACCENT: i32 = 4;
pub const LUNA_COLOR_ACCENT_HI: i32 = 5;
pub const LUNA_COLOR_ACCENT_TEXT: i32 = 6;
pub const LUNA_COLOR_DANGER: i32 = 7;
pub const LUNA_COLOR_WINDOW: i32 = 8;
pub const LUNA_COLOR_PANEL: i32 = 9;
pub const LUNA_COLOR_SELECTION_BG: i32 = 10;
pub const LUNA_COLOR_SELECTION_RING: i32 = 11;
pub const LUNA_COLORS: i32 = 12;

pub const LUNA_FOCUS_MARGIN: i32 = 4;

pub const LUNA_LAUNCHER_WIDTH: i32 = 540;
pub const LUNA_LAUNCHER_HEIGHT: i32 = 289;
pub const LUNA_LAUNCHER_MARGIN_X: i32 = 12;
pub const LUNA_LAUNCHER_TILES: i32 = 4;
pub const LUNA_LAUNCHER_NONE: i32 = -1;
pub const LUNA_LAUNCHER_LOGOFF: i32 = 4;
pub const LUNA_LAUNCHER_POWER: i32 = 5;

pub const LUNA_LM_QUERY_MAX: usize = 48;
pub const LUNA_LM_LABEL_MAX: usize = 48;
pub const LUNA_LM_APPS_MAX: u32 = 80;
pub const LUNA_LM_RECENTS: u32 = 4;
pub const LUNA_LM_ROWS: i32 = 4;
pub const LUNA_LM_NONE: i32 = -1;

pub const LUNA_LV_PINNED: i32 = 0;
pub const LUNA_LV_RECENT: i32 = 1;
pub const LUNA_LV_ALL: i32 = 2;
pub const LUNA_LV_SEARCH: i32 = 3;
pub const LUNA_LV_COUNT: i32 = 4;

pub const LG_NBTN: i32 = 3;
pub const LG_FRAMED: u32 = 0x0001;
pub const LG_SIZABLE: u32 = 0x0002;
pub const LG_MAX: u32 = 0x0004;
pub const LG_TOOL: u32 = 0x0008;
pub const LG_SYSMENU: u32 = 0x0010;
pub const LG_SHOWN: u32 = 0x0100;
pub const LG_ENABLED: u32 = 0x1000;
pub const LG_FRAMELESS_STYLES: u32 = 0x0e00;
pub const LG_DIM_MAX: i32 = 1_048_576;
pub const LG_POS_MAX: i32 = 268_435_456;

pub const LP_COORD_MAX: i32 = 16_777_216;
pub const LP_WALL_MAX: i32 = 16384;
pub const LP_RMAX: i32 = 32;
pub const LP_PMAX: usize = 6;
pub const LP_OPS_ARGS: usize = 12;
pub const LP_TEXT_MAX: i32 = 4096;
pub const LP_ICON_MAX: i32 = 512;
pub const LP_LETTERS: usize = 7;
pub const LP_LETTER_GAP: i32 = 5;
pub const LP_BLOBS: usize = 4;
pub const LX_SCALE: i32 = 4;
pub const LX_PASSES: i32 = 3;
pub const LP_VEILS: usize = 2;

pub const LA_LEFT: i32 = 0;
pub const LA_CENTER: i32 = 1;
pub const LA_RIGHT: i32 = 2;

pub const LOP_RECT: u8 = 1;
pub const LOP_RRECT: u8 = 2;
pub const LOP_DISC: u8 = 3;
pub const LOP_POLY: u8 = 4;
pub const LOP_LINE: u8 = 5;
pub const LOP_RING: u8 = 6;

pub const LC_BLACK: u32 = 0xff00_0000;
pub const LC_WHITE: u32 = 0xffff_ffff;
pub const LC_WINDOW: u32 = 0xffec_e9d8;
pub const LC_SELECTION: u32 = 0xff31_6ac5;
pub const LC_TEXT: u32 = 0xff00_0000;
pub const LC_GRAYTEXT: u32 = 0xffac_a899;

pub const FONT_UI: i32 = 0;
pub const FONT_UI_BOLD: i32 = 1;
pub const FONT_TITLE: i32 = 2;

pub const VT_ACCENT: u32 = 0xff0f_7d83;
pub const VT_ACCENT_HI: u32 = 0xff2b_b3b1;
pub const VT_ACCENT_TEXT_DARK: u32 = 0xff4f_d1cc;
pub const VT_ACCENT_SEL_BG: u32 = 0x1f0f_7d83;
pub const VT_ACCENT_SEL_RING: u32 = 0x730f_7d83;
pub const VT_ACCENT_SEL_BG_D: u32 = 0x1f4f_d1cc;
pub const VT_ACCENT_SEL_RING_D: u32 = 0x804f_d1cc;
pub const VT_INK: u32 = 0xff14_1824;
pub const VT_INK_2: u32 = 0xff2c_3242;
pub const VT_MUTED: u32 = 0xff4a_5163;
pub const VT_DIM: u32 = 0xffaa_b2c3;
pub const VT_INK_D: u32 = 0xfff1_f3f8;
pub const VT_INK_2_D: u32 = 0xffc9_d0dc;
pub const VT_MUTED_D: u32 = 0xffaa_b2c3;
pub const VT_DIM_D: u32 = 0xff5d_667a;
pub const VT_DANGER: u32 = 0xffb3_361f;
pub const VT_DANGER_D: u32 = 0xffff_9a7a;
pub const VT_GLASS_FRAME: u32 = 0x4dff_ffff;
pub const VT_GLASS_DOCK: u32 = 0x52ff_ffff;
pub const VT_GLASS_CHIP: u32 = 0x57ff_ffff;
pub const VT_GLASS_PANEL: u32 = 0x61ff_ffff;
pub const VT_GLASS_WINDOW: u32 = 0x8fff_ffff;
pub const VT_GLASS_CONTENT: u32 = 0x9eff_ffff;
pub const VT_GLASS_CONTROL: u32 = 0x8cff_ffff;
pub const VT_GLASS_ITEM_SEL: u32 = 3_019_898_879;
pub const VT_GLASS_INPUT: u32 = 3_103_784_959;
pub const VT_GLASS_FRAME_D: u32 = 0x6b16_1a26;
pub const VT_GLASS_DOCK_D: u32 = 0x6b14_1824;
pub const VT_GLASS_CHIP_D: u32 = 0x7316_1a26;
pub const VT_GLASS_PANEL_D: u32 = 0x8016_1a26;
pub const VT_GLASS_WINDOW_D: u32 = 0x9e1a_1e2c;
pub const VT_GLASS_CONTENT_D: u32 = 0x9910_131c;
pub const VT_GLASS_CONSOLE: u32 = 0xc70e_111a;
pub const VT_GLASS_CONTROL_D: u32 = 0x14ff_ffff;
pub const VT_GLASS_ITEM_SEL_D: u32 = 0x1fff_ffff;
pub const VT_GLASS_INPUT_D: u32 = 0x14ff_ffff;
pub const VT_BLUR_CHIP: i32 = 24;
pub const VT_BLUR_DOCK: i32 = 28;
pub const VT_BLUR_WINDOW: i32 = 30;
pub const VT_BLUR_PANEL: i32 = 34;
pub const VT_SAT_LIGHT_X10: i32 = 18;
pub const VT_SAT_DARK_X10: i32 = 16;
pub const VT_SAT_PANEL_LIGHT_X10: i32 = 19;
pub const VT_SAT_PANEL_DARK_X10: i32 = 17;
pub const VT_RIM_TOP: u32 = 0xf2ff_ffff;
pub const VT_RIM_IN: u32 = 0x73ff_ffff;
pub const VT_RIM_BOTTOM: u32 = 0x4dff_ffff;
pub const VT_RIM_TOP_D: u32 = 0x38ff_ffff;
pub const VT_RIM_IN_D: u32 = 0x14ff_ffff;
pub const VT_RIM_BOTTOM_D: u32 = 0x00ff_ffff;
pub const VT_SPEC_TOP: u32 = 0x73ff_ffff;
pub const VT_SPEC_TOP_D: u32 = 0x1fff_ffff;
pub const VT_SPEC_END_PCT: i32 = 32;
pub const VT_SHADOW_WIN: u32 = 0x4728_2850;
pub const VT_SHADOW_DOCK: u32 = 0x3828_2850;
pub const VT_SHADOW_WIN_D: u32 = 0x8000_0000;
pub const VT_SHADOW_DOCK_D: u32 = 0x7300_0000;
pub const VT_OPAQUE_PANEL: u32 = 0xffee_f0f5;
pub const VT_OPAQUE_CONTENT: u32 = 0xfffb_fbfc;
pub const VT_OPAQUE_PANEL_D: u32 = 0xff1b_2030;
pub const VT_OPAQUE_CONTENT_D: u32 = 0xff14_1824;
pub const VT_RADIO_BORDER: u32 = 0xff8a_93a8;
pub const VT_RADIO_BORDER_D: u32 = 0xff5d_667a;
pub const VT_R_WINDOW: i32 = 24;
pub const VT_R_WINDOW_LG: i32 = 26;
pub const VT_R_DOCK: i32 = 22;
pub const VT_R_PANEL: i32 = 30;
pub const VT_R_TILE: i32 = 20;
pub const VT_R_CARD: i32 = 18;
pub const VT_R_TRAY: i32 = 16;
pub const VT_R_TASK: i32 = 14;
pub const VT_R_ITEM: i32 = 12;
pub const VT_R_CAPBTN: i32 = 9;
pub const VT_DOCK_MARGIN: i32 = 12;
pub const VT_DOCK_H: i32 = 56;
pub const VT_DOCK_PAD_X: i32 = 8;
pub const VT_DOCK_GAP: i32 = 8;
pub const VT_LAUNCHER_D: i32 = 42;
pub const VT_TASK_H: i32 = 40;
pub const VT_TASK_ICON: i32 = 24;
pub const VT_TASK_IND_W: i32 = 14;
pub const VT_TRAY_H: i32 = 42;
pub const VT_BADGE: i32 = 30;
pub const VT_CAPTION_H: i32 = 50;
pub const VT_CAPBTN_W: i32 = 28;
pub const VT_CAPBTN_H: i32 = 24;
pub const VT_CAPGROUP_PAD: i32 = 3;
pub const VT_WALL_BASE: u32 = 0xffe3_e6f0;
pub const VT_WALL_BASE_D: u32 = 0xff0d_1120;
pub const VT_WALL_TEAL: u32 = 0xff8f_d6d2;
pub const VT_WALL_TEAL_D: u32 = 0xff1f_6f72;
pub const VT_WALL_VIOLET: u32 = 0xffbc_aaf2;
pub const VT_WALL_VIOLET_D: u32 = 0xff4b_3c8f;
pub const VT_WALL_AMBER: u32 = 0xfff6_c890;
pub const VT_WALL_AMBER_D: u32 = 0xff8a_5a2a;
pub const VT_WALL_ROSE: u32 = 0xffee_a3a6;
pub const VT_WALL_ROSE_D: u32 = 0xff7a_3640;

pub const LD_TRAY_PAD_L: i32 = 14;
pub const LD_TRAY_PAD_R: i32 = 6;
pub const LD_TRAY_GAP: i32 = 14;
pub const LD_TRAY_LANG_W: i32 = 17;
pub const LD_TRAY_DOTS_W: i32 = 38;
pub const LD_TRAY_CLOCK_W: i32 = 74;
pub const LD_TRAY_W: i32 = 221;
pub const LD_STRIP_PAD_L: i32 = 4;

pub const LCT_RAD_CTRL: i32 = 12;
pub const LCT_RAD_FIELD: i32 = 10;
pub const LCT_RAD_GROUP: i32 = 14;
pub const LCT_RAD_ITEM: i32 = 8;
pub const LCT_RAD_MENU: i32 = 10;
pub const LCT_RIM_PRIMARY: u32 = 0x73ff_ffff;
pub const LCT_FOCUS_GAP: i32 = 2;
pub const LCT_FOCUS_W: i32 = 2;

pub const LL_PAD: i32 = 18;
pub const LL_GAP: i32 = 6;
pub const LL_SEARCH_H: i32 = 48;
pub const LL_SEARCH_ICON: i32 = 26;
pub const LL_SEARCH_PAD: i32 = 10;
pub const LL_SEARCH_PAD_R: i32 = 18;
pub const LL_TITLE_H: i32 = 16;
pub const LL_TITLE_GAP: i32 = 10;
pub const LL_TITLE_PAD: i32 = 6;
pub const LL_TILE_H: i32 = 96;
pub const LL_TILE_TOP: i32 = 14;
pub const LL_FOOT_H: i32 = 36;
pub const LL_FOOT_PAD: i32 = 10;
pub const LL_AVATAR: i32 = 34;
pub const LL_LOGOFF_W: i32 = 135;
pub const LL_ICON: i32 = 44;
pub const LL_POWER_ICON: i32 = 26;
pub const LL_LABEL_H: i32 = 16;
pub const LL_FOCUS_GAP: i32 = 2;
pub const LL_MIN_W: i32 = 320;
pub const LL_MIN_H: i32 = 289;
pub const LL_HOT_L: u32 = 0x9eff_ffff;
pub const LL_HOT_D: u32 = 0x1aff_ffff;
pub const LL_HOT_RIM_L: u32 = 0xffff_ffff;
pub const LL_HOT_RIM_D: u32 = 0x26ff_ffff;
pub const LL_BTN_RIM_L: u32 = 0xffff_ffff;
pub const LL_BTN_RIM_D: u32 = 0x1fff_ffff;
pub const LL_SEP_L: u32 = 0x99ff_ffff;
pub const LL_SEP_D: u32 = 0x14ff_ffff;
pub const LL_INPUT_RIM_L: u32 = 0x1414_1824;
pub const LL_INPUT_RIM_D: u32 = 0x1aff_ffff;
pub const LL_ST_HOT: i32 = 1;
pub const LL_ST_FOCUS: i32 = 2;
pub const LL_ROW_H: i32 = 26;
pub const LL_ROW_ICON: i32 = 20;
pub const LL_ROW_PAD: i32 = 8;
pub const LL_TAB_PAD: i32 = 4;
pub const LL_BAR_W: i32 = 3;
pub const LL_BAR_MIN: i32 = 12;
pub const LL_FOOT_GAP: i32 = 11;

pub type t_color = u32;
pub type t_lunatheme = i32;
pub type t_glasskind = i32;
pub type t_iconid = i32;
pub type t_lunastate = i32;
pub type t_lunacolor = i32;
pub type t_fontid = i32;
pub type t_lmview = i32;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_point {
    pub x: i32,
    pub y: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_rect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
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
pub struct t_textreq {
    pub font: *const t_font,
    pub at: t_point,
    pub color: t_color,
    pub text: *const c_char,
    pub len: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
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
pub struct t_lunawin {
    pub outer: t_rect,
    pub style: u32,
    pub title: *const c_char,
    pub icon: i32,
    pub active: bool,
    pub maximized: bool,
    pub hot: u32,
    pub pressed: u32,
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
#[derive(Clone, Copy)]
pub struct t_lunascroll {
    pub r: t_rect,
    pub total: i32,
    pub page: i32,
    pub pos: i32,
    pub up: i32,
    pub down: i32,
    pub thumb: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_scrollparts {
    pub up: t_rect,
    pub down: t_rect,
    pub track: t_rect,
    pub thumb: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_taskbarlayout {
    pub start: t_rect,
    pub tasks: t_rect,
    pub tray: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct t_traygeo {
    pub lang: t_rect,
    pub dots: t_rect,
    pub clock: t_rect,
    pub badge: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_launchertile {
    pub label: *const c_char,
    pub icon: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_launcher {
    pub user: *const c_char,
    pub tiles: *const t_launchertile,
    pub ntiles: i32,
    pub hot: i32,
    pub focus: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_launcherlayout {
    pub panel: t_rect,
    pub search: t_rect,
    pub title: t_rect,
    pub tile: [t_rect; 4],
    pub sep: t_rect,
    pub avatar: t_rect,
    pub user: t_rect,
    pub logoff: t_rect,
    pub poweroff: t_rect,
    pub ntiles: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lmapp {
    pub label: [c_char; LUNA_LM_LABEL_MAX],
    pub id: u32,
    pub icon: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lmodel {
    pub query: [c_char; LUNA_LM_QUERY_MAX],
    pub qlen: u32,
    pub apps: [t_lmapp; LUNA_LM_APPS_MAX as usize],
    pub napps: u32,
    pub recent: [u32; LUNA_LM_RECENTS as usize],
    pub nrecent: u32,
    pub shown: [u32; LUNA_LM_APPS_MAX as usize],
    pub nshown: u32,
    pub tab: i32,
    pub top: i32,
    pub sel: i32,
    pub hot: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lpal {
    pub accent: t_color,
    pub accent_hi: t_color,
    pub accent_text: t_color,
    pub sel_bg: t_color,
    pub sel_ring: t_color,
    pub ink: t_color,
    pub ink_2: t_color,
    pub muted: t_color,
    pub dim: t_color,
    pub danger: t_color,
    pub frame: t_color,
    pub dock: t_color,
    pub chip: t_color,
    pub panel: t_color,
    pub window: t_color,
    pub content: t_color,
    pub control: t_color,
    pub sel: t_color,
    pub input: t_color,
    pub hover: t_color,
    pub press: t_color,
    pub rim_top: t_color,
    pub rim_in: t_color,
    pub rim_bottom: t_color,
    pub spec: t_color,
    pub shadow_win: t_color,
    pub shadow_dock: t_color,
    pub opaque_panel: t_color,
    pub opaque_content: t_color,
    pub wall: [t_color; 5],
    pub sat: i32,
    pub sat_panel: i32,
    pub veil_a: i32,
    pub veil_b: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lunadetail {
    pub btn_gap: i32,
    pub btn_top: i32,
    pub btn_right: i32,
    pub btn_top_max: i32,
    pub btn_right_max: i32,
    pub sysmenu_w: i32,
    pub title_pad: i32,
    pub corner_len: i32,
    pub tool_caption_h: i32,
    pub tool_btn: i32,
    pub tool_btn_top: i32,
    pub tool_btn_right: i32,
    pub check_size: i32,
    pub group_line_y: i32,
    pub group_pad: i32,
    pub progress_block: i32,
    pub progress_gap: i32,
    pub menu_header_h: i32,
    pub menu_footer_h: i32,
    pub menu_foot_item_w: i32,
    pub menu_foot_icon: i32,
    pub avatar: i32,
    pub task_gap: i32,
    pub thumb_min: i32,
    pub title_radius: i32,
    pub resize: i32,
    pub icon_left: i32,
    pub caption_icon: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lgeo {
    pub outer: t_rect,
    pub caption: t_rect,
    pub sysmenu: t_rect,
    pub icon: t_rect,
    pub title: t_rect,
    pub client: t_rect,
    pub btn: [t_rect; LG_NBTN as usize],
    pub border: i32,
    pub top: i32,
    pub bottom: i32,
    pub cap_end: i32,
    pub flags: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lbp {
    pub size: i32,
    pub height: i32,
    pub top: i32,
    pub right: i32,
    pub left: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lstop {
    pub at: i32,
    pub c: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lgrad {
    pub st: *const t_lstop,
    pub n: i32,
    pub tint: t_color,
    pub tint_t: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lrr {
    pub r: t_rect,
    pub rad: [i32; 4],
    pub g: *const t_lgrad,
    pub y0: i32,
    pub hole: t_rect,
    pub vis: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_llayers {
    pub r: t_rect,
    pub rad: [i32; 4],
    pub ring: *const t_color,
    pub nring: i32,
    pub fill: *const t_lgrad,
    pub hole: t_rect,
    pub skip: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lbox {
    pub r: t_rect,
    pub rad: i32,
    pub ring: *const t_color,
    pub nring: i32,
    pub fill: *const t_lgrad,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lspan {
    pub y: i32,
    pub x0: i32,
    pub x1: i32,
    pub c: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ledge {
    pub x: i32,
    pub y: i32,
    pub dir: i32,
    pub rad: i32,
    pub iy: i32,
    pub c: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lbc {
    pub cx: i32,
    pub cy: i32,
    pub irx: i32,
    pub iry: i32,
    pub thr: i32,
    pub ithr: i32,
    pub col: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lvc {
    pub cx: i32,
    pub cy: i32,
    pub a: i32,
    pub b: i32,
    pub co: i32,
    pub si: i32,
    pub sigma: i32,
    pub peak: i32,
    pub down: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lwall {
    pub w: i32,
    pub h: i32,
    pub ox: i32,
    pub oy: i32,
    pub vis: t_rect,
    pub bc: [t_lbc; LP_BLOBS],
    pub vc: [t_lvc; LP_VEILS],
    pub base: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lglyph {
    pub pts: *const u8,
    pub w: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lmk {
    pub org: t_point,
    pub unit: i32,
    pub c: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lboot {
    pub emblem: t_rect,
    pub mark: t_rect,
    pub bar: t_rect,
    pub text: t_rect,
    pub card: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lcring {
    pub x: i32,
    pub y: i32,
    pub dx: i32,
    pub dy: i32,
    pub rad: i32,
    pub c: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lpoly {
    pub p: [t_point; LP_PMAX],
    pub n: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lpe {
    pub a: i64,
    pub b: i64,
    pub c: i64,
    pub inv: i64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lpctx {
    pub e: [t_lpe; LP_PMAX],
    pub n: i32,
    pub vis: t_rect,
    pub top: i32,
    pub g: *const t_lgrad,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lg2 {
    pub g: t_lgrad,
    pub st: [t_lstop; 2],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ldisc {
    pub cx: i32,
    pub cy: i32,
    pub r: i32,
    pub r_in: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lline {
    pub a: t_point,
    pub b: t_point,
    pub w: i32,
    pub round: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lgb {
    pub a: *mut u32,
    pub b: *mut u32,
    pub w: i32,
    pub h: i32,
    pub src: t_rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lgl {
    pub n: i32,
    pub step: i32,
    pub r: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lglass {
    pub r: t_rect,
    pub hole: t_rect,
    pub rad_top: i32,
    pub rad_bot: i32,
    pub blur: i32,
    pub sat: i32,
    pub fill: t_color,
    pub opaque: t_color,
    pub spec: t_color,
    pub rim: [t_color; 3],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_gkind {
    pub fill: [t_color; 2],
    pub blur: i32,
    pub rad: i32,
    pub panel: bool,
    pub pill: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lscale {
    pub ox: i32,
    pub oy: i32,
    pub num: i32,
    pub den: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lop {
    pub kind: u8,
    pub n: u8,
    pub a: [u8; LP_OPS_ARGS],
    pub c0: t_color,
    pub c1: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_loplist {
    pub ops: *const t_lop,
    pub n: i32,
    pub flat: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_liconent {
    pub id: i32,
    pub ops: *const t_lop,
    pub n: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ltext {
    pub font: i32,
    pub color: t_color,
    pub shadow: t_color,
    pub text: *const c_char,
    pub box_: t_rect,
    pub align: i32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ltx {
    pub f: *const t_font,
    pub text: *const c_char,
    pub p: t_point,
    pub n: i32,
    pub ell: i32,
    pub c: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lcb {
    pub r: t_rect,
    pub kind: u32,
    pub st: i32,
    pub active: bool,
    pub maximized: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lrfill {
    pub r: t_rect,
    pub rad: i32,
    pub top: t_color,
    pub bottom: t_color,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_lchoice {
    pub r: t_rect,
    pub rad: i32,
    pub st: i32,
    pub on: bool,
    pub solid: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_ldplate {
    pub r: t_rect,
    pub rad: i32,
    pub c: t_color,
    pub thick: i32,
}

pub type t_ldcov = unsafe extern "C" fn(*const t_ldplate, i32, i32) -> i32;

#[cfg(not(test))]
mod sys {
    use super::{t_color, t_font, t_point, t_rect, t_surface, t_textreq};
    use core::ffi::c_char;

    unsafe extern "C" {
        pub fn gfx_put(s: *mut t_surface, p: t_point, c: t_color);
        pub fn gfx_get(s: *const t_surface, p: t_point) -> t_color;
        pub fn gfx_fill(s: *mut t_surface, r: t_rect, c: t_color);
        pub fn font_get(id: i32) -> *const t_font;
        pub fn font_text_width(f: *const t_font, text: *const c_char, len: i32) -> i32;
        pub fn font_fit(f: *const t_font, text: *const c_char, max_w: i32) -> i32;
        pub fn font_draw(dst: *mut t_surface, rq: *const t_textreq);
    }
}

pub(crate) fn gfx_put(s: *mut t_surface, p: t_point, c: t_color) {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: gfx_put reste un symbole extern. Le gfx noyau ou le faux du lot hôte le fournit.
        sys::gfx_put(s, p, c);
    }
    #[cfg(test)]
    crate::hostgfx::put(s, p, c);
}

pub(crate) fn gfx_get(s: *const t_surface, p: t_point) -> t_color {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: gfx_get reste un symbole extern, appelé et non inliné.
        sys::gfx_get(s, p)
    }
    #[cfg(test)]
    crate::hostgfx::get(s, p)
}

pub(crate) fn gfx_fill(s: *mut t_surface, r: t_rect, c: t_color) {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: gfx_fill reste un symbole extern. Le gfx noyau ou le faux du lot hôte le fournit.
        sys::gfx_fill(s, r, c);
    }
    #[cfg(test)]
    crate::hostgfx::fill(s, r, c);
}

pub(crate) fn font_get(id: i32) -> *const t_font {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: font_get reste un symbole extern. La fonte noyau ou le faux du lot hôte le fournit.
        sys::font_get(id)
    }
    #[cfg(test)]
    crate::hostgfx::font_get(id)
}

pub(crate) fn font_text_width(f: *const t_font, text: *const c_char, len: i32) -> i32 {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: font_text_width reste un symbole extern, appelé comme en C.
        sys::font_text_width(f, text, len)
    }
    #[cfg(test)]
    crate::hostgfx::font_text_width(f, text, len)
}

pub(crate) fn font_fit(f: *const t_font, text: *const c_char, max_w: i32) -> i32 {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: font_fit reste un symbole extern, appelé comme en C.
        sys::font_fit(f, text, max_w)
    }
    #[cfg(test)]
    crate::hostgfx::font_fit(f, text, max_w)
}

pub(crate) fn font_draw(dst: *mut t_surface, rq: *const t_textreq) {
    #[cfg(not(test))]
    unsafe {
        // SAFETY: font_draw reste un symbole extern. La fonte noyau ou le faux du lot hôte le fournit.
        sys::font_draw(dst, rq);
    }
    #[cfg(test)]
    crate::hostgfx::font_draw(dst, rq);
}
