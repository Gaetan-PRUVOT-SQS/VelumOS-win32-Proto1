use crate::geom::{box_clip, box_empty, box_of, max64};
use crate::px::{px_at, span_color, span_copy, store};
use crate::surface::clipbox;
use crate::{Color, Gradient, Surface};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Dda {
    pub m: i64,
    pub q: i64,
    pub r: i64,
    pub sq: i64,
    pub sr: i64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Ramp {
    pub den: i64,
    pub from: [i64; 4],
    pub delta: [i64; 4],
    pub c: [Dda; 4],
}

impl Dda {
    pub(crate) const fn zero() -> Self {
        Self {
            m: 0,
            q: 0,
            r: 0,
            sq: 0,
            sr: 0,
        }
    }
}

impl Ramp {
    pub(crate) const fn zero() -> Self {
        Self {
            den: 0,
            from: [0; 4],
            delta: [0; 4],
            c: [Dda::zero(); 4],
        }
    }
}

pub(crate) fn dda_make(d: &mut Dda, m: i64, step: i64) {
    *d = Dda::zero();
    d.m = m;
    if m <= 0 {
        return;
    }
    d.sq = step / m;
    d.sr = step % m;
    if d.sr < 0 {
        d.sr += m;
        d.sq -= 1;
    }
}

pub(crate) fn dda_seek(d: &mut Dda, num: i64) {
    if d.m == 0 {
        d.q = 0;
        d.r = 0;
        return;
    }
    d.q = num / d.m;
    d.r = num % d.m;
}

pub(crate) fn dda_next(d: &mut Dda) {
    if d.m <= 0 {
        return;
    }
    d.q += d.sq;
    d.r += d.sr;
    if d.r >= d.m {
        d.r -= d.m;
        d.q += 1;
    }
}

fn ramp_channel(rp: &mut Ramp, ch: usize, from: i64, to: i64) {
    rp.from[ch] = from;
    rp.delta[ch] = to - from;
    if rp.den == 0 {
        rp.delta[ch] = 0;
    }
    let m = 2 * max64(rp.den, 1);
    let step = 2 * rp.delta[ch];
    dda_make(&mut rp.c[ch], m, step);
}

pub(crate) fn ramp_init(rp: *mut Ramp, from: Color, to: Color, n: i64) {
    if rp.is_null() {
        return;
    }
    // SAFETY: rp non nul, la rampe reste valide le temps de l'appel.
    let rp = unsafe { &mut *rp };
    rp.den = max64(n.saturating_sub(1), 0);
    let mut ch = 0;
    while ch < 4 {
        let shift = 8 * ch;
        ramp_channel(
            rp,
            ch,
            i64::from((from >> shift) & 255),
            i64::from((to >> shift) & 255),
        );
        ch += 1;
    }
}

pub(crate) fn ramp_seek(rp: *mut Ramp, i: i64) {
    if rp.is_null() {
        return;
    }
    // SAFETY: rp non nul, initialisé par ramp_init ou à zéro.
    let rp = unsafe { &mut *rp };
    let den = max64(rp.den, 1);
    let mut ch = 0;
    while ch < 4 {
        let num = 2 * rp.from[ch] * den + 2 * rp.delta[ch] * i + den;
        dda_seek(&mut rp.c[ch], num);
        ch += 1;
    }
}

pub(crate) fn ramp_next(rp: *mut Ramp) {
    if rp.is_null() {
        return;
    }
    // SAFETY: rp non nul, la rampe reste valide le temps de l'appel.
    let rp = unsafe { &mut *rp };
    let mut ch = 0;
    while ch < 4 {
        dda_next(&mut rp.c[ch]);
        ch += 1;
    }
}

pub(crate) fn ramp_color(rp: &Ramp) -> Color {
    let q = |ch: usize| rp.c[ch].q as u32;
    (q(3) << 24) | (q(2) << 16) | (q(1) << 8) | q(0)
}

fn grad_row(p: *mut u32, n: usize, rp: &mut Ramp, first: i64) {
    ramp_seek(rp, first);
    let mut i = 0;
    while i < n {
        let pix = if p.is_null() {
            core::ptr::null_mut()
        } else {
            // SAFETY: i < n et p non nul, ligne dans la surface.
            unsafe { p.add(i) }
        };
        store(pix, ramp_color(rp));
        ramp_next(rp);
        i += 1;
    }
}

fn grad_horizontal(s: *mut Surface, g: &Gradient, b: crate::geom::PxBox) {
    let mut rp = Ramp::zero();
    ramp_init(&mut rp, g.from, g.to, i64::from(g.r.w));
    let opaque = g.from >> 24 == 255 && g.to >> 24 == 255;
    let n = (b.x1 - b.x0) as usize;
    let mut y = b.y0;
    while y < b.y1 {
        if opaque && y > b.y0 {
            span_copy(px_at(s, b.x0, y), px_at(s, b.x0, b.y0), n);
        } else {
            grad_row(px_at(s, b.x0, y), n, &mut rp, b.x0 - i64::from(g.r.x));
        }
        y += 1;
    }
}

fn grad_vertical(s: *mut Surface, g: &Gradient, b: crate::geom::PxBox) {
    let mut rp = Ramp::zero();
    ramp_init(&mut rp, g.from, g.to, i64::from(g.r.h));
    ramp_seek(&mut rp, b.y0 - i64::from(g.r.y));
    let n = (b.x1 - b.x0) as usize;
    let mut y = b.y0;
    while y < b.y1 {
        span_color(px_at(s, b.x0, y), n, ramp_color(&rp));
        ramp_next(&mut rp);
        y += 1;
    }
}

pub(crate) fn gradient(s: *mut Surface, g: *const Gradient) {
    if g.is_null() {
        return;
    }
    // SAFETY: g non nul, le dégradé reste valide le temps de l'appel.
    let g = unsafe { &*g };
    let b = box_clip(box_of(g.r), clipbox(s));
    if box_empty(b) {
        return;
    }
    if g.horizontal {
        grad_horizontal(s, g, b);
    } else {
        grad_vertical(s, g, b);
    }
}

#[cfg(test)]
mod tests {
    use super::{Dda, Ramp, ramp_init, ramp_next, ramp_seek};
    use core::mem::{offset_of, size_of};

    #[test]
    fn ramp_layout() {
        assert_eq!(size_of::<Dda>(), 40);
        assert_eq!(size_of::<Ramp>(), 232);
        assert_eq!(offset_of!(Dda, m), 0);
        assert_eq!(offset_of!(Dda, q), 8);
        assert_eq!(offset_of!(Dda, r), 16);
        assert_eq!(offset_of!(Dda, sq), 24);
        assert_eq!(offset_of!(Dda, sr), 32);
        assert_eq!(offset_of!(Ramp, den), 0);
        assert_eq!(offset_of!(Ramp, from), 8);
        assert_eq!(offset_of!(Ramp, delta), 40);
        assert_eq!(offset_of!(Ramp, c), 72);
    }

    #[test]
    fn ramp_step_matches_seek() {
        let mut stepped = Ramp::zero();
        let mut direct = Ramp::zero();
        ramp_init(&mut stepped, 0x1122_3344, 0xaabb_ccdd, 17);
        ramp_init(&mut direct, 0x1122_3344, 0xaabb_ccdd, 17);
        ramp_seek(&mut stepped, 0);
        for i in 0..17 {
            ramp_seek(&mut direct, i);
            for ch in 0..4 {
                assert_eq!(stepped.c[ch].q, direct.c[ch].q);
            }
            ramp_next(&mut stepped);
        }
    }

    #[test]
    fn ramp_width_i32_max() {
        let mut rp = Ramp::zero();
        ramp_init(&mut rp, 0, 0xff, 2_147_483_647);
        ramp_seek(&mut rp, 0);
        assert_eq!(rp.c[0].q, 0);
        ramp_seek(&mut rp, 2_147_483_646);
        assert_eq!(rp.c[0].q, 255);
        ramp_init(&mut rp, 0xff, 0, 2_147_483_647);
        ramp_seek(&mut rp, 0);
        assert_eq!(rp.c[0].q, 255);
        ramp_seek(&mut rp, 2_147_483_646);
        assert_eq!(rp.c[0].q, 0);
        ramp_init(&mut rp, 0x7f, 0x10, 1);
        ramp_seek(&mut rp, 0);
        assert_eq!(rp.c[0].q, 0x7f);
    }
}
