use core::ffi::{c_char, c_int, c_void};

use crate::abi::{
    t_color, t_font, t_lunabtn, t_lunametrics, t_point, t_rect, t_region, t_surface, t_textreq,
};

unsafe extern "C" {
    #[cfg(not(all(not(test), velum_host)))]
    pub fn malloc(size: usize) -> *mut c_void;
    #[cfg(not(all(not(test), velum_host)))]
    pub fn free(ptr: *mut c_void);
    pub fn memcpy(dst: *mut c_void, src: *const c_void, n: usize) -> *mut c_void;
    pub fn memmove(dst: *mut c_void, src: *const c_void, n: usize) -> *mut c_void;
    pub fn memset(dst: *mut c_void, c: c_int, n: usize) -> *mut c_void;
    pub fn strlen(s: *const c_char) -> usize;
    pub fn strcmp(a: *const c_char, b: *const c_char) -> c_int;
}

#[cfg(not(test))]
mod c {
    use core::ffi::{c_char, c_int};

    use super::{
        t_color, t_font, t_lunabtn, t_lunametrics, t_point, t_rect, t_region, t_surface, t_textreq,
    };

    unsafe extern "C" {
        pub fn rect_make(x: i32, y: i32, w: i32, h: i32) -> t_rect;
        pub fn rect_intersect(a: t_rect, b: t_rect) -> t_rect;
        pub fn rect_union(a: t_rect, b: t_rect) -> t_rect;
        pub fn rect_empty(r: t_rect) -> bool;
        pub fn rect_contains(r: t_rect, p: t_point) -> bool;
        pub fn region_clear(rg: *mut t_region);
        pub fn region_add(rg: *mut t_region, r: t_rect);
        pub fn region_bounds(rg: *const t_region) -> t_rect;
        pub fn region_empty(rg: *const t_region) -> bool;
        pub fn gfx_set_clip(s: *mut t_surface, r: t_rect);
        pub fn gfx_put(s: *mut t_surface, p: t_point, c: t_color);
        pub fn gfx_fill(s: *mut t_surface, r: t_rect, c: t_color);
        pub fn gfx_hline(s: *mut t_surface, p: t_point, len: i32, c: t_color);
        pub fn gfx_vline(s: *mut t_surface, p: t_point, len: i32, c: t_color);
        pub fn gfx_line(s: *mut t_surface, a: t_point, b: t_point, c: t_color);
        pub fn gfx_lerp(a: t_color, b: t_color, t256: u32) -> t_color;
        pub fn font_get(id: c_int) -> *const t_font;
        pub fn font_text_width(f: *const t_font, text: *const c_char, len: i32) -> i32;
        pub fn font_draw(dst: *mut t_surface, rq: *const t_textreq);
        pub fn font_fit(f: *const t_font, text: *const c_char, max_w: i32) -> i32;
        pub fn luna_metrics(out: *mut t_lunametrics);
        pub fn luna_button(s: *mut t_surface, b: *const t_lunabtn);
        pub fn luna_checkbox(s: *mut t_surface, p: t_point, st: i32, on: bool);
        pub fn luna_edit_frame(s: *mut t_surface, r: t_rect, enabled: bool);
        pub fn luna_groupbox(s: *mut t_surface, r: t_rect, label_w: i32);
        pub fn luna_progress(s: *mut t_surface, r: t_rect, pct: u32);
        pub fn luna_menu_panel(s: *mut t_surface, r: t_rect);
        pub fn luna_menu_item(s: *mut t_surface, r: t_rect, st: i32);
        pub fn luna_theme_get() -> i32;
        pub fn luna_transparency_reduced() -> bool;
        pub fn luna_color_accent() -> t_color;
        pub fn luna_color_text() -> t_color;
        pub fn luna_color_window() -> t_color;
        pub fn luna_color_selection() -> t_color;
        pub fn luna_color_panel() -> t_color;
        pub fn luna_button_ink(b: *const t_lunabtn) -> t_color;
    }
}

pub(crate) fn rect_make(x: i32, y: i32, w: i32, h: i32) -> t_rect {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur. La fonction C ne lit que ses entiers.
        unsafe { c::rect_make(x, y, w, h) }
    }
    #[cfg(test)]
    {
        t_rect { x, y, w, h }
    }
}

pub(crate) fn rect_intersect(a: t_rect, b: t_rect) -> t_rect {
    #[cfg(not(test))]
    {
        // SAFETY: valeurs seules, même contrat que gfx.
        unsafe { c::rect_intersect(a, b) }
    }
    #[cfg(test)]
    {
        crate::stub::rect_intersect(a, b)
    }
}

