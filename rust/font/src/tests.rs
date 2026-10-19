use super::{
    Cut, FONT_IDS, FONT_INS_11, FONT_INS_12, FONT_INS_13, FONT_INS_14, FONT_INS_15, FONT_INS_22,
    FONT_INS_BOLD_13, FONT_INS_BOLD_14, FONT_INS_BOLD_24, FONT_INS_BOLD_26, FONT_INS_BOLD_28,
    FONT_INS_SEMI_12, FONT_INS_SEMI_13, FONT_INS_SEMI_14, FONT_INS_SEMI_22, FONT_INS_SEMI_120,
    FONT_JBM_14, FONT_MONO, FONT_TITLE, FONT_UI, FONT_UI_BOLD, Font, Glyph, Pen, PenBox, Point,
    Rect, Surface, TextReq, font_draw, font_fit, font_get, font_glyph, font_glyph_cut,
    font_glyph_rows, font_pen_start, font_selftest, font_text_end, font_text_width, font_utf8_next,
};
use core::cell::Cell;
use core::ffi::{c_char, c_int};
use core::mem::{align_of, offset_of, size_of};

const PAPER: u32 = 0xFFFF_FFFF;
const INK: u32 = 0xFF00_0000;
const REPLACEMENT: u32 = 0xFFFD;
const RENDER_W: i32 = 96;
const RENDER_H: i32 = 40;

#[derive(Clone, Copy)]
struct Tally {
    calls: i32,
    outside: i32,
    repeated: i32,
}

thread_local! {
    static TALLY: Cell<Tally> = const {
        Cell::new(Tally {
            calls: 0,
            outside: 0,
            repeated: 0,
        })
    };
}

fn tally() -> Tally {
    TALLY.with(Cell::get)
}

fn reset() {
    TALLY.with(|slot| {
        slot.set(Tally {
            calls: 0,
            outside: 0,
            repeated: 0,
        });
    });
}

fn bump_outside() {
    TALLY.with(|slot| {
        let mut v = slot.get();
        v.outside += 1;
        slot.set(v);
    });
}

fn inside(s: &Surface, p: Point) -> bool {
    if p.x < 0 || p.y < 0 || p.x >= s.w || p.y >= s.h {
        return false;
    }
    if p.x < s.clip.x || p.y < s.clip.y {
        return false;
    }
    if i64::from(p.x) >= i64::from(s.clip.x) + i64::from(s.clip.w) {
        return false;
    }
    i64::from(p.y) < i64::from(s.clip.y) + i64::from(s.clip.h)
}

/// # Safety
/// `dst` est nul ou vise une surface dont `px` couvre `h * stride` pixels.
pub unsafe extern "C" fn gfx_put(dst: *mut Surface, at: Point, color: u32) {
    TALLY.with(|slot| {
        let mut v = slot.get();
        v.calls += 1;
        slot.set(v);
    });
    if dst.is_null() {
        bump_outside();
        return;
    }
    // SAFETY: dst est non nul. La surface reste vivante le temps de l'appel.
    let surface = unsafe { &*dst };
    if surface.px.is_null() || !inside(surface, at) {
        bump_outside();
        return;
    }
    let Ok(y) = usize::try_from(at.y) else {
        bump_outside();
        return;
    };
    let Ok(x) = usize::try_from(at.x) else {
        bump_outside();
        return;
    };
    let Ok(stride) = usize::try_from(surface.stride) else {
        bump_outside();
        return;
    };
    let Some(index) = y.checked_mul(stride).and_then(|row| row.checked_add(x)) else {
        bump_outside();
        return;
    };
    // SAFETY: inside a borné x et y. Le test alloue h * stride pixels.
    unsafe {
        let px = surface.px.add(index);
        if px.read() != PAPER {
            TALLY.with(|slot| {
                let mut v = slot.get();
                v.repeated += 1;
                slot.set(v);
            });
        }
        px.write(color);
    }
}

fn blank(w: i32, h: i32) -> Vec<u32> {
    let n = usize::try_from(w).unwrap() * usize::try_from(h).unwrap();
    vec![PAPER; n]
}

fn surface_of(px: &mut [u32], w: i32, h: i32) -> Surface {
    Surface {
        px: px.as_mut_ptr(),
        w,
        h,
        stride: w,
        clip: Rect { x: 0, y: 0, w, h },
    }
}

fn req_of(font: *const Font, x: i32, y: i32, text: &[u8], len: i32, color: u32) -> TextReq {
    TextReq {
        font,
        at: Point { x, y },
        color,
        text: text.as_ptr().cast(),
        len,
    }
}

fn draw_on(px: &mut [u32], w: i32, h: i32, rq: &TextReq) {
    let mut dst = surface_of(px, w, h);
    // SAFETY: dst et rq vivent le temps de l'appel. Le texte du test reste en place.
    unsafe { font_draw(&mut dst, rq) };
}

fn draw_fresh(
    w: i32,
    h: i32,
    font: *const Font,
    x: i32,
    y: i32,
    text: &[u8],
    color: u32,
) -> Vec<u32> {
    let mut px = blank(w, h);
    let rq = req_of(font, x, y, text, -1, color);
    reset();
    draw_on(&mut px, w, h, &rq);
    px
}

fn with_nul(bytes: &[u8]) -> Vec<u8> {
    let mut out = Vec::with_capacity(bytes.len() + 1);
    out.extend_from_slice(bytes);
    out.push(0);
    out
}

fn must_font(id: c_int) -> *const Font {
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    let font = unsafe { font_get(id) };
    assert!(!font.is_null());
    font
}

fn must_glyph(font: *const Font, cp: u32) -> Glyph {
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    let g = unsafe { font_glyph(font, cp) };
    assert!(!g.is_null());
    // SAFETY: font_glyph a rendu un glyphe de la table.
    unsafe { *g }
}

fn utf8_next(cur: &mut *const c_char, end: *const c_char) -> u32 {
    // SAFETY: cur peut être nul. Sinon *cur et end bornent un tampon du test.
    unsafe { font_utf8_next(cur, end) }
}

fn text_end(text: *const c_char, len: i32) -> *const c_char {
    // SAFETY: text est nul ou vise un tampon dont la longueur couvre len, ou un NUL.
    unsafe { font_text_end(text, len) }
}

fn text_width(font: *const Font, text: &[u8], len: i32) -> i32 {
    // SAFETY: text vit le temps de l'appel. font est nul ou une police du test.
    unsafe { font_text_width(font, text.as_ptr().cast(), len) }
}

fn text_fit(font: *const Font, text: &[u8], max_w: i32) -> i32 {
    // SAFETY: text est terminé par NUL quand max_w est utilisé jusqu'au NUL.
    unsafe { font_fit(font, text.as_ptr().cast(), max_w) }
}

fn decode_poisoned(seq: &[u8]) -> (u32, usize) {
    assert!(seq.len() <= 16);
    let mut buf = [0x80u8; 16];
    buf[..seq.len()].copy_from_slice(seq);
    let start = buf.as_ptr().cast::<c_char>();
    let mut cur = start;
    // SAFETY: start vise buf, end est au plus un octet après les 16.
    let end = unsafe { start.add(seq.len()) };
    let cp = utf8_next(&mut cur, end);
    // SAFETY: cur a été avancé depuis start dans buf.
    let used = usize::try_from(unsafe { cur.offset_from(start) }).unwrap();
    (cp, used)
}

fn decode_all(seq: &[u8]) -> Vec<u32> {
    let start = seq.as_ptr().cast::<c_char>();
    let mut cur = start;
    // SAFETY: end est la fin du slice, un octet après le dernier.
    let end = unsafe { start.add(seq.len()) };
    let mut out = Vec::new();
    while cur < end {
        out.push(utf8_next(&mut cur, end));
    }
    out
}

fn count_replacements(seq: &[u8]) -> i32 {
    let start = seq.as_ptr().cast::<c_char>();
    let mut cur = start;
    // SAFETY: end est la fin du slice.
    let end = unsafe { start.add(seq.len()) };
    let mut count = 0i32;
    while cur < end && count >= 0 {
        if utf8_next(&mut cur, end) == REPLACEMENT {
            count += 1;
        } else {
            count = -1;
        }
    }
    count
}

struct Utf8Row {
    lead_lo: u8,
    lead_hi: u8,
    second_lo: u8,
    second_hi: u8,
    len: usize,
}

