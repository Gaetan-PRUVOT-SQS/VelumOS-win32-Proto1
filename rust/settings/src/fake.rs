use core::ffi::{c_char, c_int};
use std::sync::Mutex;

const SLOTS: usize = 4;
const NAME: usize = 96;
const DATA: usize = 640;
const STEP: usize = 5;
const LOG: usize = 16;

const IO: i64 = -5;
const NOENT: i64 = -2;
const INVAL: i64 = -22;
const FAULT: i64 = -14;
const BADF: i64 = -9;
const BUSY: i64 = -16;
const NOSPC: i64 = -28;

#[derive(Clone, Copy)]
struct Slot {
    used: bool,
    open: bool,
    writable: bool,
    name: [u8; NAME],
    name_len: usize,
    data: [u8; DATA],
    len: usize,
    pos: usize,
}

impl Slot {
    const EMPTY: Self = Self {
        used: false,
        open: false,
        writable: false,
        name: [0; NAME],
        name_len: 0,
        data: [0; DATA],
        len: 0,
        pos: 0,
    };
}

#[derive(Clone, Copy)]
struct Rec {
    flags: u32,
    mode: u32,
    path: [u8; NAME],
    path_len: usize,
}

impl Rec {
    const EMPTY: Self = Self {
        flags: 0,
        mode: 0,
        path: [0; NAME],
        path_len: 0,
    };
}

struct Fs {
    slots: [Slot; SLOTS],
    write_left: i32,
    read_left: i32,
    rename_fail: bool,
    fsync_fail: bool,
    open_fail: bool,
    opens: usize,
    log: [Rec; LOG],
}

impl Fs {
    const fn new() -> Self {
        Self {
            slots: [Slot::EMPTY; SLOTS],
            write_left: -1,
            read_left: -1,
            rename_fail: false,
            fsync_fail: false,
            open_fail: false,
            opens: 0,
            log: [Rec::EMPTY; LOG],
        }
    }
}

static FS: Mutex<Fs> = Mutex::new(Fs::new());

fn with_fs<R>(f: impl FnOnce(&mut Fs) -> R) -> R {
    let mut g = match FS.lock() {
        Ok(g) => g,
        Err(p) => p.into_inner(),
    };
    f(&mut g)
}

fn take(budget: &mut i32, want: usize) -> i64 {
    if want == 0 {
        return 0;
    }
    if *budget == 0 {
        return IO;
    }
    let mut n = want;
    if n > STEP {
        n = STEP;
    }
    if *budget > 0 {
        let left = match usize::try_from(*budget) {
            Ok(v) => v,
            Err(_) => return IO,
        };
        if left < n {
            n = left;
        }
        let Ok(sub) = i32::try_from(n) else {
            return IO;
        };
        *budget = budget.saturating_sub(sub);
    }
    match i64::try_from(n) {
        Ok(v) => v,
        Err(_) => IO,
    }
}

fn read_name(path: *const c_char) -> Result<([u8; NAME], usize), i64> {
    if path.is_null() {
        return Err(FAULT);
    }
    let mut name = [0u8; NAME];
    let mut i = 0usize;
    while i < NAME {
        // SAFETY: chemin C, au plus NAME octets, arrêt au NUL.
        let b = unsafe { path.cast::<u8>().add(i).read() };
        if b == 0 {
            if i == 0 {
                return Err(INVAL);
            }
            return Ok((name, i));
        }
        name[i] = b;
        i = match i.checked_add(1) {
            Some(n) => n,
            None => return Err(INVAL),
        };
    }
    Err(INVAL)
}

fn find(slots: &[Slot], name: &[u8]) -> Option<usize> {
    slots
        .iter()
        .position(|s| s.used && s.name_len == name.len() && s.name[..name.len()] == *name)
}

fn alloc(slots: &[Slot]) -> Option<usize> {
    slots.iter().position(|s| !s.used)
}

fn slot_of(slots: &[Slot], file: u32) -> Option<usize> {
    let Ok(h) = usize::try_from(file) else {
        return None;
    };
    if h == 0 || h > slots.len() {
        return None;
    }
    let i = h - 1;
    if !slots[i].used || !slots[i].open {
        return None;
    }
    Some(i)
}

fn note(fs: &mut Fs, flags: u32, mode: u32, name: &[u8]) {
    if fs.opens < LOG && name.len() <= NAME {
        let mut rec = Rec::EMPTY;
        rec.flags = flags;
        rec.mode = mode;
        rec.path[..name.len()].copy_from_slice(name);
        rec.path_len = name.len();
        fs.log[fs.opens] = rec;
    }
    fs.opens = fs.opens.saturating_add(1);
}

