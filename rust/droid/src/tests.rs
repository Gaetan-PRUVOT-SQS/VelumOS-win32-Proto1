use core::ffi::{CStr, c_char, c_int, c_void};
use core::ptr;
use std::sync::{LazyLock, Mutex, MutexGuard};

use crate::abi::{
    DROID_KIDS_MAX, DROID_LOG_DEBUG, DROID_LOG_ERROR, DROID_LOG_INFO, DROID_LOG_WARN,
    DROID_VIEWS_MAX, DV_BUTTON, DV_LINEAR, DV_TEXT, E_INVAL, E_NOMEM, E_TIMEOUT, t_darrview,
    t_dbuiltin, t_dclass, t_dname, t_dnative, t_dnativefn, t_droid, t_drsb, t_dstr16, t_dvm,
};
use crate::util::{ACT, CSQ, OBJ, OCL, STR, THR, TV};
use crate::{
    dr_act_finish, dr_classes, dr_dec16, dr_dotted, dr_fail, dr_ll_orient, dr_math_abs, dr_missing,
    dr_release, dr_sb_append_c, dr_sb_append_i, dr_sb_append_j, dr_sb_append_obj, dr_sb_append_z,
    dr_sb_length, dr_sys_arraycopy, dr_tab_android, dr_tab_lang, dr_tv_gettext, dr_tv_settext,
    dr_utf8, dr_vg_addview, dr_view_init, dr_view_listen, droid_click, droid_install, droid_start,
    droid_stop, droid_view,
};

const E_OK: c_int = 0;
const E_NOENT: c_int = -2;
const THROWN: c_int = 1;
const ACC_NO_NEW: u32 = 0x600;
const HARNESS_THROWN: i64 = -99_999;

static GATE: Mutex<()> = Mutex::new(());
static WORLD: LazyLock<Mutex<World>> = LazyLock::new(|| Mutex::new(World::blank()));

struct Meth {
    name: *const c_char,
    sig: *const c_char,
    func: Option<t_dnativefn>,
}

struct Klass {
    desc: *const c_char,
    parent: *mut Klass,
    iface: *mut Klass,
    access: u32,
    payload: u32,
    is_array: bool,
    is_ref: u8,
    width: u8,
    #[allow(clippy::vec_box)]
    methods: Vec<Box<Meth>>,
}

enum Store {
    None,
    Id(u32),
    Sb(Box<t_drsb>),
    Str(Vec<u16>),
    Arr(Arr),
}

struct Arr {
    raw: Vec<u64>,
    len: u32,
    width: u8,
    is_ref: u8,
}

struct Obj {
    class: *mut Klass,
    pins: u32,
    message: u32,
    store: Store,
}

struct World {
    #[allow(clippy::vec_box)]
    classes: Vec<Box<Klass>>,
    #[allow(clippy::vec_box)]
    objs: Vec<Box<Obj>>,
    seeded: bool,
    fail_pin: bool,
    batches: Vec<u32>,
    logs: u32,
    level: u32,
    line: String,
}

fn with_world<T>(f: impl FnOnce(&mut World) -> T) -> T {
    let mut g = WORLD.lock().unwrap_or_else(|e| e.into_inner());
    f(&mut g)
}

fn eq(a: *const c_char, b: *const c_char) -> bool {
    if a.is_null() || b.is_null() {
        return a == b;
    }
    // SAFETY: a et b sont des chaînes C non nulles.
    unsafe { CStr::from_ptr(a) == CStr::from_ptr(b) }
}

fn cstr(p: *const c_char) -> String {
    if p.is_null() {
        return String::new();
    }
    // SAFETY: p est une chaîne C.
    unsafe { CStr::from_ptr(p).to_string_lossy().into_owned() }
}

fn decode_utf8(bytes: &[u8]) -> Vec<u16> {
    let mut o = Vec::new();
    let mut i = 0usize;
    while i < bytes.len() {
        let b = bytes[i];
        if b < 0x80 {
            o.push(u16::from(b));
            i += 1;
        } else if b & 0xe0 == 0xc0 && i + 1 < bytes.len() {
            let cp = (u32::from(b & 0x1f) << 6) | u32::from(bytes[i + 1] & 0x3f);
            o.push(cp as u16);
            i += 2;
        } else if b & 0xf0 == 0xe0 && i + 2 < bytes.len() {
            let cp = (u32::from(b & 0x0f) << 12)
                | (u32::from(bytes[i + 1] & 0x3f) << 6)
                | u32::from(bytes[i + 2] & 0x3f);
            o.push(cp as u16);
            i += 3;
        } else if b & 0xf8 == 0xf0 && i + 3 < bytes.len() {
            let cp = (u32::from(b & 0x07) << 18)
                | (u32::from(bytes[i + 1] & 0x3f) << 12)
                | (u32::from(bytes[i + 2] & 0x3f) << 6)
                | u32::from(bytes[i + 3] & 0x3f);
            let u = cp - 0x1_0000;
            o.push(0xd800 + (u >> 10) as u16);
            o.push(0xdc00 + (u & 0x3ff) as u16);
            i += 4;
        } else {
            o.push(0xfffd);
            i += 1;
        }
    }
    o
}

fn assignable(mut from: *mut Klass, to: *mut Klass) -> bool {
    if to.is_null() {
        return false;
    }
    let mut guard = 0u32;
    while !from.is_null() && guard < 64 {
        if from == to {
            return true;
        }
        // SAFETY: from vient du tableau des classes, encore alloué.
        let iface = unsafe { (*from).iface };
        if !iface.is_null() && iface == to {
            return true;
        }
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        from = unsafe { (*from).parent };
        guard += 1;
    }
    false
}

impl World {
    fn blank() -> Self {
        Self {
            classes: Vec::new(),
            objs: Vec::new(),
            seeded: false,
            fail_pin: false,
            batches: Vec::new(),
            logs: 0,
            level: 0,
            line: String::new(),
        }
    }

    fn clear(&mut self) {
        *self = Self::blank();
    }

    fn find(&self, desc: *const c_char) -> *mut Klass {
        for c in &self.classes {
            if eq(c.desc, desc) {
                return &raw const **c as *mut Klass;
            }
        }
        ptr::null_mut()
    }

    #[allow(clippy::too_many_arguments)]
    fn define(
        &mut self,
        desc: *const c_char,
        parent: *mut Klass,
        iface: *mut Klass,
        access: u32,
        payload: u32,
        is_array: bool,
        is_ref: u8,
        width: u8,
    ) -> *mut Klass {
        self.classes.push(Box::new(Klass {
            desc,
            parent,
            iface,
            access,
            payload,
            is_array,
            is_ref,
            width,
            methods: Vec::new(),
        }));
        &raw mut **self.classes.last_mut().unwrap()
    }

    fn ensure_seed(&mut self) {
        if self.seeded {
            return;
        }
        self.seeded = true;
        let obj = self.define(
            OBJ.as_ptr(),
            ptr::null_mut(),
            ptr::null_mut(),
            1,
            0,
            false,
            0,
            0,
        );
        self.define(
            c"Ljava/lang/Class;".as_ptr(),
            obj,
            ptr::null_mut(),
            1,
            0,
            false,
            0,
            0,
        );
        let csq = self.define(
            CSQ.as_ptr(),
            ptr::null_mut(),
            ptr::null_mut(),
            0x601,
            0,
            false,
            0,
            0,
        );
        self.define(STR.as_ptr(), obj, csq, 0x11, 0, false, 0, 0);
        self.define(THR.as_ptr(), obj, ptr::null_mut(), 1, 0, false, 0, 0);
    }