const ROWS: [Utf8Row; 8] = [
    Utf8Row {
        lead_lo: 0xC2,
        lead_hi: 0xDF,
        second_lo: 0x80,
        second_hi: 0xBF,
        len: 2,
    },
    Utf8Row {
        lead_lo: 0xE0,
        lead_hi: 0xE0,
        second_lo: 0xA0,
        second_hi: 0xBF,
        len: 3,
    },
    Utf8Row {
        lead_lo: 0xE1,
        lead_hi: 0xEC,
        second_lo: 0x80,
        second_hi: 0xBF,
        len: 3,
    },
    Utf8Row {
        lead_lo: 0xED,
        lead_hi: 0xED,
        second_lo: 0x80,
        second_hi: 0x9F,
        len: 3,
    },
    Utf8Row {
        lead_lo: 0xEE,
        lead_hi: 0xEF,
        second_lo: 0x80,
        second_hi: 0xBF,
        len: 3,
    },
    Utf8Row {
        lead_lo: 0xF0,
        lead_hi: 0xF0,
        second_lo: 0x90,
        second_hi: 0xBF,
        len: 4,
    },
    Utf8Row {
        lead_lo: 0xF1,
        lead_hi: 0xF3,
        second_lo: 0x80,
        second_hi: 0xBF,
        len: 4,
    },
    Utf8Row {
        lead_lo: 0xF4,
        lead_hi: 0xF4,
        second_lo: 0x80,
        second_hi: 0x8F,
        len: 4,
    },
];

fn byte_ok(row: &Utf8Row, p: &[u8], n: usize) -> bool {
    if n == 1 {
        return p[1] >= row.second_lo && p[1] <= row.second_hi;
    }
    (0x80..=0xBF).contains(&p[n])
}

fn ref_decode(p: &[u8]) -> (u32, usize) {
    let lead = p[0];
    if lead < 0x80 {
        return (u32::from(lead), 1);
    }
    let Some(row) = ROWS
        .iter()
        .find(|row| lead >= row.lead_lo && lead <= row.lead_hi)
    else {
        return (REPLACEMENT, 1);
    };
    let mut n = 1usize;
    while n < row.len && n < p.len() && byte_ok(row, p, n) {
        n += 1;
    }
    if n < row.len {
        return (REPLACEMENT, n);
    }
    let shift = u32::try_from(row.len - 1).unwrap();
    let mut cp = u32::from(lead & u8::try_from(0x3Fu32 >> shift).unwrap());
    let mut i = 1usize;
    while i < row.len {
        cp = (cp << 6) | u32::from(p[i] & 0x3F);
        i += 1;
    }
    (cp, row.len)
}

fn ref_encode(cp: u32, out: &mut [u8; 4]) -> usize {
    if cp < 0x80 {
        out[0] = u8::try_from(cp).unwrap();
        return 1;
    }
    if cp < 0x800 {
        out[0] = 0xC0 | u8::try_from(cp >> 6).unwrap();
        out[1] = 0x80 | u8::try_from(cp & 0x3F).unwrap();
        return 2;
    }
    if cp < 0x1_0000 {
        out[0] = 0xE0 | u8::try_from(cp >> 12).unwrap();
        out[1] = 0x80 | u8::try_from((cp >> 6) & 0x3F).unwrap();
        out[2] = 0x80 | u8::try_from(cp & 0x3F).unwrap();
        return 3;
    }
    out[0] = 0xF0 | u8::try_from(cp >> 18).unwrap();
    out[1] = 0x80 | u8::try_from((cp >> 12) & 0x3F).unwrap();
    out[2] = 0x80 | u8::try_from((cp >> 6) & 0x3F).unwrap();
    out[3] = 0x80 | u8::try_from(cp & 0x3F).unwrap();
    4
}

fn model_boundary(bytes: &[u8], from: usize) -> usize {
    if from >= bytes.len() || bytes[from] == 0 {
        return from;
    }
    let (_, n) = ref_decode(&bytes[from..]);
    from + n
}

fn model_is_boundary(bytes: &[u8], at: usize) -> bool {
    let mut cur = 0usize;
    while cur < at {
        cur = model_boundary(bytes, cur);
    }
    cur == at
}

fn model_width(font: *const Font, bytes: &[u8]) -> i32 {
    let mut at = 0usize;
    let mut sum = 0i64;
    while at < bytes.len() && bytes[at] != 0 {
        let (cp, n) = ref_decode(&bytes[at..]);
        sum += i64::from(must_glyph(font, cp).advance);
        at += n;
    }
    i32::try_from(sum).unwrap()
}

fn bit_at(font: *const Font, g: &Glyph, col: i32, row: i32) -> bool {
    let mut rows: *const u8 = core::ptr::null();
    // SAFETY: font et g viennent d'une police du test.
    let ok = unsafe { font_glyph_rows(font, g, &mut rows) };
    assert_eq!(ok, 1);
    let stride = usize::from(g.w).div_ceil(8);
    let index = usize::try_from(row).unwrap() * stride + usize::try_from(col).unwrap() / 8;
    // SAFETY: index est dans la bitmap acceptée par font_glyph_rows.
    let byte = unsafe { rows.add(index).read() };
    let bit = 7 - (col & 7);
    (u32::from(byte) >> bit) & 1 == 1
}

fn glyph_ink(font: *const Font, g: &Glyph) -> i32 {
    let mut count = 0i32;
    let mut row = 0i32;
    while row < i32::from(g.h) {
        let mut col = 0i32;
        while col < i32::from(g.w) {
            count += i32::from(bit_at(font, g, col, row));
            col += 1;
        }
        row += 1;
    }
    count
}

struct Spec {
    id: c_int,
    ascent: i32,
    descent: i32,
    height: i32,
    nglyphs: i32,
    nbits: u32,
}

const SPECS: [Spec; 21] = [
    Spec {
        id: FONT_UI,
        ascent: 12,
        descent: 3,
        height: 15,
        nglyphs: 128,
        nbits: 930,
    },
    Spec {
        id: FONT_UI_BOLD,
        ascent: 12,
        descent: 3,
        height: 15,
        nglyphs: 128,
        nbits: 983,
    },
    Spec {
        id: FONT_TITLE,
        ascent: 13,
        descent: 3,
        height: 16,
        nglyphs: 128,
        nbits: 1160,
    },
    Spec {
        id: FONT_MONO,
        ascent: 13,
        descent: 3,
        height: 16,
        nglyphs: 128,
        nbits: 1038,
    },
    Spec {
        id: FONT_INS_11,
        ascent: 11,
        descent: 3,
        height: 14,
        nglyphs: 128,
        nbits: 907,
    },
    Spec {
        id: FONT_INS_12,
        ascent: 11,
        descent: 3,
        height: 14,
        nglyphs: 128,
        nbits: 937,
    },
    Spec {
        id: FONT_INS_SEMI_12,
        ascent: 11,
        descent: 3,
        height: 14,
        nglyphs: 128,
        nbits: 964,
    },
    Spec {
        id: FONT_INS_13,
        ascent: 14,
        descent: 3,
        height: 17,
        nglyphs: 128,
        nbits: 1208,
    },
    Spec {
        id: FONT_INS_SEMI_13,
        ascent: 14,
        descent: 3,
        height: 17,
        nglyphs: 128,
        nbits: 1278,
    },
    Spec {
        id: FONT_INS_BOLD_13,
        ascent: 14,
        descent: 3,
        height: 17,
        nglyphs: 128,
        nbits: 1281,
    },
    Spec {
        id: FONT_INS_14,
        ascent: 14,
        descent: 3,
        height: 17,
        nglyphs: 128,
        nbits: 1246,
    },
    Spec {
        id: FONT_INS_SEMI_14,
        ascent: 14,
        descent: 3,
        height: 17,
        nglyphs: 128,
        nbits: 1318,
    },
    Spec {
        id: FONT_INS_BOLD_14,
        ascent: 14,
        descent: 3,
        height: 17,
        nglyphs: 128,
        nbits: 1352,
    },
    Spec {
        id: FONT_INS_15,
        ascent: 15,
        descent: 3,
        height: 18,
        nglyphs: 128,
        nbits: 1451,
    },
    Spec {
        id: FONT_INS_22,
        ascent: 22,
        descent: 5,
        height: 27,
        nglyphs: 128,
        nbits: 3238,
    },
    Spec {
        id: FONT_INS_SEMI_22,
        ascent: 22,
        descent: 5,
        height: 27,
        nglyphs: 128,
        nbits: 3369,
    },
    Spec {
        id: FONT_INS_BOLD_24,
        ascent: 24,
        descent: 5,
        height: 29,
        nglyphs: 122,
        nbits: 3800,
    },
    Spec {
        id: FONT_INS_BOLD_26,
        ascent: 26,
        descent: 6,
        height: 32,
        nglyphs: 122,
        nbits: 4333,
    },
    Spec {
        id: FONT_INS_BOLD_28,
        ascent: 28,
        descent: 6,
        height: 34,
        nglyphs: 122,
        nbits: 4947,
    },
    Spec {
        id: FONT_INS_SEMI_120,
        ascent: 89,
        descent: 1,
        height: 90,
        nglyphs: 14,
        nbits: 8047,
    },
    Spec {
        id: FONT_JBM_14,
        ascent: 14,
        descent: 3,
        height: 17,
        nglyphs: 128,
        nbits: 1192,
    },
];