pub fn reset() {
    with_fs(|fs| *fs = Fs::new());
}

pub fn write_budget(bytes: i32) {
    with_fs(|fs| fs.write_left = bytes);
}

pub fn read_budget(bytes: i32) {
    with_fs(|fs| fs.read_left = bytes);
}

pub fn rename_fail(on: bool) {
    with_fs(|fs| fs.rename_fail = on);
}

pub fn fsync_fail(on: bool) {
    with_fs(|fs| fs.fsync_fail = on);
}

pub fn open_fail(on: bool) {
    with_fs(|fs| fs.open_fail = on);
}

pub fn nfiles() -> c_int {
    with_fs(|fs| {
        let n = fs.slots.iter().filter(|s| s.used).count();
        c_int::try_from(n).unwrap_or(-75)
    })
}

pub fn plant(path: &[u8], data: &[u8]) -> c_int {
    if path.is_empty() || path.len() >= NAME || path.contains(&0) || data.len() > DATA {
        return -22;
    }
    with_fs(|fs| {
        let i = match find(&fs.slots, path).or_else(|| alloc(&fs.slots)) {
            Some(i) => i,
            None => return -28,
        };
        fs.slots[i] = Slot::EMPTY;
        fs.slots[i].name[..path.len()].copy_from_slice(path);
        fs.slots[i].name_len = path.len();
        if !data.is_empty() {
            fs.slots[i].data[..data.len()].copy_from_slice(data);
        }
        fs.slots[i].len = data.len();
        fs.slots[i].used = true;
        0
    })
}

pub fn slurp(path: &[u8], out: &mut [u8]) -> c_int {
    with_fs(|fs| {
        let Some(i) = find(&fs.slots, path) else {
            return -2;
        };
        let n = fs.slots[i].len;
        if n > out.len() {
            return -34;
        }
        if n > 0 {
            out[..n].copy_from_slice(&fs.slots[i].data[..n]);
        }
        c_int::try_from(n).unwrap_or(-75)
    })
}

pub struct Opened {
    pub flags: u32,
    pub mode: u32,
    pub path: Vec<u8>,
}

pub fn opened() -> Vec<Opened> {
    with_fs(|fs| {
        let n = fs.opens.min(LOG);
        let mut out = Vec::with_capacity(n);
        let mut i = 0usize;
        while i < n {
            let rec = &fs.log[i];
            out.push(Opened {
                flags: rec.flags,
                mode: rec.mode,
                path: rec.path[..rec.path_len].to_vec(),
            });
            i += 1;
        }
        out
    })
}

/// # Safety
/// `path` est nul, ou bien une chaîne C.
pub unsafe fn v_open(path: *const c_char, flags: u32, mode: u32) -> i64 {
    let (name, name_len) = match read_name(path) {
        Ok(v) => v,
        Err(e) => return e,
    };
    with_fs(|fs| {
        note(fs, flags, mode, &name[..name_len]);
        if fs.open_fail {
            return IO;
        }
        let writing = (flags & 0x3) != 0;
        let i = match find(&fs.slots, &name[..name_len]) {
            Some(i) => i,
            None => {
                if (flags & 0x40) == 0 {
                    return NOENT;
                }
                match alloc(&fs.slots) {
                    Some(i) => {
                        fs.slots[i] = Slot::EMPTY;
                        fs.slots[i].name = name;
                        fs.slots[i].name_len = name_len;
                        fs.slots[i].used = true;
                        i
                    }
                    None => return NOSPC,
                }
            }
        };
        if fs.slots[i].open {
            return BUSY;
        }
        if (flags & 0x200) != 0 {
            fs.slots[i].len = 0;
        }
        fs.slots[i].pos = 0;
        fs.slots[i].open = true;
        fs.slots[i].writable = writing;
        match i64::try_from(i.checked_add(1).unwrap_or(0)) {
            Ok(h) if h > 0 => h,
            _ => IO,
        }
    })
}

