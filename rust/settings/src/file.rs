use core::ffi::{c_char, c_int};

use crate::{
    E_INVAL, E_IO, E_OK, E_OVERFLOW, E_RANGE, MODE_0644, O_CREAT, O_RDONLY, O_TRUNC, O_WRONLY,
    SET_FILE_MAX, modes, parse, t_settings,
};

#[cfg(not(test))]
unsafe extern "C" {
    fn v_open(path: *const c_char, flags: u32, mode: u32) -> i64;
    fn v_read(file: u32, buf: *mut u8, len: usize) -> i64;
    fn v_write(file: u32, buf: *const u8, len: usize) -> i64;
    fn v_close(h: u32) -> c_int;
    fn v_unlink(path: *const c_char) -> c_int;
    fn v_rename(old_path: *const c_char, new_path: *const c_char) -> c_int;
    fn v_fsync(file: u32) -> c_int;
}

macro_rules! syscall {
    ($name:ident ( $($arg:expr),* )) => {{
        #[cfg(not(test))]
        {
            $name($($arg),*)
        }
        #[cfg(test)]
        {
            crate::fake::$name($($arg),*)
        }
    }};
}

fn narrow(n: i64) -> c_int {
    c_int::try_from(n).unwrap_or(E_OVERFLOW)
}

fn handle_of(h: i64) -> Result<u32, c_int> {
    if h < 0 {
        return Err(narrow(h));
    }
    match u32::try_from(h) {
        Ok(v) => Ok(v),
        Err(_) => Err(E_OVERFLOW),
    }
}

fn sys_open(path: *const c_char, flags: u32, mode: u32) -> i64 {
    // SAFETY: path est une chaîne C non nulle. Les drapeaux viennent de abi_syscall.h.
    unsafe { syscall!(v_open(path, flags, mode)) }
}

fn sys_read(file: u32, buf: *mut u8, len: usize) -> i64 {
    // SAFETY: buf pointe `len` octets exclusifs, ou len vaut 0.
    unsafe { syscall!(v_read(file, buf, len)) }
}

fn sys_write(file: u32, buf: *const u8, len: usize) -> i64 {
    // SAFETY: buf pointe `len` octets initialisés, ou len vaut 0.
    unsafe { syscall!(v_write(file, buf, len)) }
}

fn sys_close(file: u32) -> c_int {
    // SAFETY: file est un descripteur rendu par v_open, encore ouvert.
    unsafe { syscall!(v_close(file)) }
}

fn sys_unlink(path: *const c_char) -> c_int {
    // SAFETY: path est une chaîne C non nulle.
    unsafe { syscall!(v_unlink(path)) }
}

fn sys_rename(old_path: *const c_char, new_path: *const c_char) -> c_int {
    // SAFETY: les deux chemins sont des chaînes C non nulles.
    unsafe { syscall!(v_rename(old_path, new_path)) }
}

fn sys_fsync(file: u32) -> c_int {
    // SAFETY: file est un descripteur encore ouvert.
    unsafe { syscall!(v_fsync(file)) }
}

const LOAD_CAP: usize = SET_FILE_MAX + 1;
const TMP_SIZE: usize = 96;
const TMP_SUFFIX_SIZE: usize = 5;

fn tmp_for(path: *const c_char) -> Result<[u8; TMP_SIZE], c_int> {
    let mut tmp = [0u8; TMP_SIZE];
    let limit = TMP_SIZE - TMP_SUFFIX_SIZE;
    let mut i = 0usize;
    while i <= limit {
        // SAFETY: chemin C non nul. Au plus 92 octets, arrêt au premier NUL.
        let b = unsafe { path.cast::<u8>().add(i).read() };
        if b == 0 {
            tmp[i..i + 4].copy_from_slice(b".tmp");
            return Ok(tmp);
        }
        if i == limit {
            return Err(E_RANGE);
        }
        tmp[i] = b;
        i = match i.checked_add(1) {
            Some(n) => n,
            None => return Err(E_OVERFLOW),
        };
    }
    Err(E_RANGE)
}

fn write_all(h: u32, buf: &[u8]) -> c_int {
    let mut done = 0usize;
    while done < buf.len() {
        let rest = buf.len() - done;
        // SAFETY: done < buf.len(), le pointeur vise `rest` octets du tampon.
        let ptr = unsafe { buf.as_ptr().add(done) };
        let n = sys_write(h, ptr, rest);
        if n <= 0 {
            return if n < 0 { narrow(n) } else { E_IO };
        }
        let Ok(add) = usize::try_from(n) else {
            return E_IO;
        };
        if add > rest {
            return E_IO;
        }
        done = match done.checked_add(add) {
            Some(v) => v,
            None => return E_OVERFLOW,
        };
    }
    E_OK
}

pub(crate) fn load(path: *const c_char, s: &mut t_settings) -> c_int {
    modes::apply_default(s);
    if path.is_null() {
        return E_INVAL;
    }
    let h = match handle_of(sys_open(path, O_RDONLY, 0)) {
        Ok(h) => h,
        Err(e) => return e,
    };
    let mut buf = [0u8; LOAD_CAP];
    let mut total = 0usize;
    let mut n = 1i64;
    while n > 0 && total < buf.len() {
        let want = buf.len() - total;
        // SAFETY: total < buf.len(), want octets restent dans buf.
        let ptr = unsafe { buf.as_mut_ptr().add(total) };
        n = sys_read(h, ptr, want);
        if n > 0 {
            let Ok(add) = usize::try_from(n) else {
                let _ = sys_close(h);
                return E_IO;
            };
            if add > want {
                let _ = sys_close(h);
                return E_IO;
            }
            total = match total.checked_add(add) {
                Some(v) => v,
                None => {
                    let _ = sys_close(h);
                    return E_OVERFLOW;
                }
            };
        }
    }
    let _ = sys_close(h);
    if n < 0 {
        return narrow(n);
    }
    parse::parse(&buf[..total], s)
}

pub(crate) fn save(path: *const c_char, s: &t_settings) -> c_int {
    let mut text = [0u8; SET_FILE_MAX];
    let len_i = parse::format(s, &mut text);
    if len_i < 0 {
        return len_i;
    }
    let Ok(len) = usize::try_from(len_i) else {
        return E_OVERFLOW;
    };
    if len > text.len() {
        return E_OVERFLOW;
    }
    let tmp = match tmp_for(path) {
        Ok(tmp) => tmp,
        Err(e) => return e,
    };
    let h = match handle_of(sys_open(
        tmp.as_ptr().cast(),
        O_WRONLY | O_CREAT | O_TRUNC,
        MODE_0644,
    )) {
        Ok(h) => h,
        Err(e) => return e,
    };
    let mut r = write_all(h, &text[..len]);
    if r == 0 {
        r = sys_fsync(h);
    }
    let _ = sys_close(h);
    if r < 0 {
        let _ = sys_unlink(tmp.as_ptr().cast());
        return r;
    }
    r = sys_rename(tmp.as_ptr().cast(), path);
    if r < 0 {
        let _ = sys_unlink(tmp.as_ptr().cast());
    }
    r
}