#[test]
fn abi_sizes_match_both_gcc_layouts() {
    assert_eq!(size_of::<c_int>(), 4);
    assert_eq!(align_of::<c_int>(), 4);
    assert_eq!(size_of::<Glyph>(), 12);
    assert_eq!(align_of::<Glyph>(), 4);
    assert_eq!(offset_of!(Glyph, cp), 0);
    assert_eq!(offset_of!(Glyph, w), 4);
    assert_eq!(offset_of!(Glyph, h), 5);
    assert_eq!(offset_of!(Glyph, xoff), 6);
    assert_eq!(offset_of!(Glyph, yoff), 7);
    assert_eq!(offset_of!(Glyph, advance), 8);
    assert_eq!(offset_of!(Glyph, off), 10);
    assert_eq!(size_of::<Font>(), 40);
    assert_eq!(align_of::<Font>(), 8);
    assert_eq!(offset_of!(Font, ascent), 0);
    assert_eq!(offset_of!(Font, descent), 4);
    assert_eq!(offset_of!(Font, height), 8);
    assert_eq!(offset_of!(Font, nglyphs), 12);
    assert_eq!(offset_of!(Font, glyphs), 16);
    assert_eq!(offset_of!(Font, bits), 24);
    assert_eq!(offset_of!(Font, nbits), 32);
    assert_eq!(size_of::<TextReq>(), 40);
    assert_eq!(align_of::<TextReq>(), 8);
    assert_eq!(offset_of!(TextReq, font), 0);
    assert_eq!(offset_of!(TextReq, at), 8);
    assert_eq!(offset_of!(TextReq, color), 16);
    assert_eq!(offset_of!(TextReq, text), 24);
    assert_eq!(offset_of!(TextReq, len), 32);
    assert_eq!(size_of::<Point>(), 8);
    assert_eq!(align_of::<Point>(), 4);
    assert_eq!(offset_of!(Point, x), 0);
    assert_eq!(offset_of!(Point, y), 4);
    assert_eq!(size_of::<Rect>(), 16);
    assert_eq!(align_of::<Rect>(), 4);
    assert_eq!(offset_of!(Rect, x), 0);
    assert_eq!(offset_of!(Rect, y), 4);
    assert_eq!(offset_of!(Rect, w), 8);
    assert_eq!(offset_of!(Rect, h), 12);
    assert_eq!(size_of::<Surface>(), 40);
    assert_eq!(align_of::<Surface>(), 8);
    assert_eq!(offset_of!(Surface, px), 0);
    assert_eq!(offset_of!(Surface, w), 8);
    assert_eq!(offset_of!(Surface, h), 12);
    assert_eq!(offset_of!(Surface, stride), 16);
    assert_eq!(offset_of!(Surface, clip), 20);
    assert_eq!(size_of::<PenBox>(), 16);
    assert_eq!(align_of::<PenBox>(), 4);
    assert_eq!(offset_of!(PenBox, left), 0);
    assert_eq!(offset_of!(PenBox, top), 4);
    assert_eq!(offset_of!(PenBox, right), 8);
    assert_eq!(offset_of!(PenBox, bottom), 12);
    assert_eq!(size_of::<Pen>(), 56);
    assert_eq!(align_of::<Pen>(), 8);
    assert_eq!(offset_of!(Pen, dst), 0);
    assert_eq!(offset_of!(Pen, font), 8);
    assert_eq!(offset_of!(Pen, color), 16);
    assert_eq!(offset_of!(Pen, x), 24);
    assert_eq!(offset_of!(Pen, y), 32);
    assert_eq!(offset_of!(Pen, bounds), 40);
    assert_eq!(size_of::<Cut>(), 32);
    assert_eq!(align_of::<Cut>(), 8);
    assert_eq!(offset_of!(Cut, rows), 0);
    assert_eq!(offset_of!(Cut, x), 8);
    assert_eq!(offset_of!(Cut, y), 12);
    assert_eq!(offset_of!(Cut, c0), 16);
    assert_eq!(offset_of!(Cut, c1), 20);
    assert_eq!(offset_of!(Cut, r0), 24);
    assert_eq!(offset_of!(Cut, r1), 28);
}

#[test]
fn utf8_table_3_7_edges_and_ascii() {
    let edges: [(&[u8], u32); 19] = [
        (&[0x7F], 0x7F),
        (&[0xC2, 0x80], 0x80),
        (&[0xDF, 0xBF], 0x7FF),
        (&[0xE0, 0xA0, 0x80], 0x800),
        (&[0xE0, 0xBF, 0xBF], 0xFFF),
        (&[0xE1, 0x80, 0x80], 0x1000),
        (&[0xEC, 0xBF, 0xBF], 0xCFFF),
        (&[0xED, 0x80, 0x80], 0xD000),
        (&[0xED, 0x9F, 0xBF], 0xD7FF),
        (&[0xEE, 0x80, 0x80], 0xE000),
        (&[0xEF, 0xBF, 0xBF], 0xFFFF),
        (&[0xF0, 0x90, 0x80, 0x80], 0x1_0000),
        (&[0xF0, 0xBF, 0xBF, 0xBF], 0x3_FFFF),
        (&[0xF1, 0x80, 0x80, 0x80], 0x4_0000),
        (&[0xF3, 0xBF, 0xBF, 0xBF], 0xF_FFFF),
        (&[0xF4, 0x80, 0x80, 0x80], 0x10_0000),
        (&[0xF4, 0x8F, 0xBF, 0xBF], 0x10_FFFF),
        (&[0xC3, 0xA9], 0xE9),
        (&[0xE2, 0x82, 0xAC], 0x20AC),
    ];
    for (bytes, cp) in edges {
        let (got, used) = decode_poisoned(bytes);
        assert_eq!(got, cp);
        assert_eq!(used, bytes.len());
    }
    let (emoji, used) = decode_poisoned(&[0xF0, 0x9F, 0x98, 0x80]);
    assert_eq!(emoji, 0x1_F600);
    assert_eq!(used, 4);
    for byte in 0u8..0x80 {
        let (cp, used) = decode_poisoned(&[byte]);
        assert_eq!(cp, u32::from(byte));
        assert_eq!(used, 1);
    }
}

#[test]
fn utf8_invalid_leads_and_never_sequences() {
    for byte in 0x80u16..=0xFF {
        if byte <= 0xC1 || byte >= 0xF5 {
            let (cp, used) = decode_poisoned(&[u8::try_from(byte).unwrap()]);
            assert_eq!(cp, REPLACEMENT);
            assert_eq!(used, 1);
        }
    }
    let never: [&[u8]; 12] = [
        &[0xC0, 0x80],
        &[0xC1, 0xBF],
        &[0xE0, 0x80, 0x80],
        &[0xE0, 0x9F, 0xBF],
        &[0xED, 0xA0, 0x80],
        &[0xED, 0xBF, 0xBF],
        &[0xF0, 0x80, 0x80, 0x80],
        &[0xF0, 0x8F, 0xBF, 0xBF],
        &[0xF4, 0x90, 0x80, 0x80],
        &[0xF5, 0x80, 0x80, 0x80],
        &[0xF8, 0x88, 0x80, 0x80, 0x80],
        &[0xFF, 0xFE],
    ];
    for seq in never {
        assert_eq!(count_replacements(seq), i32::try_from(seq.len()).unwrap());
    }
}

#[test]
fn utf8_lead_times_second_matches_reference() {
    let mut seq = [0u8; 4];
    seq[2] = 0x80;
    seq[3] = 0x80;
    for lead in 0x80u16..=0xFF {
        for second in 0u16..=0xFF {
            seq[0] = u8::try_from(lead).unwrap();
            seq[1] = u8::try_from(second).unwrap();
            let (want, want_used) = ref_decode(&seq);
            let (got, used) = decode_poisoned(&seq);
            assert_eq!(got, want);
            assert_eq!(used, want_used);
        }
    }
}

#[test]
fn utf8_scalar_roundtrip_and_surrogate_encodings() {
    let mut enc = [0u8; 4];
    let mut cp = 0u32;
    while cp <= 0x10_FFFF {
        if !(0xD800..=0xDFFF).contains(&cp) {
            let len = ref_encode(cp, &mut enc);
            let (got, used) = decode_poisoned(&enc[..len]);
            assert_eq!(got, cp);
            assert_eq!(used, len);
        }
        cp += 1;
    }
    cp = 0xD800;
    while cp <= 0xDFFF {
        let len = ref_encode(cp, &mut enc);
        assert_eq!(count_replacements(&enc[..len]), 3);
        cp += 1;
    }
}

