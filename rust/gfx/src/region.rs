use crate::geom::{
    box_clip, box_empty, box_make, box_of, box_rect, rect_empty, rect_intersect, rect_make,
    rect_union,
};
use crate::{Rect, Region};

const REGION_MAX: u32 = 32;
const REGION_LIMIT: i32 = 1_073_741_823;

fn mut_rg<'a>(rg: *mut Region) -> Option<&'a mut Region> {
    if rg.is_null() {
        None
    } else {
        // SAFETY: rg non nul, la région reste valide le temps de l'appel.
        Some(unsafe { &mut *rg })
    }
}

fn ref_rg<'a>(rg: *const Region) -> Option<&'a Region> {
    if rg.is_null() {
        None
    } else {
        // SAFETY: rg non nul, la région reste valide le temps de l'appel.
        Some(unsafe { &*rg })
    }
}

fn region_check(rg: &mut Region) {
    if rg.n > REGION_MAX {
        rg.n = REGION_MAX;
    }
}

pub(crate) fn clear(rg: *mut Region) {
    if let Some(rg) = mut_rg(rg) {
        rg.n = 0;
    }
}

fn bounds_of(rg: &Region) -> Rect {
    let mut b = Rect::zero();
    let mut i = 0u32;
    let n = if rg.n < REGION_MAX { rg.n } else { REGION_MAX };
    while i < n {
        b = rect_union(b, rg.r[i as usize]);
        i += 1;
    }
    b
}

pub(crate) fn bounds(rg: *const Region) -> Rect {
    ref_rg(rg).map(bounds_of).unwrap_or_else(Rect::zero)
}

pub(crate) fn is_empty(rg: *const Region) -> bool {
    ref_rg(rg).is_none_or(|rg| rg.n == 0)
}

fn piece_add(out: &mut [Rect; 4], n: i32, b: crate::geom::PxBox) -> i32 {
    if box_empty(b) {
        return n;
    }
    out[n as usize] = box_rect(b);
    n + 1
}

fn rect_split(a: Rect, cut: Rect, out: &mut [Rect; 4]) -> i32 {
    let p = box_of(a);
    let c = box_clip(p, box_of(cut));
    if box_empty(c) {
        return -1;
    }
    let mut n = 0;
    n = piece_add(out, n, box_make(p.x0, p.y0, p.x1, c.y0));
    n = piece_add(out, n, box_make(p.x0, c.y1, p.x1, p.y1));
    n = piece_add(out, n, box_make(p.x0, c.y0, c.x0, c.y1));
    piece_add(out, n, box_make(c.x1, c.y0, p.x1, c.y1))
}

fn region_cut(rg: &mut Region, cut: Rect) -> bool {
    let start = rg.n;
    for i in (0..start).rev() {
        let mut piece = [Rect::zero(); 4];
        let k = rect_split(rg.r[i as usize], cut, &mut piece);
        if k < 0 {
            continue;
        }
        rg.n -= 1;
        rg.r[i as usize] = rg.r[rg.n as usize];
        if rg.n + u32::try_from(k).unwrap_or(u32::MAX) > REGION_MAX {
            return false;
        }
        let mut j = 0;
        while j < k {
            rg.r[rg.n as usize] = piece[j as usize];
            rg.n += 1;
            j += 1;
        }
    }
    true
}

fn rect_join(a: Rect, b: Rect) -> Option<Rect> {
    if rect_empty(a) || rect_empty(b) {
        return None;
    }
    let p = box_of(a);
    let q = box_of(b);
    let u = box_make(
        crate::geom::min64(p.x0, q.x0),
        crate::geom::min64(p.y0, q.y0),
        crate::geom::max64(p.x1, q.x1),
        crate::geom::max64(p.y1, q.y1),
    );
    if u.x1 - u.x0 > i64::from(i32::MAX) || u.y1 - u.y0 > i64::from(i32::MAX) {
        return None;
    }
    let horiz = p.y0 == q.y0 && p.y1 == q.y1 && (p.x1 == q.x0 || q.x1 == p.x0);
    let vert = p.x0 == q.x0 && p.x1 == q.x1 && (p.y1 == q.y0 || q.y1 == p.y0);
    if horiz || vert {
        Some(box_rect(u))
    } else {
        None
    }
}

fn merge_pair(rg: &mut Region) -> bool {
    let mut i = 0u32;
    while i < rg.n {
        let mut j = i + 1;
        while j < rg.n {
            if let Some(u) = rect_join(rg.r[i as usize], rg.r[j as usize]) {
                rg.r[i as usize] = u;
                rg.n -= 1;
                rg.r[j as usize] = rg.r[rg.n as usize];
                return true;
            }
            j += 1;
        }
        i += 1;
    }
    false
}

fn region_merge(rg: &mut Region) {
    while merge_pair(rg) {}
}

fn region_pieces(w: &mut Region, rg: &Region) -> bool {
    let mut i = 0u32;
    while i < rg.n && w.n > 0 {
        if !region_cut(w, rg.r[i as usize]) {
            return false;
        }
        i += 1;
    }
    true
}

fn region_append(rg: &mut Region, w: &Region) -> bool {
    let mut i = 0u32;
    while i < w.n {
        if rg.n >= REGION_MAX {
            region_merge(rg);
        }
        if rg.n >= REGION_MAX {
            return false;
        }
        rg.r[rg.n as usize] = w.r[i as usize];
        rg.n += 1;
        i += 1;
    }
    region_merge(rg);
    true
}

fn limit_rect() -> Rect {
    rect_make(
        -REGION_LIMIT,
        -REGION_LIMIT,
        REGION_LIMIT.saturating_mul(2),
        REGION_LIMIT.saturating_mul(2),
    )
}

pub(crate) fn add(rg: *mut Region, mut r: Rect) {
    let Some(rg) = mut_rg(rg) else {
        return;
    };
    region_check(rg);
    r = rect_intersect(r, limit_rect());
    if rect_empty(r) {
        return;
    }
    let all = rect_union(bounds_of(rg), r);
    let mut w = Region::zero();
    w.n = 1;
    w.r[0] = r;
    if region_pieces(&mut w, rg) && region_append(rg, &w) {
        return;
    }
    rg.r[0] = all;
    rg.n = 1;
}

pub(crate) fn subtract(rg: *mut Region, r: Rect) {
    let Some(rg) = mut_rg(rg) else {
        return;
    };
    region_check(rg);
    if rect_empty(r) || rg.n == 0 {
        return;
    }
    let all = bounds_of(rg);
    if !region_cut(rg, r) {
        rg.r[0] = all;
        rg.n = 1;
        return;
    }
    region_merge(rg);
}

pub(crate) fn intersect(rg: *mut Region, r: Rect) {
    let Some(rg) = mut_rg(rg) else {
        return;
    };
    region_check(rg);
    let mut i = 0u32;
    let mut kept = 0u32;
    while i < rg.n {
        let c = rect_intersect(rg.r[i as usize], r);
        if !rect_empty(c) {
            rg.r[kept as usize] = c;
            kept += 1;
        }
        i += 1;
    }
    rg.n = kept;
    region_merge(rg);
}
