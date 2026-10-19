use core::ffi::{c_char, c_int, c_void};

pub const DROID_VIEWS_MAX: u32 = 64;
pub const DROID_KIDS_MAX: u32 = 16;
pub const DROID_TEXT_MAX: usize = 256;
pub const DROID_HORIZONTAL: u32 = 0;
pub const DROID_VERTICAL: u32 = 1;
pub const DROID_LOG_DEBUG: u32 = 3;
pub const DROID_LOG_INFO: u32 = 4;
pub const DROID_LOG_WARN: u32 = 5;
pub const DROID_LOG_ERROR: u32 = 6;

pub const DV_NONE: u32 = 0;
pub const DV_VIEW: u32 = 1;
pub const DV_TEXT: u32 = 2;
pub const DV_BUTTON: u32 = 3;
pub const DV_LINEAR: u32 = 4;

pub(crate) const DR_SB_CAP: u32 = 1024;
pub(crate) const DR_PU: u32 = 1;
pub(crate) const DR_ST: u32 = 9;
pub(crate) const DVM_THROWN: c_int = 1;
pub(crate) const DVM_NULL: u32 = 0;
pub(crate) const E_NOMEM: c_int = -12;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_TIMEOUT: c_int = -110;
pub(crate) const ACC_PUBLIC: u32 = 0x1;
pub(crate) const ACC_CONSTRUCTOR: u32 = 0x10000;

pub type t_droidlog = Option<unsafe extern "C" fn(*mut c_void, u32, *const c_char, *const c_char)>;
pub type t_droidclock = Option<unsafe extern "C" fn(*mut c_void) -> u64>;
pub type t_dnativefn = unsafe extern "C" fn(*mut t_dvm, *const u32, *mut u64) -> c_int;

#[repr(C)]
pub struct t_dclass {
    _opaque: [u8; 0],
}

#[repr(C)]
pub struct t_dmethod {
    _opaque: [u8; 0],
}