#[test]
fn utf8_truncation_maximal_subpart_and_null() {
    let samples: [&[u8]; 7] = [
        &[0xC3, 0xA9],
        &[0xE2, 0x82, 0xAC],
        &[0xF0, 0x9F, 0x98, 0x80],
        &[0xED, 0x9F, 0xBF],
        &[0xF4, 0x8F, 0xBF, 0xBF],
        &[0xE0, 0xA0, 0x80],
        &[0xF0, 0x90, 0x80, 0x80],
    ];
    for sample in samples {
        let mut k = 1usize;
        while k < sample.len() {
            let mut seq = [0u8; 5];
            seq[..k].copy_from_slice(&sample[..k]);
            let (cp, used) = decode_poisoned(&seq[..k]);
            assert_eq!(cp, REPLACEMENT);
            assert_eq!(used, k);
            seq[k] = b'A';
            let (cp, used) = decode_poisoned(&seq[..=k]);
            assert_eq!(cp, REPLACEMENT);
            assert_eq!(used, k);
            k += 1;
        }
    }
    let example = [
        0x61, 0xF1, 0x80, 0x80, 0xE1, 0x80, 0xC2, 0x62, 0x80, 0x63, 0x80, 0xBF, 0x64,
    ];
    let out = decode_all(&example);
    assert_eq!(
        out,
        [
            0x61,
            REPLACEMENT,
            REPLACEMENT,
            REPLACEMENT,
            0x62,
            REPLACEMENT,
            0x63,
            REPLACEMENT,
            REPLACEMENT,
            0x64,
        ]
    );
    assert_eq!(decode_all(&[0xE0, 0x80]).len(), 2);
    let tail = decode_all(&[0xF0, 0x9F, 0x98, 0x41]);
    assert_eq!(tail.len(), 2);
    assert_eq!(tail[1], 0x41);
    let tables: [(&[u8], &[u8]); 8] = [
        (&[0xC0, 0xAF], b"FF"),
        (&[0xE0, 0x80, 0xBF], b"FFF"),
        (&[0xF0, 0x81, 0x82, 0x41], b"FFFA"),
        (&[0xED, 0xA0, 0x80], b"FFF"),
        (&[0xED, 0xBF, 0xBF], b"FFF"),
        (&[0xED, 0xAF, 0x41], b"FFA"),
        (
            &[0xF4, 0x91, 0x92, 0x93, 0xFF, 0x41, 0x80, 0xBF, 0x42],
            b"FFFFFAFFB",
        ),
        (
            &[0xE1, 0x80, 0xE2, 0xF0, 0x91, 0x92, 0xF1, 0xBF, 0x41],
            b"FFFFA",
        ),
    ];
    for (bytes, want) in tables {
        let got = decode_all(bytes);
        assert_eq!(got.len(), want.len());
        for (cp, mark) in got.iter().zip(want.iter()) {
            let expect = if *mark == b'F' {
                REPLACEMENT
            } else if *mark == b'A' {
                0x41
            } else {
                0x42
            };
            assert_eq!(*cp, expect);
        }
    }
    let text = *b"AB";
    let start = text.as_ptr().cast::<c_char>();
    let mut cur = start;
    assert_eq!(utf8_next(&mut cur, start), 0);
    assert!(core::ptr::eq(cur, start));
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    cur = unsafe { start.add(1) };
    assert_eq!(utf8_next(&mut cur, start), 0);
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(core::ptr::eq(cur, unsafe { start.add(1) }));
    // SAFETY: s nul est le cas refusé sans lecture.
    assert_eq!(unsafe { font_utf8_next(core::ptr::null_mut(), start) }, 0);
    cur = core::ptr::null();
    assert_eq!(utf8_next(&mut cur, start), 0);
    let nul = [0u8, b'A'];
    cur = nul.as_ptr().cast();
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        utf8_next(&mut cur, unsafe { nul.as_ptr().cast::<c_char>().add(2) }),
        0
    );
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { cur.cast::<u8>().read() }, b'A');
}

#[test]
fn glyph_lookup_tiny_sparse_and_degenerate() {
    let tiny = [
        Glyph {
            cp: 0x20,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 3,
            off: 0,
        },
        Glyph {
            cp: 0x41,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 5,
            off: 0,
        },
        Glyph {
            cp: 0x5A,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 6,
            off: 0,
        },
        Glyph {
            cp: 0x100,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 7,
            off: 0,
        },
        Glyph {
            cp: 0x2000,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 8,
            off: 0,
        },
        Glyph {
            cp: 0xFFFD,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 9,
            off: 0,
        },
    ];
    let sparse = [
        Glyph {
            cp: 0x20,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 3,
            off: 0,
        },
        Glyph {
            cp: 0x41,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 5,
            off: 0,
        },
        Glyph {
            cp: 0x42,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 6,
            off: 0,
        },
        Glyph {
            cp: 0xFFFD,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 9,
            off: 0,
        },
    ];
    let tiny_font = Font {
        ascent: 8,
        descent: 2,
        height: 10,
        nglyphs: 6,
        glyphs: tiny.as_ptr(),
        bits: core::ptr::null(),
        nbits: 0,
    };
    let sparse_font = Font {
        ascent: 8,
        descent: 2,
        height: 10,
        nglyphs: 4,
        glyphs: sparse.as_ptr(),
        bits: core::ptr::null(),
        nbits: 0,
    };
    for (i, glyph) in tiny.iter().enumerate() {
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        let found = unsafe { font_glyph(&tiny_font, glyph.cp) };
        assert!(core::ptr::eq(found, tiny.as_ptr().wrapping_add(i)));
    }
    let absent = [
        0u32,
        0x1F,
        0x21,
        0x26,
        0x40,
        0x42,
        0x59,
        0x5B,
        0xFF,
        0x101,
        0x1FFF,
        0x2001,
        0xFFFC,
        0xFFFE,
        0x10_FFFF,
        0xD800,
        0xFFFF_FFFF,
    ];
    for cp in absent {
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        let found = unsafe { font_glyph(&tiny_font, cp) };
        assert!(core::ptr::eq(found, tiny.as_ptr().wrapping_add(5)));
    }
    assert!(core::ptr::eq(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph(&sparse_font, 0x41) },
        sparse.as_ptr().wrapping_add(1)
    ));
    assert!(core::ptr::eq(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph(&sparse_font, 0x42) },
        sparse.as_ptr().wrapping_add(2)
    ));
    assert!(core::ptr::eq(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph(&sparse_font, 0x21) },
        sparse.as_ptr().wrapping_add(3)
    ));
    assert!(core::ptr::eq(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph(&sparse_font, 0x20) },
        sparse.as_ptr()
    ));
    assert!(core::ptr::eq(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph(&sparse_font, 0x24) },
        sparse.as_ptr().wrapping_add(3)
    ));
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_glyph(core::ptr::null(), 0x41) }.is_null());
    let mut broken = tiny_font;
    broken.nglyphs = 0;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_glyph(&broken, 0x41) }.is_null());
    broken.nglyphs = -3;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_glyph(&broken, 0x41) }.is_null());
    broken = tiny_font;
    broken.glyphs = core::ptr::null();
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_glyph(&broken, 0x41) }.is_null());
    broken = tiny_font;
    broken.nglyphs = 5;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_glyph(&broken, 0x42) }.is_null());
    broken.nglyphs = 1;
    assert!(core::ptr::eq(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph(&broken, 0x20) },
        tiny.as_ptr()
    ));
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_glyph(&broken, 0x41) }.is_null());
}

#[test]
fn real_fonts_match_published_metrics_and_scan() {
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_get(-1) }.is_null());
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_get(FONT_IDS) }.is_null());
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_get(77) }.is_null());
    let mut id = 0;
    while id < FONT_IDS {
        let font = must_font(id);
        assert!(core::ptr::eq(font, must_font(id)));
        if id + 1 < FONT_IDS {
            assert!(!core::ptr::eq(font, must_font(id + 1)));
        }
        id += 1;
    }
    for spec in SPECS {
        // SAFETY: l'identifiant est dans la table publiée.
        let font = unsafe { &*must_font(spec.id) };
        assert_eq!(font.ascent, spec.ascent);
        assert_eq!(font.descent, spec.descent);
        assert_eq!(font.height, spec.height);
        assert_eq!(font.nglyphs, spec.nglyphs);
        assert_eq!(font.nbits, spec.nbits);
        assert!(!font.glyphs.is_null());
        assert!(!font.bits.is_null());
        // SAFETY: nglyphs glyphes sont publiés avec la police.
        let first = unsafe { font.glyphs.read() };
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        let last = unsafe {
            font.glyphs
                .add(usize::try_from(font.nglyphs - 1).unwrap())
                .read()
        };
        assert_eq!(first.cp, 0x20);
        assert_eq!(last.cp, REPLACEMENT);
        let mut k = 1i32;
        while k < font.nglyphs {
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let prev = unsafe { font.glyphs.add(usize::try_from(k - 1).unwrap()).read().cp };
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let cur = unsafe { font.glyphs.add(usize::try_from(k).unwrap()).read().cp };
            assert!(prev < cur);
            k += 1;
        }
    }
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_selftest() }, 0);
}

