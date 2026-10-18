use crate::t_rect;

#[derive(Clone, Copy)]
struct PxBox {
    x0: i64,
    y0: i64,
    x1: i64,
    y1: i64,
}

fn max64(a: i64, b: i64) -> i64 {
    if a > b { a } else { b }
}

fn min64(a: i64, b: i64) -> i64 {
    if a < b { a } else { b }
}

fn box_of(r: t_rect) -> PxBox {
    if r.w <= 0 || r.h <= 0 {
        return PxBox {
            x0: 0,
            y0: 0,
            x1: 0,
            y1: 0,
        };
    }
    PxBox {
        x0: i64::from(r.x),
        y0: i64::from(r.y),
        x1: i64::from(r.x) + i64::from(r.w),
        y1: i64::from(r.y) + i64::from(r.h),
    }
}

fn box_clip(a: PxBox, b: PxBox) -> PxBox {
    PxBox {
        x0: max64(a.x0, b.x0),
        y0: max64(a.y0, b.y0),
        x1: min64(a.x1, b.x1),
        y1: min64(a.y1, b.y1),
    }
}

fn box_empty(b: PxBox) -> bool {
    b.x1 <= b.x0 || b.y1 <= b.y0
}

fn span(a: i64, b: i64) -> i64 {
    match b.checked_sub(a) {
        Some(v) => min64(v, i64::from(i32::MAX)),
        None => i64::from(i32::MAX),
    }
}

fn box_rect(b: PxBox) -> t_rect {
    if box_empty(b) {
        return t_rect::ZERO;
    }
    t_rect {
        x: b.x0 as i32,
        y: b.y0 as i32,
        w: span(b.x0, b.x1) as i32,
        h: span(b.y0, b.y1) as i32,
    }
}

pub(crate) fn rect_make(x: i32, y: i32, w: i32, h: i32) -> t_rect {
    t_rect { x, y, w, h }
}

pub(crate) fn rect_intersect(a: t_rect, b: t_rect) -> t_rect {
    box_rect(box_clip(box_of(a), box_of(b)))
}

pub(crate) fn rect_empty(r: t_rect) -> bool {
    r.w <= 0 || r.h <= 0
}