    fn ensure_array(&mut self, desc: *const c_char) -> Result<*mut Klass, c_int> {
        let found = self.find(desc);
        if !found.is_null() {
            return Ok(found);
        }
        // SAFETY: desc est un descripteur de tableau, chaîne C d'au moins deux octets.
        let b0 = unsafe { desc.add(1).read() } as u8;
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        let b1 = unsafe { desc.add(2).read() } as u8;
        let (width, is_ref) = if b1 == 0 && matches!(b0, b'Z' | b'B') {
            (1u8, 0u8)
        } else if b1 == 0 && matches!(b0, b'C' | b'S') {
            (2, 0)
        } else if b1 == 0 && matches!(b0, b'I' | b'F') {
            (4, 0)
        } else if b1 == 0 && matches!(b0, b'J' | b'D') {
            (8, 0)
        } else if b0 == b'L' || b0 == b'[' {
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            let elem = self.find(unsafe { desc.add(1) });
            if elem.is_null() {
                return Err(E_NOENT);
            }
            (4, 1)
        } else {
            return Err(E_INVAL);
        };
        let obj = self.find(OBJ.as_ptr());
        Ok(self.define(desc, obj, ptr::null_mut(), 0x11, 0, true, is_ref, width))
    }

    fn payload_of(&self, class: *mut Klass) -> u32 {
        let mut c = class;
        let mut pay = 0u32;
        let mut guard = 0u32;
        while !c.is_null() && guard < 64 {
            // SAFETY: c est une classe du monde.
            let here = unsafe { (*c).payload };
            if here != 0 {
                pay = here;
            }
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            c = unsafe { (*c).parent };
            guard += 1;
        }
        pay
    }

    fn alloc(&mut self, class: *mut Klass, store: Store) -> u32 {
        self.objs.push(Box::new(Obj {
            class,
            pins: 0,
            message: 0,
            store,
        }));
        self.objs.len() as u32
    }

    fn obj(&mut self, r: u32) -> Option<&mut Obj> {
        if r == 0 {
            return None;
        }
        self.objs.get_mut((r - 1) as usize).map(|b| b.as_mut())
    }

    fn store_for(&self, class: *mut Klass) -> Store {
        match self.payload_of(class) {
            0 => Store::None,
            1..=8 => Store::Id(0),
            _ => Store::Sb(Box::default()),
        }
    }

    fn intern_units(&mut self, units: Vec<u16>) -> u32 {
        let cls = self.find(STR.as_ptr());
        self.alloc(cls, Store::Str(units))
    }

    fn intern_cstr(&mut self, msg: *const c_char) -> u32 {
        // SAFETY: msg est une chaîne C.
        let bytes = unsafe { CStr::from_ptr(msg) }.to_bytes();
        let units = decode_utf8(bytes);
        self.intern_units(units)
    }

    fn make_arr(&mut self, desc: *const c_char, len: u32, fill: bool) -> u32 {
        self.ensure_seed();
        let cls = self.ensure_array(desc).expect("classe tableau");
        // SAFETY: cls vient d'être créé.
        let (width, is_ref) = unsafe { ((*cls).width, (*cls).is_ref) };
        let bytes = (len as usize).saturating_mul(width as usize);
        let words = bytes.div_ceil(8);
        let mut raw = vec![0u64; words];
        if fill && width == 4 {
            let mut i = 0u32;
            while i < len {
                // SAFETY: i < len et raw couvre len éléments de 4 octets.
                unsafe {
                    raw.as_mut_ptr()
                        .cast::<i32>()
                        .add(i as usize)
                        .write(i as i32 + 1)
                };
                i += 1;
            }
        }
        self.alloc(
            cls,
            Store::Arr(Arr {
                raw,
                len,
                width,
                is_ref,
            }),
        )
    }
}

fn add_method(c: *mut Klass, name: *const c_char, sig: *const c_char, func: Option<t_dnativefn>) {
    // SAFETY: c est une classe du monde, methods lui appartient.
    unsafe {
        (*c).methods.push(Box::new(Meth { name, sig, func }));
    }
}

// SAFETY: les pointeurs du faux monde ne sortent pas du mutex WORLD.
unsafe impl Send for Meth {}
// SAFETY: les pointeurs du faux monde ne sortent pas du mutex WORLD.
unsafe impl Send for Klass {}
// SAFETY: les pointeurs du faux monde ne sortent pas du mutex WORLD.
unsafe impl Send for Obj {}

struct Host {
    vm: Box<t_dvm>,
    d: Box<t_droid>,
    _gate: MutexGuard<'static, ()>,
}

fn gate() -> MutexGuard<'static, ()> {
    let g = GATE.lock().unwrap_or_else(|e| e.into_inner());
    with_world(World::clear);
    g
}

fn open() -> Host {
    let g = gate();
    let mut vm = Box::new(t_dvm::default());
    vm.lim.heap_bytes = 16 * 1024 * 1024;
    let mut d = Box::new(t_droid::default());
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { droid_install(d.as_mut(), vm.as_mut()) };
    assert_eq!(rc, E_OK);
    Host { vm, d, _gate: g }
}

fn harness(rc: c_int, ret: u64) -> i64 {
    if rc != 0 {
        return -100_000 + i64::from(rc);
    }
    i64::from(ret as u32 as i32)
}

fn call(vm: *mut t_dvm, cls: &CStr, name: &CStr, sig: &CStr, args: &[u32]) -> (c_int, u64) {
    let mut c = ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_class(vm, cls.as_ptr(), &mut c) }, E_OK);
    let nm = t_dname {
        name: name.as_ptr(),
        sig: sig.as_ptr(),
    };
    let mut m = ptr::null();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_method(vm, c, &nm, &mut m) }, E_OK);
    let mut ret = 0u64;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dvm_call(vm, m, args.as_ptr(), &mut ret) };
    (rc, ret)
}

fn jstr(vm: *mut t_dvm, text: &str) -> u32 {
    let c = std::ffi::CString::new(text).unwrap();
    let mut r = 0u32;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_string_utf8_new(vm, c.as_ptr(), &mut r) }, E_OK);
    r
}

fn jstr16(vm: *mut t_dvm, units: &[u16]) -> u32 {
    let s = t_dstr16 {
        p: units.as_ptr(),
        n: units.len() as u32,
    };
    let mut r = 0u32;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_string_utf16_new(vm, s, &mut r) }, E_OK);
    r
}

fn text_of(vm: *mut t_dvm, r: u32) -> String {
    let mut s = t_dstr16 {
        p: ptr::null(),
        n: 0,
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_string_get(vm, r, &mut s) }, E_OK);
    if s.n == 0 {
        return String::new();
    }
    // SAFETY: s.p couvre s.n unités tant que l'objet vit.
    let slice = unsafe { core::slice::from_raw_parts(s.p, s.n as usize) };
    String::from_utf16_lossy(slice)
}

fn pending_desc(vm: *mut t_dvm) -> String {
    // SAFETY: vm n'est pas nul.
    let p = unsafe { (*vm).pending };
    if p == 0 {
        return String::new();
    }
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    cstr(unsafe { dvm_class_name(dvm_class_of(vm, p)) })
}

fn err_of(d: &t_droid) -> String {
    cstr(d.error.as_ptr().cast())
}

fn new_obj(vm: *mut t_dvm, desc: &CStr) -> u32 {
    let mut c = ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_class(vm, desc.as_ptr(), &mut c) }, E_OK);
    let mut r = 0u32;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_new(vm, c, &mut r) }, E_OK);
    r
}

fn init_view(vm: *mut t_dvm, desc: &CStr) -> u32 {
    let r = new_obj(vm, desc);
    let args = [r];
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_view_init(vm, args.as_ptr(), ptr::null_mut()) },
        E_OK
    );
    r
}

