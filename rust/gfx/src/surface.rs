use crate::geom::{box_clip, box_empty, box_make, box_of, box_rect, rect_intersect, rect_make};
use crate::px::px_at;
use crate::{Color, Point, Rect, Surface};

pub(crate) fn cleared() -> Surface {
    Surface {
        px: core::ptr::null_mut(),
        w: 0,
        h: 0,
        stride: 0,
        clip: Rect::zero(),
    }
}

fn mut_ref<'a>(s: *mut Surface) -> Option<&'a mut Surface> {
    if s.is_null() {
        None
    } else {
        // SAFETY: s non nul, l'appelant garantit une surface valide pendant l'appel.
        Some(unsafe { &mut *s })
    }
}

fn ref_of<'a>(s: *const Surface) -> Option<&'a Surface> {
    if s.is_null() {
        None
    } else {
        // SAFETY: s non nul, l'appelant garantit une surface valide pendant l'appel.
        Some(unsafe { &*s })
    }
}

fn surface_ok(s: &Surface) -> bool {
    !s.px.is_null() && s.w > 0 && s.h > 0 && s.stride >= s.w
}

pub(crate) fn bounds(s: *const Surface) -> crate::geom::PxBox {
    let Some(s) = ref_of(s) else {
        return box_make(0, 0, 0, 0);
    };
    if !surface_ok(s) {
        return box_make(0, 0, 0, 0);
    }
    box_make(0, 0, i64::from(s.w), i64::from(s.h))
}

pub(crate) fn clipbox(s: *const Surface) -> crate::geom::PxBox {
    let Some(s) = ref_of(s) else {
        return box_make(0, 0, 0, 0);
    };
    if !surface_ok(s) {
        return box_make(0, 0, 0, 0);
    }
    box_clip(box_of(s.clip), bounds(s))
}

pub(crate) fn init(s: *mut Surface, px: *mut u32, w: i32, h: i32) {
    let Some(s) = mut_ref(s) else {
        return;
    };
    *s = cleared();
    if px.is_null() || w <= 0 || h <= 0 {
        return;
    }
    s.px = px;
    s.w = w;
    s.h = h;
    s.stride = w;
    s.clip = rect_make(0, 0, w, h);
}

pub(crate) fn set_clip(s: *mut Surface, r: Rect) {
    let Some(s) = mut_ref(s) else {
        return;
    };
    s.clip = rect_intersect(r, rect_make(0, 0, s.w, s.h));
}

pub(crate) fn sub(s: *const Surface, r: Rect) -> Surface {
    let mut out = cleared();
    let b = box_clip(bounds(s), box_of(r));
    if box_empty(b) {
        return out;
    }
    let Some(src) = ref_of(s) else {
        return out;
    };
    let c = box_clip(clipbox(s), b);
    out.px = px_at(s, b.x0, b.y0);
    out.w = (b.x1 - b.x0) as i32;
    out.h = (b.y1 - b.y0) as i32;
    out.stride = src.stride;
    out.clip = box_rect(box_make(c.x0 - b.x0, c.y0 - b.y0, c.x1 - b.x0, c.y1 - b.y0));
    out
}

pub(crate) fn put(s: *mut Surface, p: Point, c: Color) {
    use crate::px::store;
    let k = clipbox(s);
    if i64::from(p.x) < k.x0
        || i64::from(p.x) >= k.x1
        || i64::from(p.y) < k.y0
        || i64::from(p.y) >= k.y1
    {
        return;
    }
    store(px_at(s, i64::from(p.x), i64::from(p.y)), c);
}

pub(crate) fn get(s: *const Surface, p: Point) -> Color {
    let k = bounds(s);
    if i64::from(p.x) < k.x0
        || i64::from(p.x) >= k.x1
        || i64::from(p.y) < k.y0
        || i64::from(p.y) >= k.y1
    {
        return 0;
    }
    let ptr = px_at(s, i64::from(p.x), i64::from(p.y));
    if ptr.is_null() {
        return 0;
    }
    // SAFETY: le point est dans les bornes, px est non nul.
    unsafe { ptr.read() }
}
