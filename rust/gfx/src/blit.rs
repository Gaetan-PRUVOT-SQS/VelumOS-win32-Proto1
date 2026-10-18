use crate::geom::{PxBox, box_clip, box_empty, box_make, box_of, max64, min64};
use crate::px::{px_at, span_blend, span_move};
use crate::ramp::{Dda, dda_make, dda_next, dda_seek};
use crate::surface::{bounds, clipbox};
use crate::{Blit, Point, Rect, Surface};

const SMOOTH_MAX: i32 = 65536;

struct Job {
    dst: *mut Surface,
    src: *const Surface,
    d: PxBox,
    ox: i64,
    oy: i64,
}

fn blit_of<'a>(b: *const Blit) -> Option<&'a Blit> {
    if b.is_null() {
        None
    } else {
        // SAFETY: b non nul, le blit reste valide le temps de l'appel.
        Some(unsafe { &*b })
    }
}

fn job_run(j: &Job, row: fn(*mut u32, *const u32, usize)) {
    let dst0 = px_at(j.dst, j.d.x0, j.d.y0);
    let src0 = px_at(j.src, j.d.x0.wrapping_add(j.ox), j.d.y0.wrapping_add(j.oy));
    let back = (dst0 as usize) > (src0 as usize);
    let (mut y, step) = if back { (j.d.y1 - 1, -1) } else { (j.d.y0, 1) };
    let mut left = j.d.y1 - j.d.y0;
    let n = (j.d.x1 - j.d.x0) as usize;
    while left > 0 {
        row(
            px_at(j.dst, j.d.x0, y),
            px_at(j.src, j.d.x0.wrapping_add(j.ox), y.wrapping_add(j.oy)),
            n,
        );
        y += step;
        left -= 1;
    }
}

fn blit_job(j: &mut Job, b: *const Blit) -> bool {
    let Some(b) = blit_of(b) else {
        return false;
    };
    let w = min64(i64::from(b.dr.w), i64::from(b.sr.w));
    let h = min64(i64::from(b.dr.h), i64::from(b.sr.h));
    if w <= 0 || h <= 0 {
        return false;
    }
    j.dst = b.dst;
    j.src = b.src;
    j.ox = i64::from(b.sr.x) - i64::from(b.dr.x);
    j.oy = i64::from(b.sr.y) - i64::from(b.dr.y);
    j.d = box_make(
        i64::from(b.dr.x),
        i64::from(b.dr.y),
        i64::from(b.dr.x) + w,
        i64::from(b.dr.y) + h,
    );
    j.d = box_clip(j.d, clipbox(b.dst));
    let s = bounds(b.src);
    j.d = box_clip(
        j.d,
        box_make(
            s.x0.wrapping_sub(j.ox),
            s.y0.wrapping_sub(j.oy),
            s.x1.wrapping_sub(j.ox),
            s.y1.wrapping_sub(j.oy),
        ),
    );
    !box_empty(j.d)
}

pub(crate) fn blit(b: *const Blit) {
    let mut j = Job {
        dst: core::ptr::null_mut(),
        src: core::ptr::null(),
        d: box_make(0, 0, 0, 0),
        ox: 0,
        oy: 0,
    };
    if blit_job(&mut j, b) {
        job_run(&j, span_move);
    }
}

pub(crate) fn blit_alpha(b: *const Blit) {
    let mut j = Job {
        dst: core::ptr::null_mut(),
        src: core::ptr::null(),
        d: box_make(0, 0, 0, 0),
        ox: 0,
        oy: 0,
    };
    if blit_job(&mut j, b) {
        job_run(&j, span_blend);
    }
}