fn subclass(
    desc: &'static CStr,
    parent: &CStr,
    iface: Option<&CStr>,
    methods: &[(&'static CStr, &'static CStr, t_dnativefn)],
) {
    with_world(|w| {
        let p = w.find(parent.as_ptr());
        assert!(!p.is_null());
        let face = iface.map(|s| w.find(s.as_ptr())).unwrap_or(ptr::null_mut());
        let c = w.define(desc.as_ptr(), p, face, 1, 0, false, 0, 0);
        for (name, sig, func) in methods {
            add_method(c, name.as_ptr(), sig.as_ptr(), Some(*func));
        }
    });
}

fn arr_at(r: u32, i: usize) -> i32 {
    with_world(|w| {
        let obj = w.obj(r).unwrap();
        let Store::Arr(a) = &mut obj.store else {
            panic!("pas un tableau");
        };
        // SAFETY: i est dans le tableau d'entiers du test.
        unsafe { a.raw.as_ptr().cast::<i32>().add(i).read() }
    })
}

fn arr_u32(r: u32, i: usize) -> u32 {
    with_world(|w| {
        let obj = w.obj(r).unwrap();
        let Store::Arr(a) = &obj.store else {
            panic!("pas un tableau");
        };
        // SAFETY: i est dans le tableau de références du test.
        unsafe { a.raw.as_ptr().cast::<u32>().add(i).read() }
    })
}

fn set_u32(r: u32, i: usize, v: u32) {
    with_world(|w| {
        let obj = w.obj(r).unwrap();
        let Store::Arr(a) = &mut obj.store else {
            panic!("pas un tableau");
        };
        // SAFETY: i est dans le tableau de références du test.
        unsafe { a.raw.as_mut_ptr().cast::<u32>().add(i).write(v) };
    });
}

unsafe extern "C" fn test_log(
    user: *mut c_void,
    level: u32,
    tag: *const c_char,
    msg: *const c_char,
) {
    let _ = user;
    let line = format!("{}: {}", cstr(tag), cstr(msg));
    with_world(|w| {
        w.logs += 1;
        w.level = level;
        w.line = line;
    });
}

unsafe extern "C" fn test_clock(user: *mut c_void) -> u64 {
    // SAFETY: user pointe un u64 vivant, posé par le test.
    unsafe { user.cast::<u64>().read() }
}

unsafe extern "C" fn boom(vm: *mut t_dvm, _args: *const u32, _ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe {
        dvm_throw(
            vm,
            c"Ljava/lang/IllegalStateException;".as_ptr(),
            c"panne".as_ptr(),
        )
    }
}

unsafe extern "C" fn click_boom(vm: *mut t_dvm, _args: *const u32, _ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe {
        dvm_throw(
            vm,
            c"Ljava/lang/ArithmeticException;".as_ptr(),
            c"division".as_ptr(),
        )
    }
}

unsafe extern "C" fn on_destroy(vm: *mut t_dvm, args: *const u32, ret: *mut u64) -> c_int {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_act_finish(vm, args, ret) }
}

unsafe extern "C" fn crowd(vm: *mut t_dvm, _args: *const u32, ret: *mut u64) -> c_int {
    let mut i = 0u32;
    while i < 65 {
        let r = new_obj(vm, TV);
        let args = [r];
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        let rc = unsafe { dr_view_init(vm, args.as_ptr(), ret) };
        if rc != 0 {
            return rc;
        }
        i += 1;
    }
    0
}

