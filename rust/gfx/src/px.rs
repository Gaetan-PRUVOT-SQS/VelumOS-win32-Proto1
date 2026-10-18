use crate::color::blend;
use crate::{Color, Surface};

pub(crate) fn px_at(s: *const Surface, x: i64, y: i64) -> *mut u32 {
    if s.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: s non nul, la surface reste valide le temps de l'appel.
    let surf = unsafe { &*s };
    if surf.px.is_null() {
        return core::ptr::null_mut();
    }
    let off = (y as u64)
        .wrapping_mul(surf.stride as u64)
        .wrapping_add(x as u64);
    let Some(bytes) = off.checked_mul(4) else {
        return core::ptr::null_mut();
    };
    if bytes > isize::MAX as u64 {
        return core::ptr::null_mut();
    }
    // SAFETY: l'offset en octets tient dans isize. L'appelant ne déréférence que dans la surface.
    unsafe { surf.px.add(off as usize) }
}

pub(crate) fn store(p: *mut u32, c: Color) {
    if p.is_null() {
        return;
    }
    let a = c >> 24;
    if a == 255 {
        // SAFETY: p non nul, un pixel de la surface.
        unsafe { p.write(c) };
    } else if a != 0 {
        // SAFETY: p non nul, lecture puis écriture du même pixel.
        unsafe {
            let dst = p.read();
            p.write(blend(dst, c));
        }
    }
}

pub(crate) fn span_fill(d: *mut u32, n: usize, c: Color) {
    if d.is_null() || n > (isize::MAX as usize) / 4 {
        return;
    }
    let mut i = 0;
    while i < n {
        // SAFETY: i < n et d pointe au moins n pixels.
        unsafe { d.add(i).write(c) };
        i += 1;
    }
}

pub(crate) fn span_copy(d: *mut u32, s: *const u32, n: usize) {
    if n == 0 || d.is_null() || s.is_null() || n > (isize::MAX as usize) / 4 {
        return;
    }
    // SAFETY: pas de recouvrement, n pixels valides des deux côtés.
    unsafe { core::ptr::copy_nonoverlapping(s, d, n) };
}

fn span_overlap(d: *const u32, s: *const u32, n: usize) -> bool {
    let bytes = n.wrapping_mul(core::mem::size_of::<u32>());
    let a = d as usize;
    let b = s as usize;
    a < b.wrapping_add(bytes) && b < a.wrapping_add(bytes)
}

pub(crate) fn span_move(d: *mut u32, s: *const u32, n: usize) {
    if n == 0 || d.is_null() || s.is_null() || n > (isize::MAX as usize) / 4 {
        return;
    }
    if !span_overlap(d, s, n) {
        // SAFETY: pas de recouvrement, n pixels valides.
        unsafe { core::ptr::copy_nonoverlapping(s, d, n) };
        return;
    }
    if (d as usize) < (s as usize) {
        let mut i = 0;
        while i < n {
            // SAFETY: i < n, les deux plages font n pixels.
            unsafe { d.add(i).write(s.add(i).read()) };
            i += 1;
        }
    } else {
        let mut i = n;
        while i > 0 {
            i -= 1;
            // SAFETY: i < n, copie arrière pour un recouvrement.
            unsafe { d.add(i).write(s.add(i).read()) };
        }
    }
}

pub(crate) fn span_blend(d: *mut u32, s: *const u32, n: usize) {
    if n == 0 || d.is_null() || s.is_null() || n > (isize::MAX as usize) / 4 {
        return;
    }
    if (d as usize) > (s as usize) && span_overlap(d, s, n) {
        let mut i = n;
        while i > 0 {
            i -= 1;
            // SAFETY: i < n, pixel source valide.
            let px = unsafe { s.add(i).read() };
            // SAFETY: i < n, pixel destination valide.
            store(unsafe { d.add(i) }, px);
        }
        return;
    }
    let mut i = 0;
    while i < n {
        // SAFETY: i < n, pixel source valide.
        let px = unsafe { s.add(i).read() };
        // SAFETY: i < n, pixel destination valide.
        store(unsafe { d.add(i) }, px);
        i += 1;
    }
}

pub(crate) fn span_color(d: *mut u32, n: usize, c: Color) {
    let a = c >> 24;
    if a == 255 {
        span_fill(d, n, c);
    } else if a != 0 {
        let mut i = 0;
        while i < n {
            let p = if d.is_null() {
                core::ptr::null_mut()
            } else {
                // SAFETY: i < n, d non nul. Le refus si n est absurde est dans span_fill ; ici n vient d'un clip.
                unsafe { d.add(i) }
            };
            store(p, c);
            i += 1;
        }
    }
}