pub(crate) fn scroll(s: *mut Surface, r: Rect, delta: Point) {
    let area = box_clip(box_of(r), bounds(s));
    let dx = i64::from(delta.x);
    let dy = i64::from(delta.y);
    let d = box_clip(
        area,
        box_make(
            area.x0.wrapping_add(dx),
            area.y0.wrapping_add(dy),
            area.x1.wrapping_add(dx),
            area.y1.wrapping_add(dy),
        ),
    );
    let d = box_clip(d, clipbox(s));
    if box_empty(d) {
        return;
    }
    let j = Job {
        dst: s,
        src: s,
        d,
        ox: dx.wrapping_neg(),
        oy: dy.wrapping_neg(),
    };
    job_run(&j, span_move);
}

fn scale_index(pos: i64, origin: i64, src_n: i64, dst_n: i64) -> Option<i64> {
    let twice = dst_n.checked_mul(2)?;
    if twice == 0 {
        return None;
    }
    let t = pos.checked_sub(origin)?.checked_mul(2)?.checked_add(1)?;
    let prod = i128::from(t).checked_mul(i128::from(src_n))?;
    i64::try_from(prod / i128::from(twice)).ok()
}

fn scale_num(pos: i64, origin: i64, src_w: i64) -> Option<i64> {
    let t = pos.checked_sub(origin)?.checked_mul(2)?.checked_add(1)?;
    i64::try_from(i128::from(t).checked_mul(i128::from(src_w))?).ok()
}

fn scale_row(b: &Blit, d: &PxBox, x: &mut Dda, y: i64) {
    let src = bounds(b.src);
    if box_empty(src) {
        return;
    }
    let Some(off) = scale_index(y, i64::from(b.dr.y), i64::from(b.sr.h), i64::from(b.dr.h)) else {
        return;
    };
    let sy = i64::from(b.sr.y) + off;
    if sy < 0 || sy >= src.y1 {
        return;
    }
    let Some(num) = scale_num(d.x0, i64::from(b.dr.x), i64::from(b.sr.w)) else {
        return;
    };
    dda_seek(x, num);
    let mut px = d.x0;
    while px < d.x1 {
        let sx = i64::from(b.sr.x) + x.q;
        if sx >= 0 && sx < src.x1 {
            let dstp = px_at(b.dst, px, y);
            let srcp = px_at(b.src, sx, sy);
            if !dstp.is_null() && !srcp.is_null() {
                // SAFETY: les deux pixels sont dans leur surface.
                unsafe { dstp.write(srcp.read()) };
            }
        }
        dda_next(x);
        px += 1;
    }
}

pub(crate) fn blit_scaled(b: *const Blit) {
    let Some(b) = blit_of(b) else {
        return;
    };
    if b.dr.w <= 0 || b.dr.h <= 0 || b.sr.w <= 0 || b.sr.h <= 0 || box_empty(bounds(b.src)) {
        return;
    }
    let d = box_clip(box_of(b.dr), clipbox(b.dst));
    if box_empty(d) {
        return;
    }
    let mut x = Dda::zero();
    dda_make(&mut x, i64::from(b.dr.w) * 2, i64::from(b.sr.w) * 2);
    let mut y = d.y0;
    while y < d.y1 {
        scale_row(b, &d, &mut x, y);
        y += 1;
    }
}

struct Tap {
    i0: i64,
    i1: i64,
    frac: u32,
}

fn tap_axis(pos256: i64, lo: i64, hi: i64) -> Tap {
    let mut whole = pos256 / 256;
    if pos256 % 256 < 0 {
        whole -= 1;
    }
    let frac = (pos256 - whole * 256) as u32;
    Tap {
        i0: max64(lo, min64(hi, whole)),
        i1: max64(lo, min64(hi, whole + 1)),
        frac,
    }
}

fn smooth_weights(fx: u32, fy: u32) -> [u32; 4] {
    let a = 256u32.wrapping_sub(fx);
    let b = 256u32.wrapping_sub(fy);
    [
        a.wrapping_mul(b),
        fx.wrapping_mul(b),
        a.wrapping_mul(fy),
        fx.wrapping_mul(fy),
    ]
}