pub(crate) fn rect_union(a: t_rect, b: t_rect) -> t_rect {
    #[cfg(not(test))]
    {
        // SAFETY: valeurs seules, même contrat que gfx.
        unsafe { c::rect_union(a, b) }
    }
    #[cfg(test)]
    {
        crate::stub::rect_union(a, b)
    }
}

pub(crate) fn rect_empty(r: t_rect) -> bool {
    #[cfg(not(test))]
    {
        // SAFETY: valeur seule.
        unsafe { c::rect_empty(r) }
    }
    #[cfg(test)]
    {
        r.w <= 0 || r.h <= 0
    }
}

pub(crate) fn rect_contains(r: t_rect, p: t_point) -> bool {
    #[cfg(not(test))]
    {
        // SAFETY: valeurs seules.
        unsafe { c::rect_contains(r, p) }
    }
    #[cfg(test)]
    {
        crate::stub::rect_contains(r, p)
    }
}

pub(crate) fn region_clear(rg: *mut t_region) {
    #[cfg(not(test))]
    // SAFETY: rg est nul ou une région vivante, comme region_clear.
    unsafe {
        c::region_clear(rg);
    }
    #[cfg(test)]
    crate::stub::region_clear(rg);
}

pub(crate) fn region_add(rg: *mut t_region, r: t_rect) {
    #[cfg(not(test))]
    // SAFETY: rg est nul ou une région vivante.
    unsafe {
        c::region_add(rg, r);
    }
    #[cfg(test)]
    crate::stub::region_add(rg, r);
}

pub(crate) fn region_bounds(rg: *const t_region) -> t_rect {
    #[cfg(not(test))]
    {
        // SAFETY: rg est nul ou une région vivante.
        unsafe { c::region_bounds(rg) }
    }
    #[cfg(test)]
    {
        crate::stub::region_bounds(rg)
    }
}

pub(crate) fn region_empty(rg: *const t_region) -> bool {
    #[cfg(not(test))]
    {
        // SAFETY: rg est nul ou une région vivante.
        unsafe { c::region_empty(rg) }
    }
    #[cfg(test)]
    {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        rg.is_null() || unsafe { (*rg).n == 0 }
    }
}

pub(crate) fn gfx_set_clip(s: *mut t_surface, r: t_rect) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::gfx_set_clip(s, r);
    }
    #[cfg(test)]
    crate::stub::gfx_set_clip(s, r);
}

pub(crate) fn gfx_put(s: *mut t_surface, p: t_point, c: t_color) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::gfx_put(s, p, c);
    }
    #[cfg(test)]
    {
        let _ = (s, p, c);
    }
}

pub(crate) fn gfx_fill(s: *mut t_surface, r: t_rect, c: t_color) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::gfx_fill(s, r, c);
    }
    #[cfg(test)]
    {
        let _ = (s, r, c);
    }
}

pub(crate) fn gfx_hline(s: *mut t_surface, p: t_point, len: i32, c: t_color) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::gfx_hline(s, p, len, c);
    }
    #[cfg(test)]
    {
        let _ = (s, p, len, c);
    }
}

pub(crate) fn gfx_vline(s: *mut t_surface, p: t_point, len: i32, c: t_color) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::gfx_vline(s, p, len, c);
    }
    #[cfg(test)]
    {
        let _ = (s, p, len, c);
    }
}

pub(crate) fn gfx_line(s: *mut t_surface, a: t_point, b: t_point, c: t_color) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::gfx_line(s, a, b, c);
    }
    #[cfg(test)]
    {
        let _ = (s, a, b, c);
    }
}

pub(crate) fn gfx_lerp(a: t_color, b: t_color, t256: u32) -> t_color {
    #[cfg(not(test))]
    {
        // SAFETY: entiers seuls.
        unsafe { c::gfx_lerp(a, b, t256) }
    }
    #[cfg(test)]
    {
        let _ = (a, b, t256);
        0
    }
}

pub(crate) fn font_get(id: c_int) -> *const t_font {
    #[cfg(not(test))]
    {
        // SAFETY: id est un entier. Le pointeur rendu appartient à font.
        unsafe { c::font_get(id) }
    }
    #[cfg(test)]
    {
        crate::stub::font_get(id)
    }
}

pub(crate) fn font_text_width(f: *const t_font, text: *const c_char, len: i32) -> i32 {
    #[cfg(not(test))]
    {
        // SAFETY: f et text suivent le contrat de font_text_width.
        unsafe { c::font_text_width(f, text, len) }
    }
    #[cfg(test)]
    {
        crate::stub::font_text_width(f, text, len)
    }
}

