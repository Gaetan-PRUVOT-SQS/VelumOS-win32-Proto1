use crate::abi::{
    LP_TEXT_MAX, font_draw, font_fit, font_get, font_text_width, t_font, t_ltext, t_ltx, t_point,
    t_surface, t_textreq,
};
use crate::num::lp_clamp;
use core::ffi::c_char;

const ELLIPSIS: &[u8] = b"\xe2\x80\xa6\0";

pub(crate) fn lp_text_len(s: *const c_char) -> i32 {
    if s.is_null() {
        return 0;
    }
    let mut n = 0i32;
    while n < LP_TEXT_MAX {
        // SAFETY: n est sous LP_TEXT_MAX et s est une chaîne C, ou on s'arrête au NUL.
        if unsafe { *s.offset(n as isize) } == 0 {
            break;
        }
        n += 1;
    }
    n
}

fn tx_fit(f: *const t_font, t: &t_ltext, len: i32, ell: &mut i32) -> i32 {
    *ell = 0;
    if font_text_width(f, t.text, len) <= t.box_.w {
        return len;
    }
    *ell = font_text_width(f, ELLIPSIS.as_ptr().cast(), 3);
    lp_clamp(font_fit(f, t.text, t.box_.w - *ell), 0, len)
}

fn tx_x(t: &t_ltext, w: i32) -> i32 {
    if w >= t.box_.w || t.align == crate::abi::LA_LEFT {
        return t.box_.x;
    }
    if t.align == crate::abi::LA_CENTER {
        return t.box_.x + (t.box_.w - w) / 2;
    }
    t.box_.x + t.box_.w - w
}

fn tx_draw(s: *mut t_surface, x: &t_ltx) {
    let mut rq = t_textreq {
        font: x.f,
        at: x.p,
        color: x.c,
        text: x.text,
        len: x.n,
    };
    if x.n > 0 {
        font_draw(s, &rq);
    }
    if x.ell == 0 {
        return;
    }
    rq.at.x = x.p.x + font_text_width(x.f, x.text, x.n);
    rq.text = ELLIPSIS.as_ptr().cast();
    rq.len = 3;
    font_draw(s, &rq);
}

pub(crate) fn lp_text(s: *mut t_surface, t: *const t_ltext) {
    if s.is_null() || t.is_null() {
        return;
    }
    // SAFETY: t non nul.
    let t = unsafe { &*t };
    if t.text.is_null() || t.box_.w <= 0 {
        return;
    }
    let f = font_get(t.font);
    if f.is_null() {
        return;
    }
    let mut ell = 0;
    let n = tx_fit(f, t, lp_text_len(t.text), &mut ell);
    // SAFETY: f non nul, renvoyé par font_get.
    let height = unsafe { (*f).height };
    let mut x = t_ltx {
        f,
        text: t.text,
        p: t_point {
            x: tx_x(t, font_text_width(f, t.text, n) + ell),
            y: t.box_.y + (t.box_.h - height) / 2,
        },
        n,
        ell,
        c: t.color,
    };
    if (t.shadow >> 24) != 0 {
        x.c = t.shadow;
        x.p.x += 1;
        x.p.y += 1;
        tx_draw(s, &x);
        x.p.x -= 1;
        x.p.y -= 1;
    }
    x.c = t.color;
    tx_draw(s, &x);
}

pub(crate) mod ffi {
    use core::ffi::c_char;

    use crate::abi::{t_ltext, t_surface};

    c_fn!(lp_text_len(s: *const c_char) -> i32 { super::lp_text_len(s) });
    c_fn!(lp_text(s: *mut t_surface, t: *const t_ltext) { super::lp_text(s, t) });
}