unsafe extern "C" fn family(vm: *mut t_dvm, _args: *const u32, ret: *mut u64) -> c_int {
    let parent = init_view(vm, c"Landroid/widget/LinearLayout;");
    let mut kids = [0u32; 17];
    let mut i = 0usize;
    while i < kids.len() {
        kids[i] = init_view(vm, TV);
        i += 1;
    }
    i = 0;
    while i < kids.len() {
        let args = [parent, kids[i]];
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        let rc = unsafe { dr_vg_addview(vm, args.as_ptr(), ret) };
        if rc != 0 {
            return rc;
        }
        i += 1;
    }
    0
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_builtins(vm: *mut t_dvm, tab: *const t_dbuiltin, n: u32) -> c_int {
    if vm.is_null() || (tab.is_null() && n > 0) {
        return E_INVAL;
    }
    with_world(|w| {
        w.ensure_seed();
        let mut i = 0u32;
        while i < n {
            // SAFETY: tab pointe n entrées, vivantes le temps de l'appel.
            let b = unsafe { tab.add(i as usize).read() };
            if w.find(b.desc).is_null() {
                let parent = if b.super_desc.is_null() {
                    ptr::null_mut()
                } else {
                    w.find(b.super_desc)
                };
                if !b.super_desc.is_null() && parent.is_null() {
                    return E_NOENT;
                }
                let iface = if b.iface_desc.is_null() {
                    ptr::null_mut()
                } else {
                    w.find(b.iface_desc)
                };
                if !b.iface_desc.is_null() && iface.is_null() {
                    return E_NOENT;
                }
                w.define(
                    b.desc,
                    parent,
                    iface,
                    b.access,
                    b.payload_bytes,
                    false,
                    0,
                    0,
                );
            }
            i += 1;
        }
        E_OK
    })
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_natives(vm: *mut t_dvm, tab: *const t_dnative, n: u32) -> c_int {
    if vm.is_null() || (tab.is_null() && n > 0) {
        return E_INVAL;
    }
    with_world(|w| {
        w.batches.push(n);
        let mut i = 0u32;
        while i < n {
            // SAFETY: tab pointe n natives. On copie avant que l'appelant libère le tableau.
            let nat = unsafe { tab.add(i as usize).read() };
            let c = w.find(nat.cls);
            if c.is_null() {
                return E_NOENT;
            }
            add_method(c, nat.name, nat.sig, nat.r#fn);
            i += 1;
        }
        E_OK
    })
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_class(
    vm: *mut t_dvm,
    desc: *const c_char,
    out: *mut *mut t_dclass,
) -> c_int {
    if vm.is_null() || desc.is_null() || out.is_null() {
        return E_INVAL;
    }
    let (rc, c) = with_world(|w| {
        w.ensure_seed();
        let found = w.find(desc);
        if !found.is_null() {
            return (E_OK, found);
        }
        // SAFETY: desc est une chaîne C.
        if unsafe { desc.read() } == b'[' as c_char {
            return match w.ensure_array(desc) {
                Ok(k) => (E_OK, k),
                Err(e) => (e, ptr::null_mut()),
            };
        }
        (E_NOENT, ptr::null_mut())
    });
    if rc == E_OK {
        // SAFETY: out n'est pas nul.
        unsafe { out.write(c.cast()) };
    }
    rc
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_method(
    vm: *mut t_dvm,
    c: *mut t_dclass,
    name: *const t_dname,
    out: *mut *const crate::abi::t_dmethod,
) -> c_int {
    if vm.is_null() || c.is_null() || name.is_null() || out.is_null() {
        return E_INVAL;
    }
    // SAFETY: name n'est pas nul, name et sig sont des chaînes C.
    let (nm, sig) = unsafe { ((*name).name, (*name).sig) };
    let found = with_world(|_w| {
        let mut k = c.cast::<Klass>();
        let mut guard = 0u32;
        while !k.is_null() && guard < 64 {
            // SAFETY: k est une classe du monde.
            let methods = unsafe { &(*k).methods };
            for m in methods {
                if eq(m.name, nm) && eq(m.sig, sig) {
                    return &raw const **m;
                }
            }
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            k = unsafe { (*k).parent };
            guard += 1;
        }
        ptr::null()
    });
    if found.is_null() {
        return E_NOENT;
    }
    // SAFETY: out n'est pas nul.
    unsafe { out.write(found.cast()) };
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_new(vm: *mut t_dvm, c: *mut t_dclass, out: *mut u32) -> c_int {
    if vm.is_null() || c.is_null() || out.is_null() {
        return E_INVAL;
    }
    let rc = with_world(|w| {
        let k = c.cast::<Klass>();
        // SAFETY: k est une classe du monde.
        let (access, array, desc) = unsafe { ((*k).access, (*k).is_array, (*k).desc) };
        if array || access & ACC_NO_NEW != 0 || eq(desc, STR.as_ptr()) {
            return Err(E_INVAL);
        }
        let store = w.store_for(k);
        let r = w.alloc(k, store);
        Ok(r)
    });
    match rc {
        Ok(r) => {
            // SAFETY: out n'est pas nul.
            unsafe { out.write(r) };
            E_OK
        }
        Err(e) => e,
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_class_of(vm: *mut t_dvm, r: u32) -> *mut t_dclass {
    if vm.is_null() {
        return ptr::null_mut();
    }
    with_world(|w| w.obj(r).map(|o| o.class.cast()).unwrap_or(ptr::null_mut()))
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_class_name(c: *const t_dclass) -> *const c_char {
    if c.is_null() {
        return ptr::null();
    }
    // SAFETY: c est une classe du monde, ou nul (déjà filtré).
    unsafe { (*c.cast::<Klass>()).desc }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_is_instance(vm: *mut t_dvm, r: u32, c: *const t_dclass) -> c_int {
    if vm.is_null() || c.is_null() {
        return 0;
    }
    let hit = with_world(|w| {
        let Some(o) = w.obj(r) else {
            return false;
        };
        assignable(o.class, c.cast_mut().cast())
    });
    c_int::from(hit)
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_payload(vm: *mut t_dvm, r: u32, builtin: *const t_dclass) -> *mut c_void {
    if vm.is_null() || builtin.is_null() {
        return ptr::null_mut();
    }
    with_world(|w| {
        let k = builtin.cast::<Klass>().cast_mut();
        // SAFETY: builtin est une classe du monde.
        if unsafe { (*k).payload } == 0 {
            return ptr::null_mut();
        }
        let Some(o) = w.obj(r) else {
            return ptr::null_mut();
        };
        if !assignable(o.class, k) {
            return ptr::null_mut();
        }
        match &mut o.store {
            Store::Id(id) => id as *mut u32 as *mut c_void,
            Store::Sb(sb) => sb.as_mut() as *mut t_drsb as *mut c_void,
            _ => ptr::null_mut(),
        }
    })
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_string_utf8_new(
    vm: *mut t_dvm,
    utf8: *const c_char,
    out: *mut u32,
) -> c_int {
    if vm.is_null() || utf8.is_null() || out.is_null() {
        return E_INVAL;
    }
    let r = with_world(|w| {
        w.ensure_seed();
        w.intern_cstr(utf8)
    });
    // SAFETY: out n'est pas nul.
    unsafe { out.write(r) };
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_string_utf16_new(vm: *mut t_dvm, s: t_dstr16, out: *mut u32) -> c_int {
    if vm.is_null() || out.is_null() || (s.p.is_null() && s.n > 0) {
        return E_INVAL;
    }
    let units = if s.n == 0 {
        Vec::new()
    } else {
        // SAFETY: s.p couvre s.n unités, le temps de la copie.
        unsafe { core::slice::from_raw_parts(s.p, s.n as usize) }.to_vec()
    };
    let r = with_world(|w| {
        w.ensure_seed();
        w.intern_units(units)
    });
    // SAFETY: out n'est pas nul.
    unsafe { out.write(r) };
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_string_get(vm: *mut t_dvm, r: u32, out: *mut t_dstr16) -> c_int {
    if vm.is_null() || out.is_null() {
        return E_INVAL;
    }
    let got = with_world(|w| {
        let o = w.obj(r)?;
        let Store::Str(units) = &o.store else {
            return None;
        };
        Some(t_dstr16 {
            p: units.as_ptr(),
            n: units.len() as u32,
        })
    });
    let Some(s) = got else {
        return E_INVAL;
    };
    // SAFETY: out n'est pas nul.
    unsafe { out.write(s) };
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_array_view(vm: *mut t_dvm, r: u32, out: *mut t_darrview) -> c_int {
    if vm.is_null() || out.is_null() {
        return E_INVAL;
    }
    let got = with_world(|w| {
        let o = w.obj(r)?;
        let Store::Arr(a) = &mut o.store else {
            return None;
        };
        Some(t_darrview {
            data: a.raw.as_mut_ptr().cast(),
            len: a.len,
            width: a.width,
            is_ref: a.is_ref,
        })
    });
    let Some(v) = got else {
        return E_INVAL;
    };
    // SAFETY: out n'est pas nul.
    unsafe { out.write(v) };
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_throw(vm: *mut t_dvm, desc: *const c_char, msg: *const c_char) -> c_int {
    if vm.is_null() || desc.is_null() {
        return E_INVAL;
    }
    let r = with_world(|w| {
        w.ensure_seed();
        let cls = w.find(desc);
        if cls.is_null() {
            return 0;
        }
        let message = if msg.is_null() { 0 } else { w.intern_cstr(msg) };
        let obj = w.alloc(cls, Store::None);
        if let Some(o) = w.obj(obj) {
            o.message = message;
        }
        obj
    });
    if r == 0 {
        return E_NOENT;
    }
    // SAFETY: vm n'est pas nul.
    unsafe { (*vm).pending = r };
    THROWN
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_pin(vm: *mut t_dvm, r: u32) -> c_int {
    if vm.is_null() {
        return E_INVAL;
    }
    with_world(|w| {
        if w.fail_pin {
            return E_INVAL;
        }
        let Some(o) = w.obj(r) else {
            return E_INVAL;
        };
        o.pins += 1;
        E_OK
    })
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_unpin(vm: *mut t_dvm, r: u32) {
    if vm.is_null() {
        return;
    }
    with_world(|w| {
        if let Some(o) = w.obj(r)
            && o.pins > 0
        {
            o.pins -= 1;
        }
    });
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_call(
    vm: *mut t_dvm,
    m: *const crate::abi::t_dmethod,
    args: *const u32,
    ret: *mut u64,
) -> c_int {
    if vm.is_null() || m.is_null() {
        return E_INVAL;
    }
    // SAFETY: m est un Meth publié par dvm_method.
    let func = unsafe { (*m.cast::<Meth>()).func };
    let Some(func) = func else {
        return E_INVAL;
    };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { func(vm, args, ret) }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_throwable_message(vm: *mut t_dvm, exc: u32, msg: *mut u32) -> c_int {
    if vm.is_null() || msg.is_null() {
        return E_INVAL;
    }
    let got = with_world(|w| w.obj(exc).map(|o| o.message));
    let Some(v) = got else {
        return E_INVAL;
    };
    // SAFETY: msg n'est pas nul.
    unsafe { msg.write(v) };
    E_OK
}

#[unsafe(no_mangle)]
unsafe extern "C" fn dvm_throwable_set_message(vm: *mut t_dvm, exc: u32, msg: u32) -> c_int {
    if vm.is_null() {
        return E_INVAL;
    }
    let ok = with_world(|w| {
        let Some(o) = w.obj(exc) else {
            return false;
        };
        o.message = msg;
        true
    });
    if ok { E_OK } else { E_INVAL }
}

#[test]
fn tables_identiques_au_c() {
    let mut cls = ptr::null();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_classes(&mut cls) }, 35);
    // SAFETY: dr_classes a écrit un tableau de 35 classes statiques.
    let sb = unsafe { cls.add(21).read() };
    assert_eq!(cstr(sb.desc), "Ljava/lang/StringBuilder;");
    assert_eq!(sb.access, 0x11);
    assert_eq!(sb.payload_bytes, core::mem::size_of::<t_drsb>() as u32);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        cstr(unsafe { cls.add(30).read() }.desc),
        "Landroid/view/View;"
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { cls.add(30).read() }.payload_bytes, 8);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        cstr(unsafe { cls.add(29).read() }.desc),
        "Landroid/view/View$OnClickListener;"
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { cls.add(29).read() }.access, 0x601);
    let mut lang = ptr::null();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_tab_lang(&mut lang) }, 31);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let abs = unsafe { lang.add(13).read() }.r#fn;
    assert!(abs.is_some_and(|f| ptr::fn_addr_eq(f, dr_math_abs as t_dnativefn)));
    let mut and = ptr::null();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_tab_android(&mut and) }, 26);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let init16 = unsafe { and.add(16).read() }.r#fn;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let init20 = unsafe { and.add(20).read() }.r#fn;
    assert!(init16.is_some_and(|f| ptr::fn_addr_eq(f, dr_view_init as t_dnativefn)));
    assert!(init20.is_some_and(|f| ptr::fn_addr_eq(f, dr_view_init as t_dnativefn)));
}

#[test]
fn install_compte_les_natives_et_garde_photocopies() {
    let h = open();
    let batches = with_world(|w| w.batches.clone());
    assert_eq!(batches, vec![42, 31, 26]);
    assert!(ptr::eq(
        h.vm.host,
        h.d.as_ref() as *const t_droid as *mut c_void
    ));
    drop(h);
    let g = gate();
    let mut vm = Box::new(t_dvm::default());
    let mut d = Box::new(t_droid::default());
    let mut now = 9u64;
    d.clock = Some(test_clock);
    d.log = Some(test_log);
    d.user = (&raw mut now).cast();
    d.finished = 7;
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_install(ptr::null_mut(), vm.as_mut()) },
        E_INVAL
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_install(d.as_mut(), ptr::null_mut()) },
        E_INVAL
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_install(d.as_mut(), vm.as_mut()) }, E_OK);
    assert_eq!(d.finished, 0);
    assert!(d.clock.is_some_and(|f| {
        ptr::fn_addr_eq(f, test_clock as unsafe extern "C" fn(*mut c_void) -> u64)
    }));
    assert!(d.log.is_some_and(|f| {
        ptr::fn_addr_eq(
            f,
            test_log as unsafe extern "C" fn(*mut c_void, u32, *const c_char, *const c_char),
        )
    }));
    assert!(ptr::eq(d.user, (&raw mut now).cast()));
    drop(g);
}

#[test]
fn math_parse_chaines_et_hash() {
    let mut h = open();
    let vm = h.vm.as_ref() as *const t_dvm as *mut t_dvm;
    let (rc, ret) = call(vm, c"Ljava/lang/Math;", c"abs", c"(I)I", &[(-5i32) as u32]);
    assert_eq!(harness(rc, ret), 5);
    let (rc, ret) = call(vm, c"Ljava/lang/Math;", c"abs", c"(I)I", &[0]);
    assert_eq!(harness(rc, ret), 0);
    let (rc, ret) = call(vm, c"Ljava/lang/Math;", c"abs", c"(I)I", &[i32::MAX as u32]);
    assert_eq!(harness(rc, ret), i64::from(i32::MAX));
    let (rc, ret) = call(vm, c"Ljava/lang/Math;", c"abs", c"(I)I", &[i32::MIN as u32]);
    assert_eq!(ret, 0x8000_0000);
    assert_eq!(harness(rc, ret), i64::from(i32::MIN));
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/Math;",
        c"min",
        c"(II)I",
        &[(-1i32) as u32, 1],
    );
    assert_eq!(harness(rc, ret), -1);
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/Math;",
        c"min",
        c"(II)I",
        &[i32::MAX as u32, i32::MIN as u32],
    );
    assert_eq!(harness(rc, ret), i64::from(i32::MIN));
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/Math;",
        c"max",
        c"(II)I",
        &[i32::MIN as u32, i32::MAX as u32],
    );
    assert_eq!(harness(rc, ret), i64::from(i32::MAX));
    let mut now = 0x0123_4567_89ab_u64;
    h.d.clock = Some(test_clock);
    h.d.user = (&raw mut now).cast();
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/System;",
        c"currentTimeMillis",
        c"()J",
        &[0],
    );
    assert_eq!(rc, 0);
    assert_eq!(ret, now);
    h.d.clock = None;
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/System;",
        c"currentTimeMillis",
        c"()J",
        &[0],
    );
    assert_eq!(rc, 0);
    assert_eq!(ret, 0);

    for (src, want) in [
        ("0", 0i64),
        ("+42", 42),
        ("-7", -7),
        ("2147483647", i64::from(i32::MAX)),
        ("-2147483648", i64::from(i32::MIN)),
    ] {
        let (rc, ret) = call(
            vm,
            c"Ljava/lang/Integer;",
            c"parseInt",
            c"(Ljava/lang/String;)I",
            &[jstr(vm, src)],
        );
        assert_eq!(harness(rc, ret), want, "{src}");
    }
    for src in [
        "2147483648",
        "-2147483649",
        "99999999999999999999",
        "",
        "-",
        "12a",
    ] {
        let (rc, _) = call(
            vm,
            c"Ljava/lang/Integer;",
            c"parseInt",
            c"(Ljava/lang/String;)I",
            &[jstr(vm, src)],
        );
        assert_eq!(harness(rc, 0), HARNESS_THROWN, "{src}");
        assert_eq!(pending_desc(vm), "Ljava/lang/NumberFormatException;");
    }
    let (rc, _) = call(
        vm,
        c"Ljava/lang/Integer;",
        c"parseInt",
        c"(Ljava/lang/String;)I",
        &[0],
    );
    assert_eq!(harness(rc, 0), HARNESS_THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/NumberFormatException;");

    let abc = jstr(vm, "abc");
    let (rc, ret) = call(vm, STR, c"length", c"()I", &[jstr(vm, "")]);
    assert_eq!(harness(rc, ret), 0);
    let (rc, ret) = call(vm, STR, c"length", c"()I", &[jstr(vm, "héé")]);
    assert_eq!(harness(rc, ret), 3);
    let (rc, ret) = call(vm, STR, c"charAt", c"(I)C", &[abc, 0]);
    assert_eq!(harness(rc, ret), i64::from(b'a'));
    let (rc, ret) = call(vm, STR, c"charAt", c"(I)C", &[abc, 2]);
    assert_eq!(harness(rc, ret), i64::from(b'c'));
    let (rc, _) = call(vm, STR, c"charAt", c"(I)C", &[abc, 3]);
    assert_eq!(harness(rc, 0), HARNESS_THROWN);
    assert_eq!(
        pending_desc(vm),
        "Ljava/lang/StringIndexOutOfBoundsException;"
    );
    let (rc, _) = call(vm, STR, c"charAt", c"(I)C", &[abc, i32::MIN as u32]);
    assert_eq!(harness(rc, 0), HARNESS_THROWN);
    let (rc, ret) = call(vm, STR, c"hashCode", c"()I", &[abc]);
    assert_eq!(harness(rc, ret), 96_354);
    let (rc, ret) = call(vm, STR, c"hashCode", c"()I", &[jstr(vm, "")]);
    assert_eq!(harness(rc, ret), 0);
    let units = [0xffffu16; 5];
    let wide = jstr16(vm, &units);
    let (rc, ret) = call(vm, STR, c"hashCode", c"()I", &[wide]);
    assert_eq!(rc, 0);
    assert_eq!(ret, 2_410_836_031);
    let (rc, ret) = call(
        vm,
        STR,
        c"equals",
        c"(Ljava/lang/Object;)Z",
        &[abc, jstr(vm, "abc")],
    );
    assert_eq!(harness(rc, ret), 1);
    let (rc, ret) = call(
        vm,
        STR,
        c"equals",
        c"(Ljava/lang/Object;)Z",
        &[abc, jstr(vm, "abd")],
    );
    assert_eq!(harness(rc, ret), 0);
    let (rc, ret) = call(vm, STR, c"equals", c"(Ljava/lang/Object;)Z", &[abc, 0]);
    assert_eq!(rc, 0);
    assert_eq!(ret, 0);
    let (rc, ret) = call(
        vm,
        STR,
        c"concat",
        c"(Ljava/lang/String;)Ljava/lang/String;",
        &[abc, jstr(vm, "de")],
    );
    assert_eq!(rc, 0);
    assert_eq!(text_of(vm, ret as u32), "abcde");
    let (rc, _) = call(
        vm,
        STR,
        c"concat",
        c"(Ljava/lang/String;)Ljava/lang/String;",
        &[abc, 0],
    );
    assert_eq!(harness(rc, 0), HARNESS_THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/NullPointerException;");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*vm).lim.heap_bytes = 4 };
    let (rc, _) = call(
        vm,
        STR,
        c"concat",
        c"(Ljava/lang/String;)Ljava/lang/String;",
        &[jstr(vm, "a"), jstr(vm, "bb")],
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/OutOfMemoryError;");
    assert_eq!(rc, THROWN);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*vm).pending = 0 };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*vm).lim.heap_bytes = 0 };
    let (rc, ret) = call(
        vm,
        STR,
        c"concat",
        c"(Ljava/lang/String;)Ljava/lang/String;",
        &[jstr(vm, ""), jstr(vm, "")],
    );
    assert_eq!(rc, 0);
    assert_eq!(text_of(vm, ret as u32), "");
}

#[test]
fn objet_tostring_et_message() {
    let mut h = open();
    let vm = h.vm.as_mut() as *mut t_dvm;
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/Integer;",
        c"toString",
        c"(I)Ljava/lang/String;",
        &[0],
    );
    assert_eq!(rc, 0);
    assert_eq!(text_of(vm, ret as u32), "0");
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/Integer;",
        c"toString",
        c"(I)Ljava/lang/String;",
        &[i32::MIN as u32],
    );
    assert_eq!(text_of(vm, ret as u32), "-2147483648");
    assert_eq!(rc, 0);
    let (rc, ret) = call(
        vm,
        STR,
        c"valueOf",
        c"(I)Ljava/lang/String;",
        &[i32::MAX as u32],
    );
    assert_eq!(rc, 0);
    assert_eq!(text_of(vm, ret as u32), "2147483647");
    let o = new_obj(vm, OBJ);
    let (rc, ret) = call(vm, OBJ, c"equals", c"(Ljava/lang/Object;)Z", &[o, o]);
    assert_eq!(harness(rc, ret), 1);
    let (rc, ret) = call(
        vm,
        OBJ,
        c"equals",
        c"(Ljava/lang/Object;)Z",
        &[o, jstr(vm, "x")],
    );
    assert_eq!(harness(rc, ret), 0);
    let (rc, ret) = call(vm, OBJ, c"hashCode", c"()I", &[o]);
    assert_eq!(ret, u64::from(o));
    assert_eq!(rc, 0);
    let (rc, ret) = call(vm, OBJ, c"toString", c"()Ljava/lang/String;", &[o]);
    assert_eq!(rc, 0);
    assert!(text_of(vm, ret as u32).contains("java.lang.Object@"));
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe {
            dvm_throw(
                vm,
                c"Ljava/lang/IllegalStateException;".as_ptr(),
                c"motif".as_ptr(),
            )
        },
        THROWN
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let exc = unsafe { (*vm).pending };
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { (*vm).pending = 0 };
    let (rc, ret) = call(vm, THR, c"getMessage", c"()Ljava/lang/String;", &[exc]);
    assert_eq!(rc, 0);
    assert_eq!(text_of(vm, ret as u32), "motif");
}