/// # Safety
/// `buf` est nul, ou bien `len` octets exclusifs.
pub unsafe fn v_read(file: u32, buf: *mut u8, len: usize) -> i64 {
    with_fs(|fs| {
        let Some(i) = slot_of(&fs.slots, file) else {
            return BADF;
        };
        if fs.slots[i].writable || buf.is_null() {
            return BADF;
        }
        let mut left = fs.slots[i].len - fs.slots[i].pos;
        if left == 0 || len == 0 {
            return 0;
        }
        if len < left {
            left = len;
        }
        let n = take(&mut fs.read_left, left);
        if n <= 0 {
            return n;
        }
        let Some(n_usz) = usize::try_from(n).ok() else {
            return IO;
        };
        let pos = fs.slots[i].pos;
        // SAFETY: buf non nul, n_usz <= len, source dans le slot, pas de recouvrement.
        unsafe {
            core::ptr::copy_nonoverlapping(fs.slots[i].data.as_ptr().add(pos), buf, n_usz);
        }
        fs.slots[i].pos = match pos.checked_add(n_usz) {
            Some(v) => v,
            None => return IO,
        };
        n
    })
}

/// # Safety
/// `buf` est nul seulement si `len` vaut 0. Sinon il pointe `len` octets.
pub unsafe fn v_write(file: u32, buf: *const u8, len: usize) -> i64 {
    with_fs(|fs| {
        let Some(i) = slot_of(&fs.slots, file) else {
            return BADF;
        };
        if !fs.slots[i].writable || (len > 0 && buf.is_null()) {
            return BADF;
        }
        if len == 0 {
            return 0;
        }
        if fs.slots[i].len >= DATA {
            return NOSPC;
        }
        let mut room = DATA - fs.slots[i].len;
        if len < room {
            room = len;
        }
        let n = take(&mut fs.write_left, room);
        if n <= 0 {
            return n;
        }
        let Some(n_usz) = usize::try_from(n).ok() else {
            return IO;
        };
        let at = fs.slots[i].len;
        // SAFETY: buf non nul, n_usz octets, destination dans le slot, pas de recouvrement.
        unsafe {
            core::ptr::copy_nonoverlapping(buf, fs.slots[i].data.as_mut_ptr().add(at), n_usz);
        }
        fs.slots[i].len = match at.checked_add(n_usz) {
            Some(v) => v,
            None => return IO,
        };
        n
    })
}

/// # Safety
/// `h` est un descripteur, ou bien n'importe quelle valeur : un mauvais numéro rend E_BADF.
pub unsafe fn v_close(h: u32) -> c_int {
    with_fs(|fs| {
        let Some(i) = slot_of(&fs.slots, h) else {
            return -9;
        };
        fs.slots[i].open = false;
        0
    })
}

/// # Safety
/// `path` est nul, ou bien une chaîne C.
pub unsafe fn v_unlink(path: *const c_char) -> c_int {
    let (name, name_len) = match read_name(path) {
        Ok(v) => v,
        Err(e) => return narrow_err(e),
    };
    with_fs(|fs| {
        let Some(i) = find(&fs.slots, &name[..name_len]) else {
            return -2;
        };
        fs.slots[i] = Slot::EMPTY;
        0
    })
}

/// # Safety
/// Les deux chemins sont nuls, ou bien des chaînes C.
pub unsafe fn v_rename(old_path: *const c_char, new_path: *const c_char) -> c_int {
    let (old, old_len) = match read_name(old_path) {
        Ok(v) => v,
        Err(e) => return narrow_err(e),
    };
    let (new, new_len) = match read_name(new_path) {
        Ok(v) => v,
        Err(e) => return narrow_err(e),
    };
    with_fs(|fs| {
        if fs.rename_fail {
            return -5;
        }
        let Some(from) = find(&fs.slots, &old[..old_len]) else {
            return -2;
        };
        if old[..old_len] == new[..new_len] {
            return 0;
        }
        if new_len >= NAME {
            return -34;
        }
        if let Some(to) = find(&fs.slots, &new[..new_len]) {
            fs.slots[to].used = false;
            fs.slots[to].open = false;
        }
        fs.slots[from].name = [0; NAME];
        fs.slots[from].name[..new_len].copy_from_slice(&new[..new_len]);
        fs.slots[from].name_len = new_len;
        0
    })
}

/// # Safety
/// `file` est un descripteur, ou bien une valeur quelconque.
pub unsafe fn v_fsync(file: u32) -> c_int {
    with_fs(|fs| {
        if slot_of(&fs.slots, file).is_none() {
            return -9;
        }
        if fs.fsync_fail {
            return -5;
        }
        0
    })
}

fn narrow_err(n: i64) -> c_int {
    c_int::try_from(n).unwrap_or(-75)
}
