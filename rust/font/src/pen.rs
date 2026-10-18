use crate::{Cut, Glyph, Pen, PenBox, Surface, TextReq};
use core::ffi::c_int;

fn clamp_to(value: i64, low: i32, high: i32) -> i32 {
    if value < i64::from(low) {
        return low;
    }
    if value > i64::from(high) {
        return high;
    }
    i32::try_from(value).unwrap_or(low)
}

fn add_i32(a: i32, b: i32) -> i64 {
    i64::from(a) + i64::from(b)
}

pub(crate) fn start(pen: *mut Pen, dst: *mut Surface, rq: *const TextReq) -> c_int {
    if pen.is_null() || dst.is_null() || rq.is_null() {
        return 0;
    }
    // SAFETY: dst non nul. On ne lit la requête qu'après ce contrôle.
    let surface = unsafe { &*dst };
    if surface.px.is_null() || surface.w <= 0 || surface.h <= 0 {
        return 0;
    }
    // SAFETY: rq non nul.
    let req = unsafe { &*rq };
    let clip = surface.clip;
    let bounds = PenBox {
        left: clamp_to(i64::from(clip.x), 0, surface.w),
        top: clamp_to(i64::from(clip.y), 0, surface.h),
        right: clamp_to(add_i32(clip.x, clip.w), 0, surface.w),
        bottom: clamp_to(add_i32(clip.y, clip.h), 0, surface.h),
    };
    let open = bounds.left < bounds.right && bounds.top < bounds.bottom;
    // SAFETY: pen non nul, on écrit toute la structure.
    unsafe {
        pen.write(Pen {
            dst,
            font: req.font,
            color: req.color,
            x: i64::from(req.at.x),
            y: i64::from(req.at.y),
            bounds,
        });
    }
    c_int::from(open)
}

pub(crate) fn cut(pen: *const Pen, g: *const Glyph, out: *mut Cut) -> c_int {
    if pen.is_null() || g.is_null() || out.is_null() {
        return 0;
    }
    // SAFETY: pen non nul.
    let pen = unsafe { &*pen };
    // SAFETY: g non nul.
    let g = unsafe { &*g };
    let Some(gx) = pen.x.checked_add(i64::from(g.xoff)) else {
        return 0;
    };
    let Some(gy) = pen.y.checked_add(i64::from(g.yoff)) else {
        return 0;
    };
    if gx >= i64::from(pen.bounds.right) || gy >= i64::from(pen.bounds.bottom) {
        return 0;
    }
    let Some(gx_end) = gx.checked_add(i64::from(g.w)) else {
        return 0;
    };
    let Some(gy_end) = gy.checked_add(i64::from(g.h)) else {
        return 0;
    };
    if gx_end <= i64::from(pen.bounds.left) || gy_end <= i64::from(pen.bounds.top) {
        return 0;
    }
    let Ok(x) = i32::try_from(gx) else {
        return 0;
    };
    let Ok(y) = i32::try_from(gy) else {
        return 0;
    };
    let Some(from_left) = i64::from(pen.bounds.left).checked_sub(gx) else {
        return 0;
    };
    let Some(from_right) = i64::from(pen.bounds.right).checked_sub(gx) else {
        return 0;
    };
    let Some(from_top) = i64::from(pen.bounds.top).checked_sub(gy) else {
        return 0;
    };
    let Some(from_bottom) = i64::from(pen.bounds.bottom).checked_sub(gy) else {
        return 0;
    };
    let w = i32::from(g.w);
    let h = i32::from(g.h);
    // SAFETY: out non nul. rows n'est pas modifié.
    unsafe {
        let cut = &mut *out;
        cut.x = x;
        cut.y = y;
        cut.c0 = clamp_to(from_left, 0, w);
        cut.c1 = clamp_to(from_right, 0, w);
        cut.r0 = clamp_to(from_top, 0, h);
        cut.r1 = clamp_to(from_bottom, 0, h);
    }
    1
}