#[repr(C)]
#[derive(Clone, Copy, Default)]
pub struct t_dlimits {
    pub heap_bytes: u32,
    pub stack_words: u32,
    pub depth_max: u32,
    pub classes_max: u32,
    pub budget: u64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dvm {
    pub heap: *mut c_void,
    pub classes: *mut c_void,
    pub host: *mut c_void,
    pub stack: *mut u32,
    pub stack_words: u32,
    pub sp: u32,
    pub depth: u32,
    pub pending: u32,
    pub result: u64,
    pub spent: u64,
    pub lim: t_dlimits,
    pub locals: [u32; 64],
    pub nlocals: u32,
}

impl Default for t_dvm {
    fn default() -> Self {
        Self {
            heap: core::ptr::null_mut(),
            classes: core::ptr::null_mut(),
            host: core::ptr::null_mut(),
            stack: core::ptr::null_mut(),
            stack_words: 0,
            sp: 0,
            depth: 0,
            pending: 0,
            result: 0,
            spent: 0,
            lim: t_dlimits::default(),
            locals: [0; 64],
            nlocals: 0,
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dbuiltin {
    pub desc: *const c_char,
    pub super_desc: *const c_char,
    pub iface_desc: *const c_char,
    pub access: u32,
    pub payload_bytes: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dnative {
    pub cls: *const c_char,
    pub name: *const c_char,
    pub sig: *const c_char,
    pub access: u32,
    pub r#fn: Option<t_dnativefn>,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dname {
    pub name: *const c_char,
    pub sig: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dstr16 {
    pub p: *const u16,
    pub n: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_darrview {
    pub data: *mut c_void,
    pub len: u32,
    pub width: u8,
    pub is_ref: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_drsb {
    pub len: u32,
    pub buf: [u16; DR_SB_CAP as usize],
}

impl Default for t_drsb {
    fn default() -> Self {
        Self {
            len: 0,
            buf: [0; DR_SB_CAP as usize],
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_droidview {
    pub kind: u32,
    pub id: u32,
    pub parent: u32,
    pub orientation: u32,
    pub nkids: u32,
    pub kids: [u32; DROID_KIDS_MAX as usize],
    pub self_ref: u32,
    pub listener: u32,
    pub text: [u8; DROID_TEXT_MAX],
}

impl Default for t_droidview {
    fn default() -> Self {
        Self {
            kind: 0,
            id: 0,
            parent: 0,
            orientation: 0,
            nkids: 0,
            kids: [0; DROID_KIDS_MAX as usize],
            self_ref: 0,
            listener: 0,
            text: [0; DROID_TEXT_MAX],
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_droid {
    pub vm: *mut t_dvm,
    pub activity: u32,
    pub root: u32,
    pub nviews: u32,
    pub dirty: u32,
    pub finished: u32,
    pub log: t_droidlog,
    pub clock: t_droidclock,
    pub user: *mut c_void,
    pub title: [u8; DROID_TEXT_MAX],
    pub error: [u8; DROID_TEXT_MAX],
    pub views: [t_droidview; DROID_VIEWS_MAX as usize],
}

impl Default for t_droid {
    fn default() -> Self {
        Self {
            vm: core::ptr::null_mut(),
            activity: 0,
            root: 0,
            nviews: 0,
            dirty: 0,
            finished: 0,
            log: None,
            clock: None,
            user: core::ptr::null_mut(),
            title: [0; DROID_TEXT_MAX],
            error: [0; DROID_TEXT_MAX],
            views: [t_droidview::default(); DROID_VIEWS_MAX as usize],
        }
    }
}

// SAFETY: pointeurs vers des littéraux immuables, ou fonctions, partagés en lecture seule.
unsafe impl Sync for t_dbuiltin {}
// SAFETY: même contenu que t_dbuiltin, plus un pointeur de fonction immuable.
unsafe impl Sync for t_dnative {}

unsafe extern "C" {
    pub(crate) fn dvm_builtins(vm: *mut t_dvm, tab: *const t_dbuiltin, n: u32) -> c_int;
    pub(crate) fn dvm_natives(vm: *mut t_dvm, tab: *const t_dnative, n: u32) -> c_int;
    pub(crate) fn dvm_class(vm: *mut t_dvm, desc: *const c_char, out: *mut *mut t_dclass) -> c_int;
    pub(crate) fn dvm_method(
        vm: *mut t_dvm,
        c: *mut t_dclass,
        name: *const t_dname,
        out: *mut *const t_dmethod,
    ) -> c_int;
    pub(crate) fn dvm_new(vm: *mut t_dvm, c: *mut t_dclass, out: *mut u32) -> c_int;
    pub(crate) fn dvm_class_of(vm: *mut t_dvm, r: u32) -> *mut t_dclass;
    pub(crate) fn dvm_class_name(c: *const t_dclass) -> *const c_char;
    pub(crate) fn dvm_is_instance(vm: *mut t_dvm, r: u32, c: *const t_dclass) -> c_int;
    pub(crate) fn dvm_payload(vm: *mut t_dvm, r: u32, builtin: *const t_dclass) -> *mut c_void;
    pub(crate) fn dvm_string_utf8_new(vm: *mut t_dvm, utf8: *const c_char, out: *mut u32) -> c_int;
    pub(crate) fn dvm_string_utf16_new(vm: *mut t_dvm, s: t_dstr16, out: *mut u32) -> c_int;
    pub(crate) fn dvm_string_get(vm: *mut t_dvm, r: u32, out: *mut t_dstr16) -> c_int;
    pub(crate) fn dvm_array_view(vm: *mut t_dvm, r: u32, out: *mut t_darrview) -> c_int;
    pub(crate) fn dvm_throw(vm: *mut t_dvm, desc: *const c_char, msg: *const c_char) -> c_int;
    pub(crate) fn dvm_pin(vm: *mut t_dvm, r: u32) -> c_int;
    pub(crate) fn dvm_unpin(vm: *mut t_dvm, r: u32);
    pub(crate) fn dvm_call(
        vm: *mut t_dvm,
        m: *const t_dmethod,
        args: *const u32,
        ret: *mut u64,
    ) -> c_int;
    pub(crate) fn dvm_throwable_message(vm: *mut t_dvm, exc: u32, msg: *mut u32) -> c_int;
    pub(crate) fn dvm_throwable_set_message(vm: *mut t_dvm, exc: u32, msg: u32) -> c_int;
    pub(crate) fn malloc(size: usize) -> *mut c_void;
    pub(crate) fn free(ptr: *mut c_void);
    pub(crate) fn memcpy(dst: *mut c_void, src: *const c_void, n: usize) -> *mut c_void;
    pub(crate) fn memmove(dst: *mut c_void, src: *const c_void, n: usize) -> *mut c_void;
    pub(crate) fn memset(dst: *mut c_void, c: c_int, n: usize) -> *mut c_void;
    pub(crate) fn strlen(s: *const c_char) -> usize;
    pub(crate) fn strcmp(a: *const c_char, b: *const c_char) -> c_int;
}

pub(crate) unsafe fn arg(args: *const u32, i: usize) -> u32 {
    // SAFETY: args pointe au moins i + 1 mots, le temps de la lecture.
    unsafe { args.add(i).read() }
}

pub(crate) unsafe fn c_strlen(s: *const c_char) -> usize {
    // SAFETY: s est une chaîne C terminée par NUL.
    unsafe { strlen(s) }
}

pub(crate) unsafe fn c_eq(a: *const c_char, b: *const c_char) -> bool {
    // SAFETY: a et b sont des chaînes C, ou bien l'appelant ne passe pas un pointeur nul.
    unsafe { strcmp(a, b) == 0 }
}
