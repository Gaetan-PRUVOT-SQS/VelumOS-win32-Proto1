#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod blit;
mod color;
mod draw;
mod geom;
mod px;
mod ramp;
mod region;
mod rrect;
mod selftest;
mod surface;

pub use ramp::{Dda, Ramp};

pub type Color = u32;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Point {
    pub x: i32,
    pub y: i32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Rect {
    pub x: i32,
    pub y: i32,
    pub w: i32,
    pub h: i32,
}

impl Rect {
    pub const fn zero() -> Self {
        Self {
            x: 0,
            y: 0,
            w: 0,
            h: 0,
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Surface {
    pub px: *mut u32,
    pub w: i32,
    pub h: i32,
    pub stride: i32,
    pub clip: Rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Region {
    pub r: [Rect; 32],
    pub n: u32,
}

impl Region {
    pub const fn zero() -> Self {
        Self {
            r: [Rect::zero(); 32],
            n: 0,
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Gradient {
    pub r: Rect,
    pub from: Color,
    pub to: Color,
    pub horizontal: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Blit {
    pub dst: *mut Surface,
    pub src: *const Surface,
    pub dr: Rect,
    pub sr: Rect,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Rrect {
    pub r: Rect,
    pub color: Color,
    pub tl: i32,
    pub tr: i32,
    pub br: i32,
    pub bl: i32,
}

macro_rules! export {
    ($(fn $name:ident($($arg:ident: $ty:ty),*) -> $ret:ty = $path:path);* $(;)?) => {
        $(
            #[unsafe(no_mangle)]
            /// # Safety
            /// Même contrat que la fonction C du même nom. Un pointeur nul est ignoré, sans écriture.
            pub unsafe extern "C" fn $name($($arg: $ty),*) -> $ret {
                $path($($arg),*)
            }
        )*
    };
}

export! {
    fn gfx_argb(a: u8, r: u8, g: u8, b: u8) -> Color = color::argb;
    fn gfx_blend(dst: Color, src: Color) -> Color = color::blend;
    fn gfx_lerp(a: Color, b: Color, t256: u32) -> Color = color::lerp;
    fn rect_make(x: i32, y: i32, w: i32, h: i32) -> Rect = geom::rect_make;
    fn rect_intersect(a: Rect, b: Rect) -> Rect = geom::rect_intersect;
    fn rect_union(a: Rect, b: Rect) -> Rect = geom::rect_union;
    fn rect_empty(r: Rect) -> bool = geom::rect_empty;
    fn rect_contains(r: Rect, p: Point) -> bool = geom::rect_contains;
    fn gfx_surface_init(s: *mut Surface, px: *mut u32, w: i32, h: i32) -> () = surface::init;
    fn gfx_surface_sub(s: *const Surface, r: Rect) -> Surface = surface::sub;
    fn gfx_set_clip(s: *mut Surface, r: Rect) -> () = surface::set_clip;
    fn gfx_put(s: *mut Surface, p: Point, c: Color) -> () = surface::put;
    fn gfx_get(s: *const Surface, p: Point) -> Color = surface::get;
    fn gfx_fill(s: *mut Surface, r: Rect, c: Color) -> () = draw::fill;
    fn gfx_hline(s: *mut Surface, p: Point, len: i32, c: Color) -> () = draw::hline;
    fn gfx_vline(s: *mut Surface, p: Point, len: i32, c: Color) -> () = draw::vline;
    fn gfx_line(s: *mut Surface, a: Point, b: Point, c: Color) -> () = draw::line;
    fn gfx_frame(s: *mut Surface, r: Rect, c: Color) -> () = draw::frame;
    fn gfx_gradient(s: *mut Surface, g: *const Gradient) -> () = ramp::gradient;
    fn gfx_rrect_fill(s: *mut Surface, rr: *const Rrect) -> () = rrect::fill;
    fn gfx_blit(b: *const Blit) -> () = blit::blit;
    fn gfx_blit_alpha(b: *const Blit) -> () = blit::blit_alpha;
    fn gfx_blit_scaled(b: *const Blit) -> () = blit::blit_scaled;
    fn gfx_scroll(s: *mut Surface, r: Rect, delta: Point) -> () = blit::scroll;
    fn region_clear(rg: *mut Region) -> () = region::clear;
    fn region_add(rg: *mut Region, r: Rect) -> () = region::add;
    fn region_subtract(rg: *mut Region, r: Rect) -> () = region::subtract;
    fn region_intersect(rg: *mut Region, r: Rect) -> () = region::intersect;
    fn region_bounds(rg: *const Region) -> Rect = region::bounds;
    fn region_empty(rg: *const Region) -> bool = region::is_empty;
    fn gfx_blit_smooth(b: *const Blit) -> () = blit::blit_smooth;
    fn gfx_selftest() -> i32 = selftest::run;
    fn gfx_px_at(s: *const Surface, x: i64, y: i64) -> *mut u32 = px::px_at;
    fn gfx_span_fill(d: *mut u32, n: usize, c: Color) -> () = px::span_fill;
    fn gfx_ramp_init(rp: *mut Ramp, from: Color, to: Color, n: i64) -> () = ramp::ramp_init;
    fn gfx_ramp_seek(rp: *mut Ramp, i: i64) -> () = ramp::ramp_seek;
    fn gfx_ramp_next(rp: *mut Ramp) -> () = ramp::ramp_next;
}

#[cfg(test)]
mod tests {
    use super::{
        Blit, Gradient, Point, Rect, Region, Rrect, Surface, gfx_blend, gfx_blit_scaled, gfx_fill,
        gfx_frame, gfx_get, gfx_gradient, gfx_line, gfx_put, gfx_rrect_fill, gfx_scroll,
        gfx_selftest, gfx_surface_init, rect_intersect, rect_make, region_add, region_clear,
        region_empty,
    };
    use core::mem::size_of;

    fn surface(px: &mut [u32], w: i32, h: i32) -> Surface {
        let mut s = Surface {
            px: core::ptr::null_mut(),
            w: 0,
            h: 0,
            stride: 0,
            clip: Rect::zero(),
        };
        // SAFETY: px vit plus longtemps que s, et w * h tient dans le tampon.
        unsafe { gfx_surface_init(&mut s, px.as_mut_ptr(), w, h) };
        s
    }

    #[test]
    fn layout_sizes() {
        assert_eq!(size_of::<Rect>(), 16);
        assert_eq!(size_of::<Point>(), 8);
        assert_eq!(size_of::<Surface>(), 40);
        assert_eq!(size_of::<Region>(), 516);
        assert_eq!(size_of::<Gradient>(), 28);
        assert_eq!(size_of::<Blit>(), 48);
        assert_eq!(size_of::<Rrect>(), 36);
    }

    #[test]
    fn blend_opaque_and_clear() {
        // SAFETY: pas de pointeur.
        unsafe {
            assert_eq!(gfx_blend(0xff10_2030, 0xffab_cdef), 0xffab_cdef);
            assert_eq!(gfx_blend(0xff10_2030, 0x00ff_ffff), 0xff10_2030);
            assert_eq!(gfx_blend(0xff00_0000, 0x80ff_ffff), 0xff80_8080);
            assert_eq!(gfx_blend(0xffff_ffff, 0x8000_0000), 0xff7f_7f7f);
        }
    }

    #[test]
    fn rect_intersect_empty() {
        // SAFETY: pas de pointeur.
        unsafe {
            assert_eq!(
                rect_intersect(rect_make(0, 0, 4, 4), rect_make(10, 0, 2, 2)),
                Rect::zero()
            );
            assert_eq!(
                rect_intersect(rect_make(0, 0, 0, 4), rect_make(0, 0, 4, 4)),
                Rect::zero()
            );
            assert_eq!(
                rect_intersect(rect_make(0, 0, 5, 5), rect_make(5, 0, 5, 5)),
                Rect::zero()
            );
        }
    }

    #[test]
    fn region_add_past_max() {
        let mut rg = Region::zero();
        // SAFETY: rg est locale.
        unsafe {
            region_clear(core::ptr::null_mut());
            region_add(core::ptr::null_mut(), rect_make(0, 0, 1, 1));
            assert!(region_empty(core::ptr::null()));
            region_clear(&mut rg);
            let mut i = 0i32;
            while i < 45 {
                region_add(&mut rg, rect_make(3 * i, 0, 1, 1));
                assert!(rg.n <= 32);
                if i == 31 {
                    assert_eq!(rg.n, 32);
                }
                if i == 32 {
                    assert_eq!(rg.n, 1);
                    assert_eq!(rg.r[0].x, 0);
                    assert_eq!(rg.r[0].w, 3 * 32 + 1);
                }
                i += 1;
            }
        }
    }

    #[test]
    fn selftest_and_known_pixels() {
        // SAFETY: tampons locaux.
        unsafe {
            assert_eq!(gfx_selftest(), 0);
        }
        let mut px = [0u32; 9];
        let mut s = surface(&mut px, 3, 3);
        // SAFETY: s pointe sur px.
        unsafe { gfx_frame(&mut s, rect_make(0, 0, 3, 3), 0xff00_00ff) };
        assert_eq!(px[0], 0xff00_00ff);
        assert_eq!(px[4], 0);
        assert_eq!(px[8], 0xff00_00ff);

        let mut row = [0u32; 4];
        let mut g = surface(&mut row, 4, 1);
        let grad = Gradient {
            r: Rect {
                x: 0,
                y: 0,
                w: 4,
                h: 1,
            },
            from: 0xff00_0000,
            to: 0xffff_ffff,
            horizontal: true,
        };
        // SAFETY: g pointe sur row, grad est local.
        unsafe { gfx_gradient(&mut g, &grad) };
        assert_eq!(row, [0xff00_0000, 0xff55_5555, 0xffaa_aaaa, 0xffff_ffff]);

        let mut src = [0xff00_0001, 0xff00_0002, 0xff00_0003, 0xff00_0004];
        let mut dst = [0u32; 16];
        let ss = surface(&mut src, 2, 2);
        let mut dd = surface(&mut dst, 4, 4);
        let b = Blit {
            dst: &mut dd,
            src: &ss,
            dr: Rect {
                x: 0,
                y: 0,
                w: 4,
                h: 4,
            },
            sr: Rect {
                x: 0,
                y: 0,
                w: 2,
                h: 2,
            },
        };
        // SAFETY: les deux surfaces sont locales.
        unsafe { gfx_blit_scaled(&b) };
        assert_eq!(dst[0], 0xff00_0001);
        assert_eq!(dst[5], 0xff00_0001);
        assert_eq!(dst[6], 0xff00_0002);
        assert_eq!(dst[9], 0xff00_0003);
        assert_eq!(dst[10], 0xff00_0004);
        assert_eq!(dst[15], 0xff00_0004);

        let mut line = [1u32, 2, 3, 4];
        let mut ls = surface(&mut line, 4, 1);
        // SAFETY: ls pointe sur line.
        unsafe { gfx_scroll(&mut ls, rect_make(0, 0, 4, 1), Point { x: 1, y: 0 }) };
        assert_eq!(line, [1, 1, 2, 3]);

        let mut wide = [0u32; 64];
        let mut ws = surface(&mut wide, 8, 8);
        // SAFETY: ws pointe sur wide.
        unsafe {
            gfx_line(
                &mut ws,
                Point {
                    x: i32::MIN,
                    y: i32::MIN,
                },
                Point {
                    x: i32::MAX,
                    y: i32::MAX,
                },
                0xffff_ffff,
            );
        }
        // SAFETY: ws pointe sur wide, les points lus sont dans le tampon.
        unsafe {
            assert_eq!(gfx_get(&ws, Point { x: 0, y: 0 }), 0xffff_ffff);
            assert_eq!(gfx_get(&ws, Point { x: 7, y: 7 }), 0xffff_ffff);
            assert_eq!(gfx_get(&ws, Point { x: 1, y: 0 }), 0);
        }

        let mut boxpx = [0u32; 64];
        let mut bs = surface(&mut boxpx, 8, 8);
        let rr = Rrect {
            r: Rect {
                x: 1,
                y: 1,
                w: 6,
                h: 6,
            },
            color: 0xff11_2233,
            tl: 2,
            tr: 2,
            br: 2,
            bl: 2,
        };
        // SAFETY: bs pointe sur boxpx, rr est local.
        unsafe { gfx_rrect_fill(&mut bs, &rr) };
        // SAFETY: bs pointe sur boxpx, les points lus sont dans le tampon.
        unsafe {
            assert_eq!(gfx_get(&bs, Point { x: 1, y: 1 }), 0);
            assert_eq!(gfx_get(&bs, Point { x: 2, y: 1 }), 0xff11_2233);
            assert_eq!(gfx_get(&bs, Point { x: 4, y: 4 }), 0xff11_2233);
        }

        // SAFETY: pointeur nul, aucun tampon.
        unsafe {
            gfx_fill(core::ptr::null_mut(), rect_make(0, 0, 1, 1), 1);
            gfx_put(core::ptr::null_mut(), Point { x: 0, y: 0 }, 1);
        }
    }
}
