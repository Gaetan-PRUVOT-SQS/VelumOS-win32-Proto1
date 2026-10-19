#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

use core::ffi::{c_char, c_int, c_void};

fn read_u8(p: *const u8, i: usize) -> u8 {
    // SAFETY: l'appelant a écarté le pointeur nul et i reste dans la zone lue.
    unsafe { p.add(i).read() }
}

fn write_u8(p: *mut u8, i: usize, v: u8) {
    // SAFETY: l'appelant a écarté le pointeur nul et i reste dans la zone écrite.
    unsafe { p.add(i).write(v) }
}

fn cstr_len(s: *const c_char) -> usize {
    let mut n = 0usize;
    while read_u8(s.cast(), n) != 0 {
        n += 1;
    }
    n
}

fn strlen_raw(s: *const c_char) -> usize {
    if s.is_null() {
        return 0;
    }
    cstr_len(s)
}

fn strnlen_raw(s: *const c_char, max: usize) -> usize {
    if s.is_null() {
        return 0;
    }
    let mut n = 0usize;
    while n < max && read_u8(s.cast(), n) != 0 {
        n += 1;
    }
    n
}

fn byte(s: *const c_char, i: usize) -> u8 {
    read_u8(s.cast(), i)
}

fn strcmp_raw(a: *const c_char, b: *const c_char) -> c_int {
    if a.is_null() || b.is_null() {
        return 0;
    }
    let mut i = 0usize;
    loop {
        let ca = byte(a, i);
        let cb = byte(b, i);
        if ca != cb || ca == 0 {
            return c_int::from(ca) - c_int::from(cb);
        }
        i += 1;
    }
}

fn strncmp_raw(a: *const c_char, b: *const c_char, n: usize) -> c_int {
    if n == 0 || a.is_null() || b.is_null() {
        return 0;
    }
    let mut i = 0usize;
    while i < n {
        let ca = byte(a, i);
        let cb = byte(b, i);
        if ca != cb || ca == 0 {
            return c_int::from(ca) - c_int::from(cb);
        }
        i += 1;
    }
    0
}

fn strchr_raw(s: *const c_char, c: c_int) -> *const c_char {
    if s.is_null() {
        return core::ptr::null();
    }
    let want = c as u8;
    let mut i = 0usize;
    loop {
        let b = byte(s, i);
        if b == want {
            return s.wrapping_add(i);
        }
        if b == 0 {
            return core::ptr::null();
        }
        i += 1;
    }
}

fn strrchr_raw(s: *const c_char, c: c_int) -> *const c_char {
    if s.is_null() {
        return core::ptr::null();
    }
    let want = c as u8;
    let mut last: *const c_char = core::ptr::null();
    let mut i = 0usize;
    loop {
        let b = byte(s, i);
        if b == want {
            last = s.wrapping_add(i);
        }
        if b == 0 {
            if want == 0 {
                return s.wrapping_add(i);
            }
            return last;
        }
        i += 1;
    }
}

fn strstr_raw(hay: *const c_char, needle: *const c_char) -> *const c_char {
    if hay.is_null() || needle.is_null() {
        return core::ptr::null();
    }
    let n = cstr_len(needle);
    if n == 0 {
        return hay;
    }
    let mut i = 0usize;
    loop {
        if byte(hay, i) == 0 {
            return core::ptr::null();
        }
        if strncmp_raw(hay.wrapping_add(i), needle, n) == 0 {
            return hay.wrapping_add(i);
        }
        i += 1;
    }
}

fn copy_bytes(dst: *mut u8, src: *const u8, n: usize) {
    if n == 0 || dst.is_null() || src.is_null() {
        return;
    }
    let mut i = 0usize;
    while i < n {
        write_u8(dst, i, read_u8(src, i));
        i += 1;
    }
}

#[cfg_attr(not(any(velum_kernel, velum_user)), allow(dead_code))]
fn move_bytes(dst: *mut u8, src: *const u8, n: usize) {
    if n == 0 || dst.is_null() || src.is_null() {
        return;
    }
    let d = dst as usize;
    let s = src as usize;
    if d > s && d < s.wrapping_add(n) {
        let mut i = n;
        while i > 0 {
            i -= 1;
            write_u8(dst, i, read_u8(src, i));
        }
        return;
    }
    copy_bytes(dst, src, n);
}

#[cfg_attr(not(any(velum_kernel, velum_user)), allow(dead_code))]
fn memset_raw(dst: *mut c_void, c: c_int, n: usize) -> *mut c_void {
    if !dst.is_null() {
        let p = dst.cast::<u8>();
        let b = c as u8;
        let mut i = 0usize;
        while i < n {
            write_u8(p, i, b);
            i += 1;
        }
    }
    dst
}

fn memcmp_raw(a: *const c_void, b: *const c_void, n: usize) -> c_int {
    if n == 0 || a.is_null() || b.is_null() {
        return 0;
    }
    let pa = a.cast::<u8>();
    let pb = b.cast::<u8>();
    let mut i = 0usize;
    while i < n {
        let ca = read_u8(pa, i);
        let cb = read_u8(pb, i);
        if ca != cb {
            return c_int::from(ca) - c_int::from(cb);
        }
        i += 1;
    }
    0
}