pub(crate) fn font_draw(dst: *mut t_surface, rq: *const t_textreq) {
    #[cfg(not(test))]
    // SAFETY: dst et rq suivent le contrat de font_draw.
    unsafe {
        c::font_draw(dst, rq);
    }
    #[cfg(test)]
    {
        let _ = (dst, rq);
    }
}

pub(crate) fn font_fit(f: *const t_font, text: *const c_char, max_w: i32) -> i32 {
    #[cfg(not(test))]
    {
        // SAFETY: f et text suivent le contrat de font_fit.
        unsafe { c::font_fit(f, text, max_w) }
    }
    #[cfg(test)]
    {
        crate::stub::font_fit(f, text, max_w)
    }
}

pub(crate) fn luna_metrics(out: *mut t_lunametrics) {
    #[cfg(not(test))]
    // SAFETY: out est non nul et inscriptible.
    unsafe {
        c::luna_metrics(out);
    }
    #[cfg(test)]
    crate::stub::luna_metrics(out);
}

pub(crate) fn luna_button(s: *mut t_surface, b: *const t_lunabtn) {
    #[cfg(not(test))]
    // SAFETY: s et b sont vivants.
    unsafe {
        c::luna_button(s, b);
    }
    #[cfg(test)]
    {
        let _ = (s, b);
    }
}

pub(crate) fn luna_checkbox(s: *mut t_surface, p: t_point, st: i32, on: bool) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::luna_checkbox(s, p, st, on);
    }
    #[cfg(test)]
    {
        let _ = (s, p, st, on);
    }
}

pub(crate) fn luna_edit_frame(s: *mut t_surface, r: t_rect, enabled: bool) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::luna_edit_frame(s, r, enabled);
    }
    #[cfg(test)]
    {
        let _ = (s, r, enabled);
    }
}

pub(crate) fn luna_groupbox(s: *mut t_surface, r: t_rect, label_w: i32) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::luna_groupbox(s, r, label_w);
    }
    #[cfg(test)]
    {
        let _ = (s, r, label_w);
    }
}

pub(crate) fn luna_progress(s: *mut t_surface, r: t_rect, pct: u32) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::luna_progress(s, r, pct);
    }
    #[cfg(test)]
    {
        let _ = (s, r, pct);
    }
}

pub(crate) fn luna_menu_panel(s: *mut t_surface, r: t_rect) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::luna_menu_panel(s, r);
    }
    #[cfg(test)]
    {
        let _ = (s, r);
    }
}

pub(crate) fn luna_menu_item(s: *mut t_surface, r: t_rect, st: i32) {
    #[cfg(not(test))]
    // SAFETY: s est une surface vivante.
    unsafe {
        c::luna_menu_item(s, r, st);
    }
    #[cfg(test)]
    {
        let _ = (s, r, st);
    }
}

pub(crate) fn luna_theme_get() -> i32 {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur.
        unsafe { c::luna_theme_get() }
    }
    #[cfg(test)]
    {
        crate::abi::LUNA_THEME_LIGHT
    }
}

pub(crate) fn luna_transparency_reduced() -> bool {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur.
        unsafe { c::luna_transparency_reduced() }
    }
    #[cfg(test)]
    {
        false
    }
}

pub(crate) fn luna_color_accent() -> t_color {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur.
        unsafe { c::luna_color_accent() }
    }
    #[cfg(test)]
    {
        0xff0f_7d83
    }
}

pub(crate) fn luna_color_text() -> t_color {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur.
        unsafe { c::luna_color_text() }
    }
    #[cfg(test)]
    {
        0xff00_0000
    }
}

pub(crate) fn luna_color_window() -> t_color {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur.
        unsafe { c::luna_color_window() }
    }
    #[cfg(test)]
    {
        0xffec_e9d8
    }
}

pub(crate) fn luna_color_selection() -> t_color {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur.
        unsafe { c::luna_color_selection() }
    }
    #[cfg(test)]
    {
        0xff31_6ac5
    }
}

pub(crate) fn luna_color_panel() -> t_color {
    #[cfg(not(test))]
    {
        // SAFETY: pas de pointeur.
        unsafe { c::luna_color_panel() }
    }
    #[cfg(test)]
    {
        0xffee_f0f5
    }
}

pub(crate) fn luna_button_ink(b: *const t_lunabtn) -> t_color {
    #[cfg(not(test))]
    {
        // SAFETY: b est non nul et vivant.
        unsafe { c::luna_button_ink(b) }
    }
    #[cfg(test)]
    {
        let _ = b;
        0xff00_0000
    }
}