#[test]
fn real_glyph_lookup_matches_linear_scan() {
    let mut id = 0;
    while id < FONT_IDS {
        let font_ptr = must_font(id);
        // SAFETY: police du tableau statique.
        let font = unsafe { &*font_ptr };
        let mut cp = 0u32;
        while cp < 0x2200 {
            let mut want_i = if cp == 0x202F {
                0
            } else {
                usize::try_from(font.nglyphs - 1).unwrap()
            };
            let mut k = 0i32;
            while k < font.nglyphs {
                // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
                let found = unsafe { font.glyphs.add(usize::try_from(k).unwrap()).read().cp };
                if found == cp {
                    want_i = usize::try_from(k).unwrap();
                    break;
                }
                k += 1;
            }
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let want = unsafe { font.glyphs.add(want_i) };
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            assert!(core::ptr::eq(unsafe { font_glyph(font_ptr, cp) }, want));
            cp += 1;
        }
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        let last = unsafe { font.glyphs.add(usize::try_from(font.nglyphs - 1).unwrap()) };
        assert!(core::ptr::eq(
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            unsafe { font_glyph(font_ptr, 0x10_FFFF) },
            last
        ));
        id += 1;
    }
}

#[test]
fn pinned_ui_eacute_replacement_and_mono_h() {
    let ui = must_font(FONT_UI);
    let accent = must_glyph(ui, 0xE9);
    assert_eq!(
        (
            accent.cp,
            accent.w,
            accent.h,
            accent.xoff,
            accent.yoff,
            accent.advance,
            accent.off
        ),
        (0xE9, 4, 9, 1, 3, 6, 831)
    );
    let missing = must_glyph(ui, REPLACEMENT);
    assert_eq!(
        (
            missing.w,
            missing.h,
            missing.xoff,
            missing.yoff,
            missing.advance,
            missing.off
        ),
        (4, 8, 1, 4, 6, 922)
    );
    let mono = must_font(FONT_MONO);
    let h = must_glyph(mono, u32::from(b'H'));
    assert_eq!(
        (h.w, h.h, h.xoff, h.yoff, h.advance, h.off),
        (6, 9, 1, 4, 8, 303)
    );
    assert_eq!(must_glyph(mono, 0xE9).cp, 0xE9);
    let rows_expect = [0x84u8, 0x84, 0x84, 0x84, 0xFC, 0x84, 0x84, 0x84, 0x84];
    let mut rows: *const u8 = rows_expect.as_ptr().wrapping_add(1);
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_glyph_rows(mono, &h, &mut rows) }, 1);
    for (i, byte) in rows_expect.iter().enumerate() {
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        assert_eq!(unsafe { rows.add(i).read() }, *byte);
    }
}

#[test]
fn nbsp_matches_space_and_permille_is_a_box() {
    let mut id = 0;
    while id < FONT_IDS {
        let font = must_font(id);
        let space = must_glyph(font, 0x20);
        let nbsp = must_glyph(font, 0xA0);
        let nnbsp = must_glyph(font, 0x202F);
        assert_ne!(nbsp.cp, REPLACEMENT);
        assert_ne!(nnbsp.cp, REPLACEMENT);
        assert_eq!(nbsp.w, 0);
        assert_eq!(nnbsp.w, 0);
        assert_eq!(must_glyph(font, 0x2030).cp, REPLACEMENT);
        let spaced = with_nul("a :".as_bytes());
        let nb = with_nul(&[b'a', 0xC2, 0xA0, b':']);
        let nn = with_nul(&[b'a', 0xE2, 0x80, 0xAF, b':']);
        let want = text_width(font, &spaced, -1);
        assert_eq!(text_width(font, &nb, -1), want);
        assert_eq!(text_width(font, &nn, -1), want);
        assert_eq!(
            text_width(font, &with_nul(&[0xC2, 0xA0]), -1),
            i32::from(space.advance)
        );
        assert_eq!(
            text_width(font, &with_nul(&[0xE2, 0x80, 0xAF]), -1),
            i32::from(space.advance)
        );
        assert_eq!(
            draw_fresh(
                RENDER_W,
                RENDER_H,
                font,
                3,
                4,
                &with_nul(&[0xC2, 0xA0]),
                INK
            )
            .iter()
            .filter(|p| **p == INK)
            .count(),
            0
        );
        assert_eq!(tally().calls, 0);
        assert_eq!(
            draw_fresh(
                RENDER_W,
                RENDER_H,
                font,
                3,
                4,
                &with_nul(&[0xE2, 0x80, 0xAF]),
                INK
            )
            .iter()
            .filter(|p| **p == INK)
            .count(),
            0
        );
        let known = draw_fresh(RENDER_W, RENDER_H, font, 3, 4, &spaced, INK);
        let with_nb = draw_fresh(RENDER_W, RENDER_H, font, 3, 4, &nb, INK);
        let with_nn = draw_fresh(RENDER_W, RENDER_H, font, 3, 4, &nn, INK);
        assert_eq!(known, with_nb);
        assert_eq!(known, with_nn);
        let box_ink = draw_fresh(
            RENDER_W,
            RENDER_H,
            font,
            3,
            4,
            &with_nul(&[0xE2, 0x80, 0xB0]),
            INK,
        );
        assert!(box_ink.contains(&INK));
        id += 1;
    }
}

#[test]
fn width_matches_reference_and_len_bounds() {
    let phrases: [&[u8]; 12] = [
        b"",
        b"A",
        "Démarrer".as_bytes(),
        "Poste de travail".as_bytes(),
        "Éteindre l'ordinateur".as_bytes(),
        "€ œ Œ « » … – — ‘ ’ “ ”".as_bytes(),
        "ÀÂÇÉÈ àâçéè".as_bytes(),
        "😀x".as_bytes(),
        &[0xE2, 0x82],
        &[b'a', 0xFF, b'z'],
        "日本語".as_bytes(),
        &[0xC0, 0x80, 0xED, 0xA0, 0x80],
    ];
    for spec in SPECS {
        let font = must_font(spec.id);
        for phrase in phrases {
            let exact = text_width(font, phrase, i32::try_from(phrase.len()).unwrap());
            assert_eq!(exact, model_width(font, phrase));
            let ended = with_nul(phrase);
            assert_eq!(text_width(font, &ended, -1), exact);
        }
    }
    let mono = must_font(FONT_MONO);
    let dem = with_nul("Démarrer".as_bytes());
    assert_eq!(text_width(mono, &dem, 9), 64);
    assert_eq!(text_width(mono, &dem, -1), 64);
    assert_eq!(text_fit(mono, &dem, 63), 8);
    assert_eq!(text_fit(mono, &dem, 24), 4);
    assert_eq!(text_width(mono, b"ab", 1), 8);
    let ui = must_font(FONT_UI);
    assert_eq!(text_width(ui, b"abc", 0), 0);
    assert_eq!(
        text_width(ui, b"ab\0", i32::MIN),
        text_width(ui, b"ab\0", 2)
    );
    assert_eq!(text_width(ui, b"ab\0cd", 5), text_width(ui, b"ab\0", 2));
    assert_eq!(
        text_width(ui, &[0xC3, 0xA9], 1),
        i32::from(must_glyph(ui, REPLACEMENT).advance)
    );
    assert_eq!(
        text_width(ui, &[0xC3, 0xA9], 2),
        i32::from(must_glyph(ui, 0xE9).advance)
    );
    assert_ne!(must_glyph(ui, 0xE9).h, must_glyph(ui, REPLACEMENT).h);
    assert_eq!(
        text_width(ui, b"ab\0", i32::MAX),
        text_width(ui, b"ab\0", 2)
    );
    assert_eq!(text_width(core::ptr::null(), b"abc\0", 3), 0);
    // SAFETY: le texte nul est refusé sans lecture.
    assert_eq!(unsafe { font_text_width(ui, core::ptr::null(), 3) }, 0);
    let wide_glyphs = [
        Glyph {
            cp: 0x41,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 255,
            off: 0,
        },
        Glyph {
            cp: REPLACEMENT,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 1,
            off: 0,
        },
    ];
    let wide = Font {
        ascent: 8,
        descent: 2,
        height: 10,
        nglyphs: 2,
        glyphs: wide_glyphs.as_ptr(),
        bits: core::ptr::null(),
        nbits: 0,
    };
    let mut many = vec![b'A'; 8_000_000];
    many.push(0);
    assert_eq!(text_width(&wide, &many, -1), 2_040_000_000);
    let mut more = vec![b'A'; 9_000_000];
    more.push(0);
    assert_eq!(text_width(&wide, &more, -1), i32::MAX);
}

