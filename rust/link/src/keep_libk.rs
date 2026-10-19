use core::ffi::{c_char, c_int, c_void};

type FnStrlen = unsafe extern "C" fn(*const c_char) -> usize;
type FnStrnlen = unsafe extern "C" fn(*const c_char, usize) -> usize;
type FnStrcmp = unsafe extern "C" fn(*const c_char, *const c_char) -> c_int;
type FnStrncmp = unsafe extern "C" fn(*const c_char, *const c_char, usize) -> c_int;
type FnStrchr = unsafe extern "C" fn(*const c_char, c_int) -> *const c_char;
type FnStrstr = unsafe extern "C" fn(*const c_char, *const c_char) -> *const c_char;
type FnStrl = unsafe extern "C" fn(*mut c_char, *const c_char, usize) -> usize;
type FnMemcmp = unsafe extern "C" fn(*const c_void, *const c_void, usize) -> c_int;
type FnMemchr = unsafe extern "C" fn(*const c_void, c_int, usize) -> *const c_void;
#[cfg(any(velum_kernel, velum_user))]
type FnMemcpy = unsafe extern "C" fn(*mut c_void, *const c_void, usize) -> *mut c_void;
#[cfg(any(velum_kernel, velum_user))]
type FnMemset = unsafe extern "C" fn(*mut c_void, c_int, usize) -> *mut c_void;

macro_rules! anchor {
    ($name:ident: $ty:ty = $func:path) => {
        #[used]
        static $name: $ty = $func;
    };
}

#[cfg(not(any(velum_kernel, velum_user)))]
mod host {
    use super::{
        FnMemchr, FnMemcmp, FnStrchr, FnStrcmp, FnStrl, FnStrlen, FnStrncmp, FnStrnlen, FnStrstr,
    };

    anchor!(STRLEN: FnStrlen = velum_libk::vk_strlen);
    anchor!(STRNLEN: FnStrnlen = velum_libk::vk_strnlen);
    anchor!(STRCMP: FnStrcmp = velum_libk::vk_strcmp);
    anchor!(STRNCMP: FnStrncmp = velum_libk::vk_strncmp);
    anchor!(STRCHR: FnStrchr = velum_libk::vk_strchr);
    anchor!(STRRCHR: FnStrchr = velum_libk::vk_strrchr);
    anchor!(STRSTR: FnStrstr = velum_libk::vk_strstr);
    anchor!(STRLCPY: FnStrl = velum_libk::vk_strlcpy);
    anchor!(STRLCAT: FnStrl = velum_libk::vk_strlcat);
    anchor!(MEMCMP: FnMemcmp = velum_libk::vk_memcmp);
    anchor!(MEMCHR: FnMemchr = velum_libk::vk_memchr);
}

#[cfg(any(velum_kernel, velum_user))]
mod kern {
    use super::{
        FnMemchr, FnMemcmp, FnMemcpy, FnMemset, FnStrchr, FnStrcmp, FnStrl, FnStrlen, FnStrncmp,
        FnStrnlen, FnStrstr,
    };

    anchor!(STRLEN: FnStrlen = velum_libk::strlen);
    anchor!(STRNLEN: FnStrnlen = velum_libk::strnlen);
    anchor!(STRCMP: FnStrcmp = velum_libk::strcmp);
    anchor!(STRNCMP: FnStrncmp = velum_libk::strncmp);
    anchor!(STRCHR: FnStrchr = velum_libk::strchr);
    anchor!(STRRCHR: FnStrchr = velum_libk::strrchr);
    anchor!(STRSTR: FnStrstr = velum_libk::strstr);
    anchor!(STRLCPY: FnStrl = velum_libk::strlcpy);
    anchor!(STRLCAT: FnStrl = velum_libk::strlcat);
    anchor!(MEMCMP: FnMemcmp = velum_libk::memcmp);
    anchor!(MEMCHR: FnMemchr = velum_libk::memchr);
    anchor!(MEMCPY: FnMemcpy = velum_libk::memcpy);
    anchor!(MEMMOVE: FnMemcpy = velum_libk::memmove);
    anchor!(MEMSET: FnMemset = velum_libk::memset);
}