#[test]
fn utf8_tronque_sans_couper_et_dec16() {
    let mut out = [0u8; 8];
    let pair = [0xd83du16, 0xde00];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe {
        dr_utf8(
            t_dstr16 {
                p: pair.as_ptr(),
                n: 2,
            },
            out.as_mut_ptr().cast(),
            out.len(),
        )
    };
    assert_eq!(n, 4);
    assert_eq!(&out[..4], &[0xf0, 0x9f, 0x98, 0x80]);
    let lone = [0xd800u16];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe {
        dr_utf8(
            t_dstr16 {
                p: lone.as_ptr(),
                n: 1,
            },
            out.as_mut_ptr().cast(),
            out.len(),
        )
    };
    assert_eq!(&out[..n], &[0xef, 0xbf, 0xbd]);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe {
            dr_utf8(
                t_dstr16 {
                    p: lone.as_ptr(),
                    n: 1,
                },
                out.as_mut_ptr().cast(),
                0,
            )
        },
        0
    );
    let mut buf = [0u16; 24];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_dec16(i64::from(i32::MIN), buf.as_mut_ptr()) };
    assert_eq!(String::from_utf16_lossy(&buf[..n as usize]), "-2147483648");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { dr_dec16(i64::MIN, buf.as_mut_ptr()) };
    assert_eq!(
        String::from_utf16_lossy(&buf[..n as usize]),
        "-9223372036854775808"
    );
    let mut dotted = [0u8; 64];
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe {
        dr_dotted(
            c"Ljava/lang/String;".as_ptr(),
            dotted.as_mut_ptr().cast(),
            dotted.len(),
        )
    };
    assert_eq!(cstr(dotted.as_ptr().cast()), "java.lang.String");
    assert_eq!(n, "java.lang.String".len());
}

