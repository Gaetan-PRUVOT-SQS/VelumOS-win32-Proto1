use crate::{Cut, Font, Glyph, Pen, Point, Surface, TextReq, glyph, pen, text, utf8};
use core::ffi::c_int;

#[cfg(not(test))]
mod gfx_link {
    use super::{Point, Surface};

    // SAFETY: gfx_put est fourni au lien par la bibliothèque gfx. Signature de include/velum/gfx.h.
    unsafe extern "C" {
        pub fn gfx_put(dst: *mut Surface, at: Point, color: u32);
    }
}

fn put(dst: *mut Surface, at: Point, color: u32) {
    // SAFETY: dst vient de pen_start, non nul. Le point est celui du pixel allumé.
    unsafe {
        #[cfg(test)]
        crate::tests::gfx_put(dst, at, color);
        #[cfg(not(test))]
        gfx_link::gfx_put(dst, at, color);
    }
}

pub(crate) fn rows(f: *const Font, g: *const Glyph, rows: *mut *const u8) -> c_int {
    if f.is_null() || g.is_null() || rows.is_null() {
        return 0;
    }
    // SAFETY: f non nul, police vivante le temps de l'appel.
    let font = unsafe { &*f };
    // SAFETY: g non nul, glyphe de cette police.
    let glyph = unsafe { &*g };
    if font.bits.is_null() || glyph.w == 0 || glyph.h == 0 {
        return 0;
    }
    let stride = u32::from(glyph.w).div_ceil(8);
    let Some(need) = u32::from(glyph.h).checked_mul(stride) else {
        return 0;
    };
    let off = u32::from(glyph.off);
    if off > font.nbits {
        return 0;
    }
    let Some(room) = font.nbits.checked_sub(off) else {
        return 0;
    };
    if need > room {
        return 0;
    }
    let Ok(index) = usize::try_from(off) else {
        return 0;
    };
    // SAFETY: off + need <= nbits, bits + off reste dans la table.
    unsafe { *rows = font.bits.add(index) };
    1
}

fn paint_row(pen: &Pen, g: &Glyph, cut: &Cut, row: i32) {
    let stride = usize::from(g.w).div_ceil(8);
    let Ok(row_u) = usize::try_from(row) else {
        return;
    };
    let Some(base) = row_u.checked_mul(stride) else {
        return;
    };
    // SAFETY: base + stride tient dans la bitmap validée par rows().
    let line = unsafe { cut.rows.add(base) };
    let mut col = cut.c0;
    while col < cut.c1 {
        let Ok(col_u) = usize::try_from(col) else {
            break;
        };
        let Some(byte_i) = col_u.checked_div(8) else {
            break;
        };
        // SAFETY: col < c1 <= w, donc byte_i < stride.
        let byte = unsafe { line.add(byte_i).read() };
        let Ok(low) = u32::try_from(col & 7) else {
            break;
        };
        let bit = 7 - low;
        if (u32::from(byte) >> bit) & 1 == 1
            && let (Some(x), Some(y)) = (cut.x.checked_add(col), cut.y.checked_add(row))
        {
            put(pen.dst, Point { x, y }, pen.color);
        }
        let Some(next) = col.checked_add(1) else {
            break;
        };
        col = next;
    }
}

fn draw_glyph(pen: &Pen, g: *const Glyph) {
    let mut bitmap: *const u8 = core::ptr::null();
    if rows(pen.font, g, &mut bitmap) == 0 {
        return;
    }
    let mut cut = Cut {
        rows: bitmap,
        x: 0,
        y: 0,
        c0: 0,
        c1: 0,
        r0: 0,
        r1: 0,
    };
    if pen::cut(pen, g, &mut cut) == 0 {
        return;
    }
    // SAFETY: g non nul, glyphe déjà accepté par rows().
    let glyph = unsafe { &*g };
    let mut row = cut.r0;
    while row < cut.r1 {
        paint_row(pen, glyph, &cut, row);
        let Some(next) = row.checked_add(1) else {
            break;
        };
        row = next;
    }
}

pub(crate) fn draw(dst: *mut Surface, rq: *const TextReq) {
    if dst.is_null() || rq.is_null() {
        return;
    }
    // SAFETY: rq non nul.
    let req = unsafe { &*rq };
    if req.font.is_null() || req.text.is_null() {
        return;
    }
    let mut slot = core::mem::MaybeUninit::<Pen>::uninit();
    if pen::start(slot.as_mut_ptr(), dst, rq) == 0 {
        return;
    }
    // SAFETY: pen_start a écrit tout le stylo avant de rendre une valeur non nulle.
    let mut pen = unsafe { slot.assume_init() };
    let mut text = req.text;
    let stop = text::end(text, req.len);
    while text < stop {
        let Some(mark) = pen.x.checked_sub(128) else {
            break;
        };
        if mark >= i64::from(pen.bounds.right) {
            break;
        }
        let prev = text;
        let cp = utf8::next(&mut text, stop);
        if text == prev {
            break;
        }
        let g = glyph::glyph(req.font, cp);
        if g.is_null() {
            continue;
        }
        draw_glyph(&pen, g);
        // SAFETY: g non nul.
        let adv = i64::from(unsafe { (*g).advance });
        let Some(next_x) = pen.x.checked_add(adv) else {
            break;
        };
        pen.x = next_x;
    }
}
