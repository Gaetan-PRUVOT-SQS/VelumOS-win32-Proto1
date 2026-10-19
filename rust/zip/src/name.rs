use core::ffi::c_char;

pub(crate) fn is_safe(name: *const c_char, len: usize) -> bool {
    if name.is_null() || len == 0 || len > isize::MAX as usize {
        return false;
    }
    // SAFETY: name non nul, len borné. Le contrat de zip_name_safe exige len octets.
    let bytes = unsafe { core::slice::from_raw_parts(name.cast::<u8>(), len) };
    safe_bytes(bytes)
}

fn safe_bytes(name: &[u8]) -> bool {
    if name.is_empty() || name[0] == b'/' {
        return false;
    }
    let mut start = 0usize;
    let mut i = 0usize;
    loop {
        if i < name.len() && (name[i] == 0 || name[i] == b'\\') {
            return false;
        }
        if i == name.len() || name[i] == b'/' {
            let seglen = i - start;
            if seglen == 2 && name[start] == b'.' && name[start + 1] == b'.' {
                return false;
            }
            if i == name.len() {
                return true;
            }
            start = i + 1;
        }
        i += 1;
    }
}