#[test]
fn constructeur_de_chaines_et_tampon_plein() {
    let mut h = open();
    let vm = h.vm.as_mut() as *mut t_dvm;
    let sb = new_obj(vm, c"Ljava/lang/StringBuilder;");
    let mut args = [sb, jstr(vm, "Vé"), 0];
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/StringBuilder;",
        c"append",
        c"(Ljava/lang/String;)Ljava/lang/StringBuilder;",
        &args,
    );
    assert_eq!(harness(rc, ret), i64::from(sb));
    args[1] = i32::MIN as u32;
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/StringBuilder;",
        c"append",
        c"(I)Ljava/lang/StringBuilder;",
        &args,
    );
    assert_eq!(harness(rc, ret), i64::from(sb));
    args[1] = 0;
    args[2] = i32::MIN as u32;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_sb_append_j(vm, args.as_ptr(), &mut 0) };
    assert_eq!(rc, 0);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_sb_append_c(vm, [sb, u32::from(b'x')].as_ptr(), &mut 0) },
        0
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_sb_append_z(vm, [sb, 1].as_ptr(), &mut 0) }, 0);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_sb_append_z(vm, [sb, 0].as_ptr(), &mut 0) }, 0);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_sb_append_obj(vm, [sb, 0].as_ptr(), &mut 0) },
        0
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_sb_append_obj(vm, [sb, 0].as_ptr(), &mut 0) },
        0
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_sb_append_obj(vm, [sb, sb].as_ptr(), &mut 0) },
        0
    );
    let (rc, ret) = call(vm, c"Ljava/lang/StringBuilder;", c"length", c"()I", &[sb]);
    assert_eq!(harness(rc, ret), 102);
    let (rc, ret) = call(
        vm,
        c"Ljava/lang/StringBuilder;",
        c"toString",
        c"()Ljava/lang/String;",
        &[sb],
    );
    assert_eq!(rc, 0);
    assert_eq!(
        text_of(vm, ret as u32),
        "Vé-2147483648-9223372036854775808xtruefalsenullnullVé-2147483648-9223372036854775808xtruefalsenullnull"
    );

    let sb = new_obj(vm, c"Ljava/lang/StringBuilder;");
    let dix = jstr(vm, "0123456789");
    let mut i = 0u32;
    while i < 102 {
        assert_eq!(
            // SAFETY: l'appelant garantit les pointeurs de ce bloc.
            unsafe { dr_sb_append_obj(vm, [sb, dix].as_ptr(), &mut 0) },
            0
        );
        i += 1;
    }
    let mut ret = 0u64;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_sb_length(vm, [sb].as_ptr(), &mut ret) }, 0);
    assert_eq!(ret, 1020);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_sb_append_obj(vm, [sb, jstr(vm, "abcd")].as_ptr(), &mut 0) },
        0
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_sb_append_c(vm, [sb, u32::from(b'x')].as_ptr(), &mut 0) };
    assert_eq!(harness(rc, 0), HARNESS_THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/OutOfMemoryError;");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_sb_length(vm, [sb].as_ptr(), &mut ret) }, 0);
    assert_eq!(ret, 1024);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_sb_append_i(vm, [0, 1].as_ptr(), &mut 0) };
    assert_eq!(rc, THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/NullPointerException;");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_sb_append_i(vm, [dix, 1].as_ptr(), &mut 0) };
    assert_eq!(rc, THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/ClassCastException;");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_sb_append_i(vm, [0x7fff_fff0, 1].as_ptr(), &mut 0) };
    assert_eq!(rc, THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/ClassCastException;");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let rc = unsafe { dr_sb_length(vm, [dix].as_ptr(), &mut ret) };
    assert_eq!(rc, THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/NullPointerException;");
}

#[test]
fn arraycopy_recouvrement_bornes_et_types() {
    let mut h = open();
    let vm = h.vm.as_mut() as *mut t_dvm;
    let a = with_world(|w| w.make_arr(c"[I".as_ptr(), 5, true));
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let copy = |args: [u32; 5]| unsafe { dr_sys_arraycopy(vm, args.as_ptr(), ptr::null_mut()) };
    assert_eq!(copy([a, 0, a, 1, 4]), 0);
    assert_eq!(
        arr_at(a, 1) * 1000 + arr_at(a, 2) * 100 + arr_at(a, 4),
        1204
    );
    assert_eq!(copy([a, 1, a, 0, 4]), 0);
    assert_eq!(arr_at(a, 0) * 1000 + arr_at(a, 3) * 10 + arr_at(a, 4), 1044);
    assert_eq!(copy([a, 5, a, 5, 0]), 0);
    assert_eq!(copy([a, 2, a, 0, 4]), THROWN);
    assert_eq!(
        pending_desc(vm),
        "Ljava/lang/ArrayIndexOutOfBoundsException;"
    );
    assert_eq!(copy([a, 0, a, 2, 4]), THROWN);
    assert_eq!(copy([a, i32::MIN as u32, a, 0, 1]), THROWN);
    assert_eq!(copy([a, 0, a, 0, 0xffff_ffff]), THROWN);
    assert_eq!(copy([a, 6, a, 0, 0]), THROWN);
    assert_eq!(copy([0, 0, a, 0, 1]), THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/NullPointerException;");
    assert_eq!(copy([a, 0, 0, 0, 1]), THROWN);
    let shorts = with_world(|w| w.make_arr(c"[S".as_ptr(), 4, false));
    let objs = with_world(|w| w.make_arr(c"[Ljava/lang/Object;".as_ptr(), 2, false));
    let ints = with_world(|w| w.make_arr(c"[I".as_ptr(), 4, true));
    assert_eq!(copy([ints, 0, shorts, 0, 1]), THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/ArrayStoreException;");
    assert_eq!(copy([ints, 0, objs, 0, 1]), THROWN);
    assert_eq!(copy([jstr(vm, "x"), 0, objs, 0, 1]), THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/ArrayStoreException;");
    let strs = with_world(|w| w.make_arr(c"[Ljava/lang/String;".as_ptr(), 2, false));
    set_u32(objs, 0, jstr(vm, "un"));
    set_u32(objs, 1, new_obj(vm, OBJ));
    assert_eq!(copy([objs, 0, strs, 0, 2]), THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/ArrayStoreException;");
    assert_ne!(arr_u32(strs, 0), 0);
    assert_eq!(arr_u32(strs, 1), 0);
    assert_eq!(copy([strs, 0, objs, 1, 1]), 0);
}

#[test]
fn vues_cycles_journal_et_texte() {
    let mut h = open();
    let vm = h.vm.as_mut() as *mut t_dvm;
    let tv = init_view(vm, TV);
    assert_eq!(h.d.views[0].kind, DV_TEXT);
    let mut units = vec![u16::from(b'a'); 300];
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [tv, jstr16(vm, &units)].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(cstr(h.d.views[0].text.as_ptr().cast()).len(), 255);
    units.truncate(254);
    units.push(0x00e9);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [tv, jstr16(vm, &units)].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(cstr(h.d.views[0].text.as_ptr().cast()).len(), 254);
    units = vec![u16::from(b'a'); 253];
    units.push(0x00e9);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [tv, jstr16(vm, &units)].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(cstr(h.d.views[0].text.as_ptr().cast()).len(), 255);
    units = vec![u16::from(b'a'); 253];
    units.push(0x20ac);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [tv, jstr16(vm, &units)].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(cstr(h.d.views[0].text.as_ptr().cast()).len(), 253);
    units = vec![u16::from(b'a'); 252];
    units.push(0xd83d);
    units.push(0xde00);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [tv, jstr16(vm, &units)].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(cstr(h.d.views[0].text.as_ptr().cast()).len(), 252);
    let mut ret = 0u64;
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_tv_gettext(vm, [tv].as_ptr(), &mut ret) }, 0);
    let (rc, n) = call(vm, STR, c"length", c"()I", &[ret as u32]);
    assert_eq!(harness(rc, n), 252);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [tv, 0].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(cstr(h.d.views[0].text.as_ptr().cast()), "");

    let a = init_view(vm, c"Landroid/widget/LinearLayout;");
    let b = init_view(vm, c"Landroid/widget/LinearLayout;");
    let c = init_view(vm, c"Landroid/widget/LinearLayout;");
    assert_eq!(h.d.views[1].kind, DV_LINEAR);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [a, b].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [b, c].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [c, a].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/IllegalStateException;");
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [a, a].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [a, c].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [a, 0].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/NullPointerException;");
    let s = jstr(vm, "x");
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [a, s].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/ClassCastException;");
    assert_eq!(h.d.views[1].nkids, 1);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_ll_orient(vm, [a, 1].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(h.d.views[1].orientation, 1);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_ll_orient(vm, [a, 2].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/IllegalArgumentException;");
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [s, s].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/ClassCastException;");
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [0, s].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/NullPointerException;");
    let brute = new_obj(vm, TV);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_tv_settext(vm, [brute, s].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_vg_addview(vm, [tv, tv].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_ll_orient(vm, [tv, 1].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_view_listen(vm, [tv, s].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/ClassCastException;");
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_view_listen(vm, [tv, 0].as_ptr(), ptr::null_mut()) },
        0
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_view_init(vm, [tv].as_ptr(), ptr::null_mut()) },
        THROWN
    );
    assert_eq!(pending_desc(vm), "Ljava/lang/IllegalStateException;");
    let nviews = h.d.nviews;
    assert_eq!(nviews, 4);
    let btn = init_view(vm, c"Landroid/widget/Button;");
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { (*droid_view(h.d.as_ref(), btn_id(h.d.as_ref(), btn))).kind },
        DV_BUTTON
    );

    h.d.log = Some(test_log);
    h.d.user = ptr::null_mut();
    let tag = jstr(vm, "Étiq");
    let msg = jstr(vm, "été");
    let (rc, ret) = call(
        vm,
        c"Landroid/util/Log;",
        c"d",
        c"(Ljava/lang/String;Ljava/lang/String;)I",
        &[tag, msg],
    );
    assert_eq!(harness(rc, ret), 5);
    assert_eq!(with_world(|w| w.level), DROID_LOG_DEBUG);
    assert_eq!(with_world(|w| w.line.clone()), "Étiq: été");
    let _ = call(
        vm,
        c"Landroid/util/Log;",
        c"i",
        c"(Ljava/lang/String;Ljava/lang/String;)I",
        &[tag, msg],
    );
    assert_eq!(with_world(|w| w.level), DROID_LOG_INFO);
    let _ = call(
        vm,
        c"Landroid/util/Log;",
        c"w",
        c"(Ljava/lang/String;Ljava/lang/String;)I",
        &[tag, msg],
    );
    assert_eq!(with_world(|w| w.level), DROID_LOG_WARN);
    let _ = call(
        vm,
        c"Landroid/util/Log;",
        c"e",
        c"(Ljava/lang/String;Ljava/lang/String;)I",
        &[tag, msg],
    );
    assert_eq!(with_world(|w| w.level), DROID_LOG_ERROR);
    assert_eq!(with_world(|w| w.logs), 4);
    let (rc, _) = call(
        vm,
        c"Landroid/util/Log;",
        c"e",
        c"(Ljava/lang/String;Ljava/lang/String;)I",
        &[tag, 0],
    );
    assert_eq!(harness(rc, 0), HARNESS_THROWN);
    assert_eq!(pending_desc(vm), "Ljava/lang/NullPointerException;");
    h.d.log = None;
    let (rc, ret) = call(
        vm,
        c"Landroid/util/Log;",
        c"e",
        c"(Ljava/lang/String;Ljava/lang/String;)I",
        &[tag, msg],
    );
    assert_eq!(harness(rc, ret), 5);
    assert_eq!(with_world(|w| w.logs), 4);
}

fn btn_id(d: *const t_droid, r: u32) -> u32 {
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    let n = unsafe { (*d).nviews };
    let mut i = 0u32;
    while i < n {
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        if unsafe { (*d).views[i as usize].self_ref } == r {
            return i + 1;
        }
        i += 1;
    }
    0
}

#[test]
fn cycle_de_vie_plafonds_et_dr_fail() {
    let mut h = open();
    let vm = h.vm.as_mut() as *mut t_dvm;
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_start(ptr::null_mut(), c"Landroid/app/Activity;".as_ptr()) },
        E_INVAL
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_start(h.d.as_mut(), ptr::null()) }, E_INVAL);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_start(h.d.as_mut(), c"Lx/Absente;".as_ptr()) },
        E_NOENT
    );
    assert_eq!(err_of(h.d.as_ref()), "arrêt de la machine virtuelle");
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_start(h.d.as_mut(), c"Ljava/lang/Integer;".as_ptr()) },
        THROWN
    );
    assert_eq!(
        err_of(h.d.as_ref()),
        "java.lang.ClassCastException : Ljava/lang/Integer;"
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { (*vm).pending }, 0);
    with_world(|w| w.fail_pin = true);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_start(h.d.as_mut(), ACT.as_ptr()) }, THROWN);
    assert_eq!(
        err_of(h.d.as_ref()),
        "java.lang.ClassCastException : Landroid/app/Activity;"
    );
    assert_eq!(h.d.activity, 0);
    with_world(|w| w.fail_pin = false);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_start(h.d.as_mut(), ACT.as_ptr()) }, 0);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_start(h.d.as_mut(), ACT.as_ptr()) }, E_INVAL);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_click(h.d.as_mut(), 0) }, E_INVAL);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert!(unsafe { droid_view(h.d.as_ref(), 0) }.is_null());
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_stop(h.d.as_mut()) }, 0);
    assert_eq!(h.d.activity, 0);
    assert_eq!(h.d.nviews, 0);
    assert_eq!(h.d.dirty, 1);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_stop(h.d.as_mut()) }, E_INVAL);

    subclass(
        c"Lcom/velum/d11/Casse;",
        ACT,
        None,
        &[(c"onCreate", c"(Landroid/os/Bundle;)V", boom)],
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_start(h.d.as_mut(), c"Lcom/velum/d11/Casse;".as_ptr()) },
        THROWN
    );
    assert_eq!(
        err_of(h.d.as_ref()),
        "java.lang.IllegalStateException : panne"
    );
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { (*vm).pending }, 0);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { (*vm).sp }, 0);
    let (rc, ret) = call(vm, c"Ljava/lang/Math;", c"abs", c"(I)I", &[(-5i32) as u32]);
    assert_eq!(harness(rc, ret), 5);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_stop(h.d.as_mut()) }, 0);

    subclass(
        c"Lcom/velum/d11/Clic;",
        ACT,
        None,
        &[(c"onDestroy", c"()V", on_destroy)],
    );
    subclass(
        c"Lcom/velum/Ear;",
        OBJ,
        Some(OCL),
        &[(c"onClick", c"(Landroid/view/View;)V", click_boom)],
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_start(h.d.as_mut(), c"Lcom/velum/d11/Clic;".as_ptr()) },
        0
    );
    let ear = new_obj(vm, c"Lcom/velum/Ear;");
    let v = init_view(vm, TV);
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { dr_view_listen(vm, [v, ear].as_ptr(), ptr::null_mut()) },
        0
    );
    let id = btn_id(h.d.as_ref(), v);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_click(h.d.as_mut(), id) }, THROWN);
    assert!(err_of(h.d.as_ref()).contains("java.lang.ArithmeticException"));
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { (*vm).pending }, 0);
    assert_eq!(h.d.finished, 0);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_stop(h.d.as_mut()) }, 0);
    assert_eq!(h.d.finished, 1);

    subclass(
        c"Lcom/velum/d11/Foule;",
        ACT,
        None,
        &[(c"onCreate", c"(Landroid/os/Bundle;)V", crowd)],
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_start(h.d.as_mut(), c"Lcom/velum/d11/Foule;".as_ptr()) },
        THROWN
    );
    assert_eq!(
        err_of(h.d.as_ref()),
        "java.lang.OutOfMemoryError : trop de vues"
    );
    assert_eq!(h.d.nviews, DROID_VIEWS_MAX);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { droid_stop(h.d.as_mut()) }, 0);

    subclass(
        c"Lcom/velum/d11/Famille;",
        ACT,
        None,
        &[(c"onCreate", c"(Landroid/os/Bundle;)V", family)],
    );
    assert_eq!(
        // SAFETY: l'appelant garantit les pointeurs de ce bloc.
        unsafe { droid_start(h.d.as_mut(), c"Lcom/velum/d11/Famille;".as_ptr()) },
        THROWN
    );
    assert_eq!(
        err_of(h.d.as_ref()),
        "java.lang.IllegalStateException : trop d'enfants"
    );
    assert_eq!(h.d.views[0].nkids, DROID_KIDS_MAX);
    assert_eq!(h.d.nviews, 18);

    let nm = t_dname {
        name: c"setTextColor".as_ptr(),
        sig: c"(I)V".as_ptr(),
    };
    let mut c = ptr::null_mut();
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dvm_class(vm, TV.as_ptr(), &mut c) }, 0);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_missing(h.d.as_mut(), c, &nm) }, THROWN);
    let err = err_of(h.d.as_ref());
    assert!(err.contains("java.lang.NoSuchMethodError"));
    assert!(err.contains("setTextColor(I)V"));
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_fail(h.d.as_mut(), 0) }, 0);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_fail(h.d.as_mut(), E_TIMEOUT) }, E_TIMEOUT);
    assert_eq!(err_of(h.d.as_ref()), "budget d'instructions épuisé");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert_eq!(unsafe { dr_fail(h.d.as_mut(), E_NOMEM) }, E_NOMEM);
    assert_eq!(err_of(h.d.as_ref()), "mémoire épuisée");
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    unsafe { dr_release(h.d.as_mut()) };
    assert_eq!(h.d.nviews, 0);
    // SAFETY: l'appelant garantit les pointeurs de ce bloc.
    assert!(unsafe { droid_view(h.d.as_ref(), 1) }.is_null());
}
