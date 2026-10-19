use crate::color::{blend, lerp};
use crate::draw::{fill, line};
use crate::geom::rect_make;
use crate::region::{add, bounds, clear, is_empty, subtract};
use crate::surface::{get, init};
use crate::{Point, Region, Surface};

const E_OK: i32 = 0;
const E_INVAL: i32 = -22;

fn check_blend() -> i32 {
    if blend(0xff00_0000, 0x80ff_ffff) != 0xff80_8080 {
        return E_INVAL;
    }
    if blend(0xff10_2030, 0x00ff_ffff) != 0xff10_2030 {
        return E_INVAL;
    }
    if blend(0xff10_2030, 0xffab_cdef) != 0xffab_cdef {
        return E_INVAL;
    }
    if lerp(0xff00_0000, 0xffff_ffff, 256) != 0xffff_ffff {
        return E_INVAL;
    }
    E_OK
}

fn check_fill(s: *mut Surface, px: &mut [u32; 64]) -> i32 {
    px.fill(0);
    fill(s, rect_make(2, 2, 4, 4), 0xffff_0000);
    if get(s, Point { x: 2, y: 2 }) != 0xffff_0000 {
        return E_INVAL;
    }
    if get(s, Point { x: 5, y: 5 }) != 0xffff_0000 {
        return E_INVAL;
    }
    if get(s, Point { x: 6, y: 6 }) != 0 || get(s, Point { x: 1, y: 2 }) != 0 {
        return E_INVAL;
    }
    fill(s, rect_make(-100, -100, 1000, 1000), 0xff00_ff00);
    if get(s, Point { x: 7, y: 7 }) != 0xff00_ff00 {
        return E_INVAL;
    }
    E_OK
}

fn check_line(s: *mut Surface, px: &mut [u32; 64]) -> i32 {
    px.fill(0);
    line(s, Point { x: 0, y: 0 }, Point { x: 7, y: 7 }, 0xffff_ffff);
    let mut x = 0;
    while x < 8 {
        if get(s, Point { x, y: x }) != 0xffff_ffff {
            return E_INVAL;
        }
        x += 1;
    }
    if get(s, Point { x: 1, y: 0 }) != 0 {
        return E_INVAL;
    }
    E_OK
}

fn check_region() -> i32 {
    let mut rg = Region::zero();
    clear(&mut rg);
    add(&mut rg, rect_make(0, 0, 4, 4));
    add(&mut rg, rect_make(4, 0, 4, 4));
    let b = bounds(&rg);
    if rg.n != 1 || b.x != 0 || b.y != 0 || b.w != 8 || b.h != 4 {
        return E_INVAL;
    }
    subtract(&mut rg, rect_make(0, 0, 8, 4));
    if !is_empty(&rg) {
        return E_INVAL;
    }
    E_OK
}

pub(crate) fn run() -> i32 {
    let mut px = [0u32; 64];
    let mut s = crate::surface::cleared();
    init(&mut s, px.as_mut_ptr(), 8, 8);
    let mut rc = check_blend();
    if rc == E_OK {
        rc = check_fill(&mut s, &mut px);
    }
    if rc == E_OK {
        rc = check_line(&mut s, &mut px);
    }
    if rc == E_OK {
        rc = check_region();
    }
    rc
}