#[test]
fn fit_phrases_and_degenerate_widths() {
    let phrases: [&[u8]; 7] = [
        "Démarrer".as_bytes(),
        "Poste de travail".as_bytes(),
        "Éteindre l'ordinateur".as_bytes(),
        "€ œ Œ « » … – —".as_bytes(),
        "😀x日本".as_bytes(),
        &[b'a', 0xFF, b'z', 0xE2, 0x82],
        "ÀÂÇÉÈ àâçéè".as_bytes(),
    ];
    for spec in SPECS {
        let font = must_font(spec.id);
        for phrase in phrases {
            let ended = with_nul(phrase);
            let total = text_width(font, &ended, -1);
            let mut prev = 0i32;
            let mut max_w = -2i32;
            while max_w <= total + 3 {
                let n = text_fit(font, &ended, max_w);
                let n_us = usize::try_from(n).unwrap();
                assert!(n_us <= phrase.len());
                assert!(model_is_boundary(phrase, n_us));
                if max_w < 0 {
                    assert_eq!(n, 0);
                } else {
                    assert!(model_width(font, &phrase[..n_us]) <= max_w || n == 0);
                    if n_us < phrase.len() {
                        let next = model_boundary(phrase, n_us);
                        assert!(model_width(font, &phrase[..next]) > max_w);
                    }
                }
                assert!(n >= prev);
                prev = n;
                max_w += 1;
            }
            assert_eq!(
                text_fit(font, &ended, total),
                i32::try_from(phrase.len()).unwrap()
            );
        }
    }
    let ui = must_font(FONT_UI);
    let abc = with_nul(b"abc");
    assert_eq!(text_fit(core::ptr::null(), &abc, 100), 0);
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_fit(ui, core::ptr::null(), 100) }, 0);
    assert_eq!(text_fit(ui, &with_nul(b""), 100), 0);
    assert_eq!(text_fit(ui, &abc, 0), 0);
    assert_eq!(text_fit(ui, &abc, i32::MAX), 3);
    assert_eq!(text_fit(ui, &abc, i32::MIN), 0);
    let wide_glyphs = [
        Glyph {
            cp: 0x41,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 255,
            off: 0,
        },
        Glyph {
            cp: REPLACEMENT,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 1,
            off: 0,
        },
    ];
    let wide = Font {
        ascent: 8,
        descent: 2,
        height: 10,
        nglyphs: 2,
        glyphs: wide_glyphs.as_ptr(),
        bits: core::ptr::null(),
        nbits: 0,
    };
    let mut more = vec![b'A'; 9_000_000];
    more.push(0);
    assert_eq!(text_fit(&wide, &more, i32::MAX), 8_421_504);
}

#[test]
fn bare_font_width_fit_and_degenerate_draw() {
    let bits = [0x80u8, 0x80, 0x80];
    let glyphs = [
        Glyph {
            cp: 0x41,
            w: 1,
            h: 3,
            xoff: 0,
            yoff: 0,
            advance: 5,
            off: 0,
        },
        Glyph {
            cp: 0x42,
            w: 0,
            h: 3,
            xoff: 0,
            yoff: 0,
            advance: 4,
            off: 0,
        },
        Glyph {
            cp: 0x43,
            w: 3,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 4,
            off: 0,
        },
    ];
    let font = Font {
        ascent: 3,
        descent: 0,
        height: 3,
        nglyphs: 3,
        glyphs: glyphs.as_ptr(),
        bits: bits.as_ptr(),
        nbits: 3,
    };
    let xay = with_nul(b"xAy");
    let abc = with_nul(b"ABC");
    let xaa = with_nul(b"xAA");
    assert_eq!(text_width(&font, &xay, -1), 5);
    assert_eq!(text_width(&font, &abc, -1), 13);
    assert_eq!(text_fit(&font, &xay, 5), 3);
    assert_eq!(text_fit(&font, &xaa, 5), 2);
    assert_eq!(text_fit(&font, &xaa, 10), 3);
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(unsafe { font_glyph(&font, 0x78) }.is_null());
    let cases: [(&[u8], i32); 5] = [
        (&xay, 3),
        (&with_nul(b"B"), 0),
        (&with_nul(b"C"), 0),
        (&abc, 3),
        (&with_nul(b"xyz"), 0),
    ];
    for (text, want) in cases {
        let _px = draw_fresh(RENDER_W, RENDER_H, &font, 3, 4, text, INK);
        assert_eq!(tally().calls, want);
        assert_eq!(tally().outside, 0);
    }
    let px = draw_fresh(RENDER_W, RENDER_H, &font, 3, 4, &xay, INK);
    assert_eq!(px[usize::try_from(4 * RENDER_W + 3).unwrap()], INK);
    assert_eq!(px[usize::try_from(5 * RENDER_W + 3).unwrap()], INK);
    assert_eq!(px[usize::try_from(6 * RENDER_W + 3).unwrap()], INK);
    let edges: [(i32, i32, i32); 6] = [
        (3, 37, 3),
        (3, 39, 1),
        (3, 40, 0),
        (95, 4, 3),
        (96, 4, 0),
        (-1, 4, 0),
    ];
    for (x, y, want) in edges {
        let _px = draw_fresh(RENDER_W, RENDER_H, &font, x, y, &with_nul(b"A"), INK);
        assert_eq!(tally().calls, want);
        assert_eq!(tally().outside, 0);
    }
}

#[test]
fn bitmap_bounds_exact_and_overflow() {
    let blob = [0xFFu8; 6];
    let cases: [(u16, u8, u32, i32); 11] = [
        (3, 3, 6, 24),
        (0, 3, 6, 24),
        (0, 6, 6, 48),
        (4, 3, 6, 0),
        (0, 7, 6, 0),
        (6, 1, 6, 0),
        (7, 1, 6, 0),
        (0xFFFF, 255, 6, 0),
        (0, 1, 0, 0),
        (3, 3, 5, 0),
        (5, 1, 6, 8),
    ];
    for (off, h, nbits, want) in cases {
        let glyphs = [
            Glyph {
                cp: 0x41,
                w: 8,
                h,
                xoff: 0,
                yoff: 0,
                advance: 9,
                off,
            },
            Glyph {
                cp: REPLACEMENT,
                w: 0,
                h: 0,
                xoff: 0,
                yoff: 0,
                advance: 1,
                off: 0,
            },
        ];
        let font = Font {
            ascent: 8,
            descent: 0,
            height: 8,
            nglyphs: 2,
            glyphs: glyphs.as_ptr(),
            bits: blob.as_ptr(),
            nbits,
        };
        let _px = draw_fresh(RENDER_W, RENDER_H, &font, 3, 4, &with_nul(b"A"), INK);
        assert_eq!(tally().calls, want);
        assert_eq!(tally().outside, 0);
    }
    let glyphs = [
        Glyph {
            cp: 0x41,
            w: 8,
            h: 3,
            xoff: 0,
            yoff: 0,
            advance: 9,
            off: 3,
        },
        Glyph {
            cp: REPLACEMENT,
            w: 0,
            h: 0,
            xoff: 0,
            yoff: 0,
            advance: 1,
            off: 0,
        },
    ];
    let font = Font {
        ascent: 8,
        descent: 0,
        height: 8,
        nglyphs: 2,
        glyphs: glyphs.as_ptr(),
        bits: blob.as_ptr(),
        nbits: 6,
    };
    let mut rows: *const u8 = core::ptr::null();
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_glyph_rows(&font, &glyphs[0], &mut rows) }, 1);
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(core::ptr::eq(rows, unsafe { blob.as_ptr().add(3) }));
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph_rows(core::ptr::null(), &glyphs[0], &mut rows) },
        0
    );
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph_rows(&font, core::ptr::null(), &mut rows) },
        0
    );
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph_rows(&font, &glyphs[0], core::ptr::null_mut()) },
        0
    );
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_glyph_rows(&font, &glyphs[1], &mut rows) }, 0);
    let mut empty = font;
    empty.bits = core::ptr::null();
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_glyph_rows(&empty, &glyphs[0], &mut rows) }, 0);
    for spec in SPECS {
        let font = must_font(spec.id);
        // SAFETY: police publiée.
        let view = unsafe { &*font };
        let mut k = 0i32;
        while k < view.nglyphs {
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let g = unsafe { view.glyphs.add(usize::try_from(k).unwrap()).read() };
            let stride = u32::from(g.w).div_ceil(8);
            let need = u32::from(g.h) * stride;
            assert!(u32::from(g.off) + need <= view.nbits);
            k += 1;
        }
    }
}

