use core::ffi::c_char;

use crate::abi::{
    CTL_ALIGN_CENTER, CTL_ALIGN_RIGHT, CTL_BOLD, CTL_ELLIPSIS, CTL_ST_HOT, CTL_ST_PRESSED,
    CTL_TEXT_MAX, FONT_IDS, FONT_UI, FONT_UI_BOLD, LS_DISABLED, LS_HOT, LS_NORMAL, LS_PRESSED,
    t_color, t_ctl, t_ctltext, t_point, t_surface, t_textrun,
};
use crate::ffi::{font_draw, font_fit, font_get, font_text_width, gfx_hline};
use crate::raw::{bytes, scopy, scopy_distinct, slen};
use crate::utf8::ctl_u8_len;
use crate::utf8::ctl_u8_next;

fn at(s: &[u8], i: u32) -> u8 {
    if i as usize >= s.len() {
        0
    } else {
        s[i as usize]
    }
}

/// # Safety
/// `dst` a `CTL_TEXT_MAX` octets. `src` est nul ou une chaîne C terminée.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_copy_text(dst: *mut c_char, src: *const c_char) {
    let len = slen(src);
    let mut pos = 0u32;
    while pos < len {
        // SAFETY: src couvre len octets, pos < len.
        let next = unsafe { ctl_u8_next(src, len, pos) };
        if next > CTL_TEXT_MAX as u32 - 1 {
            break;
        }
        pos = next;
    }
    if pos > 0 {
        scopy(dst.cast(), src.cast(), pos as usize);
    }
    // SAFETY: dst a au moins pos+1 octets, pos < CTL_TEXT_MAX.
    unsafe { dst.add(pos as usize).write(0) };
}

fn strip_amp(text: &[u8], i: &mut u32, mn: &mut i32, o: i32) {
    *i += 1;
    if *mn < 0 && at(text, *i) != 0 {
        *mn = o;
    }
}

/// # Safety
/// `out` a `CTL_TEXT_MAX` octets. `text` est nul ou terminé. `mn` est non nul et inscriptible.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_text_strip(
    text: *const c_char,
    out: *mut c_char,
    mn: *mut i32,
) -> i32 {
    let len = slen(text);
    let src = bytes(text, len);
    let mut i = 0u32;
    let mut o = 0i32;
    // SAFETY: mn est non nul et inscriptible.
    let mn = unsafe { &mut *mn };
    *mn = -1;
    while i < len && o < CTL_TEXT_MAX as i32 - 1 {
        if at(src, i) == b'&' && at(src, i + 1) == b'&' {
            i += 1;
        } else if at(src, i) == b'&' {
            strip_amp(src, &mut i, mn, o);
            continue;
        }
        // SAFETY: i < len, text couvre len octets.
        let next = unsafe { ctl_u8_next(text, len, i) };
        if o + (next - i) as i32 > CTL_TEXT_MAX as i32 - 1 {
            break;
        }
        let dst = out.wrapping_add(o as usize);
        let srcp = text.wrapping_add(i as usize);
        scopy_distinct(dst.cast(), srcp.cast(), (next - i) as usize);
        o += (next - i) as i32;
        i = next;
    }
    // SAFETY: out a o+1 octets, o < CTL_TEXT_MAX.
    unsafe { out.add(o as usize).write(0) };
    o
}

/// # Safety
/// `text` est nul ou une chaîne C terminée.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnemonic(text: *const c_char) -> i32 {
    let mut buf = [0u8; CTL_TEXT_MAX];
    let mut mn = 0i32;
    // SAFETY: buf a CTL_TEXT_MAX octets, mn est local.
    unsafe { ctl_text_strip(text, buf.as_mut_ptr().cast(), &mut mn) };
    if mn < 0 {
        return 0;
    }
    let ch = buf[mn as usize];
    if ch.is_ascii_lowercase() {
        return i32::from(ch - b'a' + b'A');
    }
    if ch.is_ascii_uppercase() || ch.is_ascii_digit() {
        return i32::from(ch);
    }
    0
}

/// # Safety
/// Aucun pointeur. `flags` est un masque de contrôle.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_font(flags: u32) -> *const crate::abi::t_font {
    if flags & CTL_BOLD != 0 {
        return font_get(FONT_UI_BOLD);
    }
    font_get(FONT_UI)
}