fn bilinear(p: [u32; 4], w: [u32; 4]) -> u32 {
    let mut out = 0u32;
    let mut ch = 0;
    while ch < 4 {
        let mut v = 32768u32;
        let mut k = 0;
        while k < 4 {
            v = v.wrapping_add(((p[k] >> (8 * ch)) & 255).wrapping_mul(w[k]));
            k += 1;
        }
        out |= (v >> 16) << (8 * ch);
        ch += 1;
    }
    out
}

fn smooth_pixel(b: &Blit, tx: &Tap, ty: &Tap) -> u32 {
    let coords = [
        (tx.i0, ty.i0),
        (tx.i1, ty.i0),
        (tx.i0, ty.i1),
        (tx.i1, ty.i1),
    ];
    let mut p = [0u32; 4];
    let mut k = 0;
    while k < 4 {
        let ptr = px_at(b.src, coords[k].0, coords[k].1);
        if ptr.is_null() {
            return 0;
        }
        // SAFETY: le tap est borné à la surface source.
        p[k] = unsafe { ptr.read() };
        k += 1;
    }
    bilinear(p, smooth_weights(tx.frac, ty.frac))
}

fn smooth_axis(pos: i64, origin: i64, src_n: i64, dst_n: i64, src_origin: i64) -> Option<i64> {
    if dst_n == 0 {
        return None;
    }
    let t = pos.checked_sub(origin)?.checked_mul(2)?.checked_add(1)?;
    let prod = i128::from(t)
        .checked_mul(i128::from(src_n))?
        .checked_mul(128)?;
    let q = i64::try_from(prod / i128::from(dst_n)).ok()?;
    q.checked_sub(128)?
        .checked_add(src_origin.checked_mul(256)?)
}

fn smooth_x_num(pos: i64, origin: i64, src_w: i64) -> Option<i64> {
    let t = pos.checked_sub(origin)?.checked_mul(2)?.checked_add(1)?;
    i64::try_from(
        i128::from(t)
            .checked_mul(i128::from(src_w))?
            .checked_mul(128)?,
    )
    .ok()
}

fn smooth_row(b: &Blit, d: &PxBox, y: i64, x: &mut Dda, range: PxBox) {
    let Some(py) = smooth_axis(
        y,
        i64::from(b.dr.y),
        i64::from(b.sr.h),
        i64::from(b.dr.h),
        i64::from(b.sr.y),
    ) else {
        return;
    };
    let ty = tap_axis(py, range.y0, range.y1 - 1);
    let Some(num) = smooth_x_num(d.x0, i64::from(b.dr.x), i64::from(b.sr.w)) else {
        return;
    };
    dda_seek(x, num);
    let mut px = d.x0;
    while px < d.x1 {
        let pos =
            x.q.wrapping_sub(128)
                .wrapping_add(i64::from(b.sr.x).wrapping_mul(256));
        let tx = tap_axis(pos, range.x0, range.x1 - 1);
        let dstp = px_at(b.dst, px, y);
        if !dstp.is_null() {
            // SAFETY: px est dans le clip de destination.
            unsafe { dstp.write(smooth_pixel(b, &tx, &ty)) };
        }
        dda_next(x);
        px += 1;
    }
}

pub(crate) fn blit_smooth(b: *const Blit) {
    let Some(view) = blit_of(b) else {
        return;
    };
    if view.dr.w <= 0
        || view.dr.h <= 0
        || view.sr.w <= 0
        || view.sr.h <= 0
        || box_empty(bounds(view.src))
    {
        return;
    }
    if view.sr.w > SMOOTH_MAX || view.sr.h > SMOOTH_MAX {
        blit_scaled(b);
        return;
    }
    let d = box_clip(box_of(view.dr), clipbox(view.dst));
    let range = box_clip(box_of(view.sr), bounds(view.src));
    if box_empty(d) || box_empty(range) {
        return;
    }
    let mut x = Dda::zero();
    dda_make(&mut x, i64::from(view.dr.w), 256 * i64::from(view.sr.w));
    let mut y = d.y0;
    while y < d.y1 {
        smooth_row(view, &d, y, &mut x, range);
        y += 1;
    }
}