#[test]
fn pen_null_cut_keeps_rows_and_clips() {
    let mut px = blank(RENDER_W, RENDER_H);
    let mut dst = surface_of(&mut px, RENDER_W, RENDER_H);
    let text = with_nul(b"AB");
    let rq = req_of(must_font(FONT_UI), 3, 4, &text, -1, INK);
    let mut pen = Pen {
        dst: core::ptr::null_mut(),
        font: core::ptr::null(),
        color: 0,
        x: 0,
        y: 0,
        bounds: PenBox {
            left: 0,
            top: 0,
            right: 0,
            bottom: 0,
        },
    };
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_pen_start(core::ptr::null_mut(), &mut dst, &rq) },
        0
    );
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_pen_start(&mut pen, core::ptr::null_mut(), &rq) },
        0
    );
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_pen_start(&mut pen, &mut dst, core::ptr::null()) },
        0
    );
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_pen_start(&mut pen, &mut dst, &rq) }, 1);
    assert!(core::ptr::eq(pen.dst, &dst));
    let glyph = Glyph {
        cp: 0x41,
        w: 8,
        h: 3,
        xoff: 0,
        yoff: 0,
        advance: 9,
        off: 0,
    };
    let sentinel = 0x11u8;
    let mut cut = Cut {
        rows: &sentinel,
        x: -1,
        y: -1,
        c0: -1,
        c1: -1,
        r0: -1,
        r1: -1,
    };
    pen.bounds = PenBox {
        left: 5,
        top: 0,
        right: RENDER_W,
        bottom: RENDER_H,
    };
    pen.x = 3;
    pen.y = 4;
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph_cut(core::ptr::null(), &glyph, &mut cut) },
        0
    );
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph_cut(&pen, core::ptr::null(), &mut cut) },
        0
    );
    assert_eq!(
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_glyph_cut(&pen, &glyph, core::ptr::null_mut()) },
        0
    );
    assert!(core::ptr::eq(cut.rows, &sentinel));
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_glyph_cut(&pen, &glyph, &mut cut) }, 1);
    assert!(core::ptr::eq(cut.rows, &sentinel));
    assert_eq!(
        (cut.x, cut.y, cut.c0, cut.c1, cut.r0, cut.r1),
        (3, 4, 2, 8, 0, 3)
    );
    let overflow = Glyph {
        cp: 0x41,
        w: 8,
        h: 3,
        xoff: 1,
        yoff: 0,
        advance: 9,
        off: 0,
    };
    pen.x = i64::MAX;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert_eq!(unsafe { font_glyph_cut(&pen, &overflow, &mut cut) }, 0);
    assert!(core::ptr::eq(cut.rows, &sentinel));
    assert!(text_end(core::ptr::null(), 4).is_null());
    let exact = *b"abcX";
    let start = exact.as_ptr().cast::<c_char>();
    assert!(core::ptr::eq(text_end(start, 0), start));
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(core::ptr::eq(text_end(start, 3), unsafe { start.add(3) }));
    let longer = *b"ab\0X";
    let mid = longer.as_ptr().cast::<c_char>();
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(core::ptr::eq(text_end(mid, 4), unsafe { mid.add(2) }));
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    assert!(core::ptr::eq(text_end(mid, i32::MIN), unsafe {
        mid.add(2)
    }));
}

#[test]
fn draw_len_null_and_empty_targets() {
    let ui = must_font(FONT_UI);
    let ab = with_nul(b"AB");
    let a = with_nul(b"A");
    let mut px = blank(RENDER_W, RENDER_H);
    let rq = req_of(ui, 3, 4, &ab, 0, INK);
    reset();
    draw_on(&mut px, RENDER_W, RENDER_H, &rq);
    assert_eq!(tally().calls, 0);
    let one = {
        let mut buf = blank(RENDER_W, RENDER_H);
        let rq = req_of(ui, 3, 4, &ab, 1, INK);
        draw_on(&mut buf, RENDER_W, RENDER_H, &rq);
        buf
    };
    let only_a = draw_fresh(RENDER_W, RENDER_H, ui, 3, 4, &a, INK);
    assert_eq!(one, only_a);
    let full = draw_fresh(RENDER_W, RENDER_H, ui, 3, 4, &ab, INK);
    for len in [-1, 2, i32::MIN, 10, i32::MAX] {
        let mut buf = blank(RENDER_W, RENDER_H);
        let rq = req_of(ui, 3, 4, &ab, len, INK);
        draw_on(&mut buf, RENDER_W, RENDER_H, &rq);
        assert_eq!(buf, full);
    }
    let embedded = *b"A\0B";
    let mut buf = blank(RENDER_W, RENDER_H);
    let rq = req_of(ui, 3, 4, &embedded, 3, INK);
    draw_on(&mut buf, RENDER_W, RENDER_H, &rq);
    assert_eq!(buf, only_a);
    let cut = [0xC3u8, 0xA9, 0];
    let repl = with_nul(&[0xEF, 0xBF, 0xBD]);
    let mut left = blank(RENDER_W, RENDER_H);
    let rq = req_of(ui, 3, 4, &cut, 1, INK);
    draw_on(&mut left, RENDER_W, RENDER_H, &rq);
    assert_eq!(left, draw_fresh(RENDER_W, RENDER_H, ui, 3, 4, &repl, INK));
    let mut px = blank(RENDER_W, RENDER_H);
    let mut dst = surface_of(&mut px, RENDER_W, RENDER_H);
    let rq = req_of(ui, 3, 4, &ab, -1, INK);
    reset();
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(core::ptr::null_mut(), &rq) };
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, core::ptr::null()) };
    let mut missing = rq;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    missing.font = unsafe { font_get(FONT_IDS) };
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, &missing) };
    missing.font = ui;
    missing.text = core::ptr::null();
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, &missing) };
    assert_eq!(tally().calls, 0);
    dst.clip.w = 0;
    let again = req_of(ui, 3, 4, &ab, -1, INK);
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, &again) };
    dst.clip.w = -5;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, &again) };
    dst.clip.w = RENDER_W;
    dst.h = 0;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, &again) };
    dst.h = RENDER_H;
    dst.w = -RENDER_W;
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, &again) };
    dst.w = RENDER_W;
    dst.px = core::ptr::null_mut();
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    unsafe { font_draw(&mut dst, &again) };
    assert_eq!(tally().calls, 0);
}

#[test]
fn pixels_match_glyph_bitmap() {
    let chars: [&[u8]; 7] = [
        b"H\0",
        b"j\0",
        &[0xC3, 0xA9, 0],
        &[0xE2, 0x82, 0xAC, 0],
        &[0xC5, 0x92, 0],
        &[0xFF, 0],
        &[0xF0, 0x9F, 0x98, 0x80, 0],
    ];
    const SURF_W: i32 = 160;
    const SURF_H: i32 = 160;
    for spec in SPECS {
        let font = must_font(spec.id);
        for text in chars {
            let px = draw_fresh(SURF_W, SURF_H, font, 5, 7, text, INK);
            let mut cur = text.as_ptr().cast::<c_char>();
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let end = unsafe { cur.add(text.len()) };
            let cp = utf8_next(&mut cur, end);
            let g = must_glyph(font, cp);
            assert_eq!(tally().calls, glyph_ink(font, &g));
            assert_eq!(tally().outside, 0);
            assert_eq!(tally().repeated, 0);
            let mut bad = 0i32;
            for (pos, pixel) in px.iter().enumerate() {
                let dx = i32::try_from(pos % usize::try_from(SURF_W).unwrap()).unwrap()
                    - 5
                    - i32::from(g.xoff);
                let dy = i32::try_from(pos / usize::try_from(SURF_W).unwrap()).unwrap()
                    - 7
                    - i32::from(g.yoff);
                let on = dx >= 0
                    && dy >= 0
                    && dx < i32::from(g.w)
                    && dy < i32::from(g.h)
                    && bit_at(font, &g, dx, dy);
                if (*pixel == INK) != on {
                    bad += 1;
                }
            }
            assert_eq!(bad, 0);
        }
    }
    let mono = must_font(FONT_MONO);
    let px = draw_fresh(160, 160, mono, 5, 7, b"H\0", INK);
    assert_eq!(tally().calls, 22);
    assert_eq!(px.iter().filter(|p| **p == INK).count(), 22);
}