/// # Safety
/// `c` est non nul et vivant.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_font_of(c: *const t_ctl) -> *const crate::abi::t_font {
    // SAFETY: c est non nul.
    let c = unsafe { &*c };
    if c.font > 0 && c.font <= FONT_IDS {
        return font_get((c.font - 1) as i32);
    }
    // SAFETY: pas de pointeur au-delà de l'entier.
    unsafe { ctl_font(c.flags) }
}

/// # Safety
/// `tx` et `c` sont non nuls. `tx` est inscriptible. `c` est vivant.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_text_init(tx: *mut t_ctltext, c: *const t_ctl) {
    // SAFETY: les deux pointeurs sont non nuls.
    let c = unsafe { &*c };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let tx = unsafe { &mut *tx };
    tx.r#box = c.rect;
    tx.text = c.text_c();
    tx.flags = c.flags;
    // SAFETY: c est vivant, la chaîne de parents aussi.
    tx.enabled = unsafe { crate::tree::ctl_enabled(c as *const t_ctl) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    tx.color = unsafe { crate::color::ctl_color_text(tx.enabled) };
    tx.mnemonic = true;
    tx.font = core::ptr::null();
    if c.font != 0 {
        // SAFETY: c est non nul.
        tx.font = unsafe { ctl_font_of(c) };
    }
}

/// # Safety
/// `run` et `tx` sont non nuls. `run` est inscriptible. `tx.text` est nul ou terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_run_init(run: *mut t_textrun, tx: *const t_ctltext) -> bool {
    // SAFETY: pointeurs non nuls.
    let tx = unsafe { &*tx };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let run = unsafe { &mut *run };
    run.font = tx.font;
    if run.font.is_null() {
        // SAFETY: flags est un entier.
        run.font = unsafe { ctl_font(tx.flags) };
    }
    if run.font.is_null() || tx.text.is_null() {
        return false;
    }
    run.mn = -1;
    if tx.mnemonic {
        // SAFETY: run.buf a CTL_TEXT_MAX octets, tx.text est terminé, mn est local au run.
        run.len = unsafe { ctl_text_strip(tx.text, run.buf.as_mut_ptr().cast(), &mut run.mn) };
    } else {
        // SAFETY: run.buf a CTL_TEXT_MAX octets, tx.text est terminé.
        unsafe { ctl_copy_text(run.buf.as_mut_ptr().cast(), tx.text) };
        run.len = slen(run.buf.as_ptr().cast()) as i32;
    }
    run.cut = run.len;
    run.ellipsis = false;
    true
}

/// # Safety
/// `run` et `tx` sont non nuls. `run.font` est un `t_font` vivant. `run.buf` est terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_run_fit(run: *mut t_textrun, tx: *const t_ctltext) {
    // SAFETY: pointeurs non nuls.
    let tx = unsafe { &*tx };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let run = unsafe { &mut *run };
    run.width = font_text_width(run.font, run.buf.as_ptr().cast(), run.len);
    if run.width <= tx.r#box.w {
        return;
    }
    let mut ell = 0i32;
    let mut room = tx.r#box.w;
    run.ellipsis = tx.flags & CTL_ELLIPSIS != 0;
    if run.ellipsis {
        ell = font_text_width(run.font, b"\xe2\x80\xa6".as_ptr().cast(), 3);
        if ell > room {
            run.ellipsis = false;
            ell = 0;
        }
    }
    room -= ell;
    if room < 0 {
        room = 0;
    }
    run.cut = font_fit(run.font, run.buf.as_ptr().cast(), room);
    if run.cut > run.len {
        run.cut = run.len;
    }
    run.width = font_text_width(run.font, run.buf.as_ptr().cast(), run.cut) + ell;
}

/// # Safety
/// `run` et `tx` sont non nuls. `run.font` est vivant.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_run_place(run: *mut t_textrun, tx: *const t_ctltext) {
    // SAFETY: pointeurs non nuls.
    let tx = unsafe { &*tx };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let run = unsafe { &mut *run };
    run.at.x = tx.r#box.x;
    if tx.flags & CTL_ALIGN_CENTER != 0 {
        run.at.x += (tx.r#box.w - run.width) / 2;
    } else if tx.flags & CTL_ALIGN_RIGHT != 0 {
        run.at.x += tx.r#box.w - run.width;
    }
    if run.at.x < tx.r#box.x {
        run.at.x = tx.r#box.x;
    }
    // SAFETY: font n'est pas nul, posé par ctl_run_init.
    let height = unsafe { (*run.font).height };
    run.at.y = tx.r#box.y + (tx.r#box.h - height) / 2;
}