fn memchr_raw(s: *const c_void, c: c_int, n: usize) -> *const c_void {
    if n == 0 || s.is_null() {
        return core::ptr::null();
    }
    let p = s.cast::<u8>();
    let want = c as u8;
    let mut i = 0usize;
    while i < n {
        if read_u8(p, i) == want {
            return p.wrapping_add(i).cast();
        }
        i += 1;
    }
    core::ptr::null()
}

fn strlcpy_raw(dst: *mut c_char, src: *const c_char, size: usize) -> usize {
    if src.is_null() {
        return 0;
    }
    let len = cstr_len(src);
    if size == 0 || dst.is_null() {
        return len;
    }
    let n = if len >= size { size - 1 } else { len };
    copy_bytes(dst.cast(), src.cast(), n);
    write_u8(dst.cast(), n, 0);
    len
}

fn strlcat_raw(dst: *mut c_char, src: *const c_char, size: usize) -> usize {
    if dst.is_null() || src.is_null() {
        return 0;
    }
    let dlen = strnlen_raw(dst, size);
    let slen = strlen_raw(src);
    if dlen == size {
        return size.wrapping_add(slen);
    }
    dlen.wrapping_add(strlcpy_raw(dst.wrapping_add(dlen), src, size - dlen))
}

macro_rules! ffi_renamed {
    ($raw:ident, $name:ident, $vk:ident ( $($arg:ident : $ty:ty),* ) -> $ret:ty) => {
        #[cfg(any(velum_kernel, velum_user))]
        #[unsafe(no_mangle)]
        /// # Safety
        /// Même contrat que la fonction C du même nom. Un pointeur nul est ignoré.
        pub unsafe extern "C" fn $name($($arg: $ty),*) -> $ret {
            $raw($($arg),*)
        }

        #[cfg(not(any(velum_kernel, velum_user)))]
        #[unsafe(no_mangle)]
        /// # Safety
        /// Même contrat que la fonction C du même nom. Un pointeur nul est ignoré.
        pub unsafe extern "C" fn $vk($($arg: $ty),*) -> $ret {
            $raw($($arg),*)
        }
    };
}

ffi_renamed!(strlen_raw, strlen, vk_strlen(s: *const c_char) -> usize);
ffi_renamed!(
    strnlen_raw,
    strnlen,
    vk_strnlen(s: *const c_char, max: usize) -> usize
);
ffi_renamed!(
    strcmp_raw,
    strcmp,
    vk_strcmp(a: *const c_char, b: *const c_char) -> c_int
);
ffi_renamed!(
    strncmp_raw,
    strncmp,
    vk_strncmp(a: *const c_char, b: *const c_char, n: usize) -> c_int
);
ffi_renamed!(
    strchr_raw,
    strchr,
    vk_strchr(s: *const c_char, c: c_int) -> *const c_char
);
ffi_renamed!(
    strrchr_raw,
    strrchr,
    vk_strrchr(s: *const c_char, c: c_int) -> *const c_char
);
ffi_renamed!(
    strstr_raw,
    strstr,
    vk_strstr(hay: *const c_char, needle: *const c_char) -> *const c_char
);
ffi_renamed!(
    strlcpy_raw,
    strlcpy,
    vk_strlcpy(dst: *mut c_char, src: *const c_char, size: usize) -> usize
);
ffi_renamed!(
    strlcat_raw,
    strlcat,
    vk_strlcat(dst: *mut c_char, src: *const c_char, size: usize) -> usize
);
ffi_renamed!(
    memcmp_raw,
    memcmp,
    vk_memcmp(a: *const c_void, b: *const c_void, n: usize) -> c_int
);
ffi_renamed!(
    memchr_raw,
    memchr,
    vk_memchr(s: *const c_void, c: c_int, n: usize) -> *const c_void
);

/// # Safety
/// `dst` et `src` sont valides pour `n` octets et ne se chevauchent pas, ou `n` vaut 0.
#[cfg(any(velum_kernel, velum_user))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn memcpy(dst: *mut c_void, src: *const c_void, n: usize) -> *mut c_void {
    copy_bytes(dst.cast(), src.cast(), n);
    dst
}

/// # Safety
/// `dst` et `src` sont valides pour `n` octets. Le chevauchement est géré.
#[cfg(any(velum_kernel, velum_user))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn memmove(dst: *mut c_void, src: *const c_void, n: usize) -> *mut c_void {
    move_bytes(dst.cast(), src.cast(), n);
    dst
}

