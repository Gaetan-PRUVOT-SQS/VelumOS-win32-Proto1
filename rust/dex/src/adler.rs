pub(crate) fn adler32(p: *const u8, n: usize) -> u32 {
    if n == 0 {
        return 1;
    }
    if p.is_null() {
        return 0;
    }
    let mut a = 1u32;
    let mut b = 0u32;
    let mut i = 0usize;
    while i < n {
        let mut stop = n;
        if n - i > 5552 {
            stop = i + 5552;
        }
        while i < stop {
            if i > isize::MAX as usize {
                return 0;
            }
            // SAFETY: i < n et i tient dans isize, p non nul. L'appelant garantit n octets.
            let byte = unsafe { *p.add(i) };
            a = a.wrapping_add(u32::from(byte));
            b = b.wrapping_add(a);
            i += 1;
        }
        a %= 65521;
        b %= 65521;
    }
    (b << 16) | a
}