/// # Safety
/// `tx` est non nul. `tx.text` est nul ou terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_text_width(tx: *const t_ctltext) -> i32 {
    let mut run = core::mem::MaybeUninit::<t_textrun>::uninit();
    // SAFETY: run est une place locale, tx suit le contrat de ctl_run_init.
    if !unsafe { ctl_run_init(run.as_mut_ptr(), tx) } {
        return 0;
    }
    // SAFETY: ctl_run_init a écrit font, buf et len. at et width ne sont pas lus.
    let run = unsafe { &*run.as_ptr() };
    font_text_width(run.font, run.buf.as_ptr().cast(), run.len)
}

fn run_text(s: *mut t_surface, run: &t_textrun, col: t_color, off: i32) {
    use crate::abi::t_textreq;
    let mut rq = t_textreq {
        font: run.font,
        at: t_point {
            x: run.at.x + off,
            y: run.at.y + off,
        },
        color: col,
        text: run.buf.as_ptr().cast(),
        len: run.cut,
    };
    font_draw(s, &rq);
    if !run.ellipsis {
        return;
    }
    rq.at.x += font_text_width(run.font, run.buf.as_ptr().cast(), run.cut);
    rq.text = b"\xe2\x80\xa6".as_ptr().cast();
    rq.len = 3;
    font_draw(s, &rq);
}

fn run_underline(s: *mut t_surface, run: &t_textrun, col: t_color, off: i32) {
    if run.mn < 0 || run.mn >= run.cut {
        return;
    }
    let p = t_point {
        x: run.at.x + off + font_text_width(run.font, run.buf.as_ptr().cast(), run.mn),
        // SAFETY: font n'est pas nul.
        y: run.at.y + off + unsafe { (*run.font).ascent } + 1,
    };
    // SAFETY: mn < cut, buf couvre cut octets.
    let cl = unsafe {
        ctl_u8_len(
            run.buf.as_ptr().cast::<c_char>().add(run.mn as usize),
            (run.cut - run.mn) as u32,
        )
    } as i32;
    gfx_hline(
        s,
        p,
        font_text_width(
            run.font,
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { run.buf.as_ptr().cast::<c_char>().add(run.mn as usize) },
            cl,
        ),
        col,
    );
}

/// # Safety
/// `s` est une surface vivante. `tx` est non nul, son texte est nul ou terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_text_draw(s: *mut t_surface, tx: *const t_ctltext) {
    let mut run = core::mem::MaybeUninit::<t_textrun>::uninit();
    // SAFETY: run est local, tx suit ctl_run_init.
    if !unsafe { ctl_run_init(run.as_mut_ptr(), tx) } {
        return;
    }
    // SAFETY: run est initialisé.
    let run = unsafe { &mut *run.as_mut_ptr() };
    // SAFETY: run et tx sont vivants.
    unsafe { ctl_run_fit(run, tx) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_run_place(run, tx) };
    // SAFETY: tx est non nul.
    let tx = unsafe { &*tx };
    if !tx.enabled {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        run_text(s, run, unsafe { crate::color::ctl_color_shadow() }, 1);
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        run_underline(s, run, unsafe { crate::color::ctl_color_shadow() }, 1);
    }
    run_text(s, run, tx.color, 0);
    run_underline(s, run, tx.color, 0);
}

/// # Safety
/// `c` est non nul. La chaîne des parents est finie.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lstate(c: *const t_ctl) -> i32 {
    // SAFETY: c est non nul, les parents aussi.
    if unsafe { !crate::tree::ctl_enabled(c) } {
        return LS_DISABLED;
    }
    // SAFETY: c est non nul.
    let c = unsafe { &*c };
    if c.state & CTL_ST_PRESSED != 0 {
        return LS_PRESSED;
    }
    if c.state & CTL_ST_HOT != 0 {
        return LS_HOT;
    }
    LS_NORMAL
}