/// # Safety
/// `dst` est valide pour `n` octets, ou `n` vaut 0.
#[cfg(any(velum_kernel, velum_user))]
#[unsafe(no_mangle)]
pub unsafe extern "C" fn memset(dst: *mut c_void, c: c_int, n: usize) -> *mut c_void {
    memset_raw(dst, c, n)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn s(bytes: &[u8]) -> *const c_char {
        bytes.as_ptr().cast()
    }

    #[test]
    fn strlen_empty() {
        assert_eq!(strlen_raw(s(b"\0")), 0);
    }

    #[test]
    fn strlen_two() {
        assert_eq!(strlen_raw(s(b"ab\0")), 2);
    }

    #[test]
    fn strlen_null() {
        assert_eq!(strlen_raw(core::ptr::null()), 0);
    }

    #[test]
    fn strnlen_stops_at_max() {
        assert_eq!(strnlen_raw(s(b"abcd\0"), 2), 2);
        assert_eq!(strnlen_raw(s(b"ab\0"), 8), 2);
    }

    #[test]
    fn strcmp_high_bit() {
        assert!(strcmp_raw(s(b"\xff\0"), s(b"\x01\0")) > 0);
        assert_eq!(strcmp_raw(s(b"\0"), s(b"\0")), 0);
    }

    #[test]
    fn strncmp_zero_len_does_not_read() {
        assert_eq!(strncmp_raw(core::ptr::null(), core::ptr::null(), 0), 0);
    }

    #[test]
    fn memcmp_zero_len() {
        assert_eq!(memcmp_raw(core::ptr::null(), core::ptr::null(), 0), 0);
    }

    #[test]
    fn memcmp_last_byte() {
        let a = [1u8, 2, 3];
        let b = [1u8, 2, 4];
        assert!(memcmp_raw(a.as_ptr().cast(), b.as_ptr().cast(), 3) < 0);
        assert_eq!(memcmp_raw(a.as_ptr().cast(), b.as_ptr().cast(), 2), 0);
    }

    #[test]
    fn memcmp_high_bit() {
        let a = [0xffu8];
        let b = [0x01u8];
        assert!(memcmp_raw(a.as_ptr().cast(), b.as_ptr().cast(), 1) > 0);
    }

    #[test]
    fn memchr_finds_and_misses() {
        let a = [9u8, 8, 7];
        assert!(memchr_raw(a.as_ptr().cast(), 8, 3) == a.as_ptr().wrapping_add(1).cast());
        assert!(memchr_raw(a.as_ptr().cast(), 1, 3).is_null());
        assert!(memchr_raw(a.as_ptr().cast(), 9, 0).is_null());
    }

    #[test]
    fn memmove_overlap_backward() {
        let mut b = [1u8, 2, 3, 4, 5];
        move_bytes(b.as_mut_ptr().wrapping_add(1), b.as_ptr(), 4);
        assert_eq!(b, [1, 1, 2, 3, 4]);
    }

    #[test]
    fn memmove_overlap_forward() {
        let mut b = [1u8, 2, 3, 4, 5];
        move_bytes(b.as_mut_ptr(), b.as_ptr().wrapping_add(1), 4);
        assert_eq!(b, [2, 3, 4, 5, 5]);
    }

    #[test]
    fn memset_fills() {
        let mut b = [1u8, 2, 3];
        memset_raw(b.as_mut_ptr().cast(), 0x1ff, 2);
        assert_eq!(b, [0xff, 0xff, 3]);
    }

    #[test]
    fn strchr_nul_and_missing() {
        let p = s(b"ab\0");
        assert_eq!(strchr_raw(p, 0), p.wrapping_add(2));
        assert!(strchr_raw(p, 'z' as c_int).is_null());
        assert_eq!(strchr_raw(p, 'b' as c_int), p.wrapping_add(1));
    }

    #[test]
    fn strrchr_last() {
        let p = s(b"aba\0");
        assert_eq!(strrchr_raw(p, 'a' as c_int), p.wrapping_add(2));
        assert_eq!(strrchr_raw(p, 0), p.wrapping_add(3));
    }

    #[test]
    fn strstr_empty_needle() {
        let p = s(b"ab\0");
        assert_eq!(strstr_raw(p, s(b"\0")), p);
        assert!(strstr_raw(p, s(b"z\0")).is_null());
        assert_eq!(strstr_raw(p, s(b"b\0")), p.wrapping_add(1));
    }

    #[test]
    fn strlcpy_size_zero() {
        let src = s(b"abcd\0");
        let mut d = [9u8; 4];
        let n = strlcpy_raw(d.as_mut_ptr().cast(), src, 0);
        assert_eq!(n, 4);
        assert_eq!(d, [9, 9, 9, 9]);
    }

    #[test]
    fn strlcpy_truncates() {
        let mut d = [9u8; 4];
        let n = strlcpy_raw(d.as_mut_ptr().cast(), s(b"abcd\0"), 3);
        assert_eq!(n, 4);
        assert_eq!(&d[..3], b"ab\0");
        assert_eq!(d[3], 9);
    }

    #[test]
    fn strlcat_appends_and_full() {
        let mut d = [b'a', b'b', 0, 9, 9, 9];
        let n = strlcat_raw(d.as_mut_ptr().cast(), s(b"cd\0"), 6);
        assert_eq!(n, 4);
        assert_eq!(&d[..5], b"abcd\0");
        let mut full = *b"abcd";
        let n = strlcat_raw(full.as_mut_ptr().cast(), s(b"x\0"), 4);
        assert_eq!(n, 5);
        assert_eq!(&full, b"abcd");
    }
}
