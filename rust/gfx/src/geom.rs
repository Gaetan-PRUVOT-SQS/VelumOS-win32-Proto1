use crate::{Point, Rect};

#[derive(Clone, Copy)]
pub(crate) struct PxBox {
    pub x0: i64,
    pub y0: i64,
    pub x1: i64,
    pub y1: i64,
}

pub(crate) const fn box_make(x0: i64, y0: i64, x1: i64, y1: i64) -> PxBox {
    PxBox { x0, y0, x1, y1 }
}

pub(crate) fn max64(a: i64, b: i64) -> i64 {
    if a > b { a } else { b }
}

pub(crate) fn min64(a: i64, b: i64) -> i64 {
    if a < b { a } else { b }
}

pub(crate) fn abs64(a: i64) -> i64 {
    if a < 0 { a.saturating_neg() } else { a }
}

pub(crate) fn box_of(r: Rect) -> PxBox {
    if r.w <= 0 || r.h <= 0 {
        return box_make(0, 0, 0, 0);
    }
    box_make(
        i64::from(r.x),
        i64::from(r.y),
        i64::from(r.x) + i64::from(r.w),
        i64::from(r.y) + i64::from(r.h),
    )
}

pub(crate) fn box_clip(a: PxBox, b: PxBox) -> PxBox {
    box_make(
        max64(a.x0, b.x0),
        max64(a.y0, b.y0),
        min64(a.x1, b.x1),
        min64(a.y1, b.y1),
    )
}

pub(crate) fn box_empty(b: PxBox) -> bool {
    b.x1 <= b.x0 || b.y1 <= b.y0
}

fn trunc_i32(v: i64) -> i32 {
    v as i32
}

pub(crate) fn box_rect(b: PxBox) -> Rect {
    if box_empty(b) {
        return Rect::zero();
    }
    let w = min64(b.x1 - b.x0, i64::from(i32::MAX));
    let h = min64(b.y1 - b.y0, i64::from(i32::MAX));
    Rect {
        x: trunc_i32(b.x0),
        y: trunc_i32(b.y0),
        w: trunc_i32(w),
        h: trunc_i32(h),
    }
}

pub(crate) const fn rect_make(x: i32, y: i32, w: i32, h: i32) -> Rect {
    Rect { x, y, w, h }
}

pub(crate) fn rect_intersect(a: Rect, b: Rect) -> Rect {
    box_rect(box_clip(box_of(a), box_of(b)))
}

pub(crate) fn rect_union(a: Rect, b: Rect) -> Rect {
    if rect_empty(a) && rect_empty(b) {
        return Rect::zero();
    }
    if rect_empty(a) {
        return b;
    }
    if rect_empty(b) {
        return a;
    }
    let p = box_of(a);
    let q = box_of(b);
    box_rect(box_make(
        min64(p.x0, q.x0),
        min64(p.y0, q.y0),
        max64(p.x1, q.x1),
        max64(p.y1, q.y1),
    ))
}

pub(crate) fn rect_empty(r: Rect) -> bool {
    r.w <= 0 || r.h <= 0
}

pub(crate) fn rect_contains(r: Rect, p: Point) -> bool {
    let b = box_of(r);
    let x = i64::from(p.x);
    let y = i64::from(p.y);
    x >= b.x0 && x < b.x1 && y >= b.y0 && y < b.y1
}