#[test]
fn clips_match_unclipped_mask_and_extremes_stay_inside() {
    let clips = [
        Rect {
            x: 0,
            y: 0,
            w: RENDER_W,
            h: RENDER_H,
        },
        Rect {
            x: 0,
            y: 0,
            w: 48,
            h: RENDER_H,
        },
        Rect {
            x: 48,
            y: 0,
            w: 48,
            h: RENDER_H,
        },
        Rect {
            x: 0,
            y: 10,
            w: RENDER_W,
            h: 3,
        },
        Rect {
            x: 20,
            y: 12,
            w: 1,
            h: 1,
        },
        Rect {
            x: -50,
            y: -50,
            w: 400,
            h: 400,
        },
        Rect {
            x: 70,
            y: 0,
            w: 100,
            h: 100,
        },
        Rect {
            x: -10,
            y: -10,
            w: 20,
            h: 20,
        },
        Rect {
            x: 30,
            y: 5,
            w: 0,
            h: 20,
        },
        Rect {
            x: 30,
            y: 5,
            w: 20,
            h: -4,
        },
        Rect {
            x: 95,
            y: 39,
            w: 5,
            h: 5,
        },
    ];
    let text = with_nul("Poste de travail Éteindre €".as_bytes());
    let mono = must_font(FONT_MONO);
    let plain = draw_fresh(RENDER_W, RENDER_H, mono, -3, 2, &text, INK);
    for clip in clips {
        let mut px = blank(RENDER_W, RENDER_H);
        let mut dst = surface_of(&mut px, RENDER_W, RENDER_H);
        dst.clip = clip;
        let rq = req_of(mono, -3, 2, &text, -1, INK);
        reset();
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_draw(&mut dst, &rq) };
        assert_eq!(tally().outside, 0);
        for (pos, pixel) in px.iter().enumerate() {
            let x = i64::try_from(pos % usize::try_from(RENDER_W).unwrap()).unwrap();
            let y = i64::try_from(pos / usize::try_from(RENDER_W).unwrap()).unwrap();
            let inside_clip = x >= i64::from(clip.x)
                && y >= i64::from(clip.y)
                && x < i64::from(clip.x) + i64::from(clip.w)
                && y < i64::from(clip.y) + i64::from(clip.h);
            let want = if inside_clip { plain[pos] } else { PAPER };
            assert_eq!(*pixel, want);
        }
    }
    let places = [
        Point {
            x: i32::MIN,
            y: i32::MIN,
        },
        Point {
            x: i32::MAX,
            y: i32::MAX,
        },
        Point {
            x: i32::MAX - 3,
            y: 5,
        },
        Point {
            x: -1_000_000,
            y: 5,
        },
        Point { x: 5, y: i32::MAX },
        Point { x: 5, y: i32::MIN },
        Point { x: i32::MIN, y: 0 },
        Point { x: -40, y: -6 },
    ];
    let mut px = blank(RENDER_W, RENDER_H);
    let mut dst = surface_of(&mut px, RENDER_W, RENDER_H);
    let ui = must_font(FONT_UI);
    let sample = with_nul("Démarrer €".as_bytes());
    reset();
    for place in places {
        let rq = TextReq {
            font: ui,
            at: place,
            color: INK,
            text: sample.as_ptr().cast(),
            len: -1,
        };
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        unsafe { font_draw(&mut dst, &rq) };
    }
    assert_eq!(tally().outside, 0);
}

#[test]
fn color_passes_through_and_draw_is_additive() {
    let bold = must_font(FONT_UI_BOLD);
    let text = with_nul("Hé".as_bytes());
    let px = draw_fresh(RENDER_W, RENDER_H, bold, 3, 4, &text, 0x0011_2233);
    let mut ink = 0i32;
    for px in &px {
        assert!(*px == PAPER || *px == 0x0011_2233);
        ink += i32::from(*px == 0x0011_2233);
    }
    assert!(ink > 10);
    let pairs: [(&[u8], &[u8]); 8] = [
        (b"A", b"B"),
        ("é".as_bytes(), "€".as_bytes()),
        (b"W", b"j"),
        (b"f", b"i"),
        ("À".as_bytes(), "É".as_bytes()),
        (b"l", b"'"),
        (b"T", b"."),
        (&[0xFF], b"x"),
    ];
    for spec in SPECS {
        let font = must_font(spec.id);
        for (a, b) in pairs {
            let mut both = Vec::new();
            both.extend_from_slice(a);
            both.extend_from_slice(b);
            both.push(0);
            let together = draw_fresh(RENDER_W, RENDER_H, font, 3, 4, &both, INK);
            let mut split = blank(RENDER_W, RENDER_H);
            let left = with_nul(a);
            let right = with_nul(b);
            let rq = req_of(font, 3, 4, &left, -1, INK);
            draw_on(&mut split, RENDER_W, RENDER_H, &rq);
            let adv = text_width(font, &left, -1);
            let mut dst = surface_of(&mut split, RENDER_W, RENDER_H);
            let rq = TextReq {
                font,
                at: Point { x: 3 + adv, y: 4 },
                color: INK,
                text: right.as_ptr().cast(),
                len: -1,
            };
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            unsafe { font_draw(&mut dst, &rq) };
            assert_eq!(together, split);
        }
    }
    let broken: [(&[u8], &[u8]); 5] = [
        (&[0xFF], &[0xEF, 0xBF, 0xBD]),
        (&[0xE2, 0x82], &[0xEF, 0xBF, 0xBD]),
        (&[0xC0, 0x80], &[0xEF, 0xBF, 0xBD, 0xEF, 0xBF, 0xBD]),
        (
            &[0xED, 0xA0, 0x80],
            &[0xEF, 0xBF, 0xBD, 0xEF, 0xBF, 0xBD, 0xEF, 0xBF, 0xBD],
        ),
        (
            &[b'a', 0xF4, 0x90, 0x80, 0x80],
            &[
                b'a', 0xEF, 0xBF, 0xBD, 0xEF, 0xBF, 0xBD, 0xEF, 0xBF, 0xBD, 0xEF, 0xBF, 0xBD,
            ],
        ),
    ];
    for spec in SPECS {
        let font = must_font(spec.id);
        for (bad, good) in broken {
            let left = draw_fresh(RENDER_W, RENDER_H, font, 3, 4, &with_nul(bad), INK);
            let right = draw_fresh(RENDER_W, RENDER_H, font, 3, 4, &with_nul(good), INK);
            assert_eq!(left, right);
        }
    }
}

#[test]
fn shape_budget_and_negative_xoff_edge() {
    let flat = b"HIEFLTxzvwkn";
    let mut total = 0i64;
    for spec in SPECS {
        let font_ptr = must_font(spec.id);
        // SAFETY: police publiée.
        let font = unsafe { &*font_ptr };
        total += i64::from(font.nglyphs) * i64::try_from(size_of::<Glyph>()).unwrap();
        total += i64::try_from(size_of::<Font>()).unwrap();
        total += i64::from(font.nbits);
        let mut k = 0i32;
        while k < font.nglyphs {
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let g = unsafe { font.glyphs.add(usize::try_from(k).unwrap()).read() };
            assert!(g.advance >= 1);
            assert_eq!(g.w == 0, g.h == 0);
            let mut rows: *const u8 = core::ptr::null();
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let has_rows = unsafe { font_glyph_rows(font_ptr, &g, &mut rows) };
            assert_eq!(g.w == 0, has_rows == 0);
            if g.h != 0 {
                assert!(g.yoff >= 0);
                assert!(i32::from(g.yoff) + i32::from(g.h) <= font.height);
                assert!(g.xoff >= -4);
                assert!(i32::from(g.xoff) + i32::from(g.w) <= i32::from(g.advance) + 4);
            }
            k += 1;
        }
        for byte in flat {
            let g = must_glyph(font_ptr, u32::from(*byte));
            assert_eq!(i32::from(g.yoff) + i32::from(g.h), font.ascent);
        }
        let box_g = must_glyph(font_ptr, REPLACEMENT);
        let mut row = 0i32;
        while row < i32::from(box_g.h) {
            let mut col = 0i32;
            while col < i32::from(box_g.w) {
                let border = row == 0
                    || row == i32::from(box_g.h) - 1
                    || col == 0
                    || col == i32::from(box_g.w) - 1;
                assert_eq!(bit_at(font_ptr, &box_g, col, row), border);
                col += 1;
            }
            row += 1;
        }
    }
    assert!(total < 81920);
    // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
    let mono = unsafe { &*must_font(FONT_MONO) };
    assert_eq!(mono.height, 16);
    let mut k = 0i32;
    while k < mono.nglyphs {
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        let g = unsafe { mono.glyphs.add(usize::try_from(k).unwrap()).read() };
        assert_eq!(g.advance, 8);
        assert!(g.xoff >= 0);
        assert!(i32::from(g.xoff) + i32::from(g.w) <= 8);
        k += 1;
    }
    let mut id = 0;
    while id < FONT_IDS {
        let font_ptr = must_font(id);
        // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
        let font = unsafe { &*font_ptr };
        let mut k = 0i32;
        while k < font.nglyphs {
            // SAFETY: pointeurs du test, vivants et bornés avant l'appel.
            let g = unsafe { font.glyphs.add(usize::try_from(k).unwrap()).read() };
            if g.xoff < 0 && g.cp != REPLACEMENT {
                let mut enc = [0u8; 5];
                let len = ref_encode(g.cp, (&mut enc[..4]).try_into().unwrap());
                enc[len] = 0;
                let mut visible = 0i32;
                let mut row = 0i32;
                while row < i32::from(g.h) {
                    let mut col = 0i32;
                    while col < i32::from(g.w) && col < -i32::from(g.xoff) {
                        visible += i32::from(bit_at(font_ptr, &g, col, row));
                        col += 1;
                    }
                    row += 1;
                }
                let _px = draw_fresh(RENDER_W, RENDER_H, font_ptr, RENDER_W, 2, &enc, INK);
                assert_eq!(tally().calls, visible);
                assert_eq!(tally().outside, 0);
            }
            k += 1;
        }
        id += 1;
    }
}
