use core::ffi::{c_char, c_void};

use crate::abi::{
    CN_CHANGED, CT_BUTTON, CT_EDIT, CT_LIST, CTL_COORD_MAX, CTL_ENABLED, CTL_FOCUSABLE,
    CTL_MI_SEPARATOR, CTL_TEXT_MAX, CTL_THUMB_MIN, CTL_VISIBLE, E_INVAL, E_RANGE, INP_CHAR,
    INP_KEY_DOWN, INPM_CTRL, VK_BACK, t_ctl, t_ctlmenuitem, t_ctlmenuspec, t_ctlroot, t_ctlspec,
    t_edit, t_inpevent, t_lunametrics, t_point, t_rect, t_sblo, t_scrst, t_surface, t_wmmouse,
};
use crate::clip::{ctl_clip_clear, ctl_clip_get, ctl_clip_set};
use crate::color::{ctl_color_back, ctl_color_selected_text, ctl_color_shadow, ctl_color_text};
use crate::edit::ctl_edit_set_limit;
use crate::ffi::{luna_metrics, rect_contains, rect_empty, rect_make};
use crate::focus::ctl_focus;
use crate::key::ctl_key;
use crate::list::{
    ctl_list_add, ctl_list_count, ctl_list_select, ctl_list_selected, ctl_list_text,
};
use crate::menu::{ctl_menu_close, ctl_menu_popup, ctl_menu_result, ctl_mnu_item_h, ctl_mnu_pool};
use crate::mouse::{ctl_mouse, ctl_wheel_delta};
use crate::paint::ctl_paint;
use crate::raw::slen;
use crate::scroll::{
    ctl_sb_layout, ctl_sb_pos_from, ctl_sb_set, ctl_sb_vertical, ctl_sb_zone, ctl_scroll_move,
    ctl_scroll_pos, ctl_scroll_set,
};
use crate::text::{ctl_copy_text, ctl_mnemonic, ctl_text_strip};
use crate::tree::{
    ctl_add, ctl_find, ctl_rect_ok, ctl_root_destroy, ctl_root_init, ctl_set_rect, ctl_set_text,
};
use crate::utf8::{ctl_u8_count, ctl_u8_encode, ctl_u8_len, ctl_u8_next, ctl_u8_prev};
use crate::widgets::ctl_progress_set;

unsafe extern "C" fn on_changed(_c: *mut c_void, code: u32, user: *mut c_void) {
    if code == CN_CHANGED && !user.is_null() {
        // SAFETY: user pointe le compteur du test, vivant pendant l'appel.
        unsafe { *user.cast::<u32>() += 1 };
    }
}

struct Desk {
    px: Vec<u32>,
    surface: Box<t_surface>,
    root: t_ctlroot,
}

fn desk(w: i32, h: i32, changed: *mut u32) -> Desk {
    let mut px = vec![0u32; (w as usize) * (h as usize)];
    let surface = Box::new(t_surface {
        px: px.as_mut_ptr(),
        w,
        h,
        stride: w,
        clip: rect_make(0, 0, w, h),
    });
    let mut root = t_ctlroot {
        root: core::ptr::null_mut(),
        focus: core::ptr::null_mut(),
        hot: core::ptr::null_mut(),
        capture: core::ptr::null_mut(),
        surface: core::ptr::null_mut(),
        dirty: crate::abi::t_region {
            r: [rect_make(0, 0, 0, 0); 32],
            n: 0,
        },
        command: None,
        user: core::ptr::null_mut(),
    };
    // SAFETY: surface et root sont locaux et vivants.
    let rc = unsafe {
        ctl_root_init(
            &mut root,
            &*surface as *const t_surface as *mut t_surface,
            Some(on_changed),
        )
    };
    assert_eq!(rc, 0);
    root.user = changed.cast();
    Desk { px, surface, root }
}

fn shut(desk: &mut Desk) {
    assert!(!desk.px.is_empty());
    assert_eq!(desk.surface.px, desk.px.as_mut_ptr());
    // SAFETY: root a été initialisé par ctl_root_init.
    unsafe { ctl_root_destroy(&mut desk.root) };
}

fn spec(ty: u32, id: u32, rect: t_rect, text: *const c_char) -> t_ctlspec {
    let mut flags = CTL_VISIBLE | CTL_ENABLED;
    if ty == CT_BUTTON || ty == CT_EDIT || ty == CT_LIST {
        flags |= CTL_FOCUSABLE;
    }
    t_ctlspec {
        r#type: ty,
        id,
        flags,
        rect,
        text,
    }
}

fn add(desk: &mut Desk, sp: &t_ctlspec) -> *mut t_ctl {
    // SAFETY: la racine est un panneau, sp est vivant.
    unsafe { ctl_add(&mut desk.root, desk.root.root, sp) }
}

fn press(desk: &mut Desk, code: u32, mods: u32) -> bool {
    let ev = t_inpevent {
        r#type: INP_KEY_DOWN,
        code,
        x: 0,
        y: 0,
        mods,
        scancode: 0,
        time_ns: 0,
    };
    // SAFETY: root et ev sont vivants.
    unsafe { ctl_key(&mut desk.root, &ev) }
}

fn type_cp(desk: &mut Desk, cp: u32) -> bool {
    let ev = t_inpevent {
        r#type: INP_CHAR,
        code: cp,
        x: 0,
        y: 0,
        mods: 0,
        scancode: 0,
        time_ns: 0,
    };
    // SAFETY: root et ev sont vivants.
    unsafe { ctl_key(&mut desk.root, &ev) }
}

fn type_utf8(desk: &mut Desk, bytes: &[u8]) {
    let mut i = 0usize;
    while i < bytes.len() && bytes[i] != 0 {
        let b = bytes[i];
        let (cp, n) = if b < 0x80 {
            (b as u32, 1)
        } else if b < 0xe0 {
            (((b as u32 & 0x1f) << 6) | (bytes[i + 1] as u32 & 0x3f), 2)
        } else if b < 0xf0 {
            (
                ((b as u32 & 0x0f) << 12)
                    | ((bytes[i + 1] as u32 & 0x3f) << 6)
                    | (bytes[i + 2] as u32 & 0x3f),
                3,
            )
        } else {
            (
                ((b as u32 & 0x07) << 18)
                    | ((bytes[i + 1] as u32 & 0x3f) << 12)
                    | ((bytes[i + 2] as u32 & 0x3f) << 6)
                    | (bytes[i + 3] as u32 & 0x3f),
                4,
            )
        };
        type_cp(desk, cp);
        i += n;
    }
}

fn bytes_at(p: *const c_char) -> Vec<u8> {
    let n = slen(p) as usize;
    // SAFETY: p est terminé, slen octets précèdent le NUL.
    unsafe { core::slice::from_raw_parts(p.cast::<u8>(), n).to_vec() }
}

fn chars_at(p: *const c_char) -> u32 {
    let n = slen(p);
    // SAFETY: p couvre n octets.
    unsafe { ctl_u8_count(p, n) }
}

#[test]
fn utf8_partitions_et_limites() {
    let cases: &[(&[u8], u32, u32)] = &[
        (b"a", 1, 1),
        ("é".as_bytes(), 2, 2),
        ("€".as_bytes(), 3, 3),
        ("😀".as_bytes(), 4, 4),
        (b"a", 0, 0),
        (b"\xc0\x80", 2, 1),
        (b"\xc1\xbf", 2, 1),
        (b"\xe0\x80\x80", 3, 1),
        (b"\xf0\x80\x80\x80", 4, 1),
        (b"\xed\xa0\x80", 3, 1),
        (b"\xed\xbf\xbf", 3, 1),
        (b"\xf4\x90\x80\x80", 4, 1),
        (b"\xf5\x80\x80\x80", 4, 1),
        (b"\x80", 1, 1),
        (b"\xe2\x82", 2, 1),
        (b"\xf0\x9f\x98", 3, 1),
        (b"\xf4\x8f\xbf\xbf", 4, 4),
    ];
    for (s, avail, want) in cases {
        // SAFETY: s couvre avail octets, ou avail vaut 0.
        let got = unsafe { ctl_u8_len(s.as_ptr().cast(), *avail) };
        assert_eq!(got, *want);
    }
    let cps = [
        0x7fu32, 0x80, 0x7ff, 0x800, 0xd7ff, 0xe000, 0xffff, 0x10000, 0x10ffff, 0,
    ];
    let ns = [1u32, 2, 2, 3, 3, 3, 3, 4, 4, 1];
    for (cp, n) in cps.iter().zip(ns) {
        let mut b = [0u8; 5];
        // SAFETY: b a 5 octets.
        let got = unsafe { ctl_u8_encode(*cp, b.as_mut_ptr().cast()) };
        assert_eq!(got, n);
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let back = unsafe { ctl_u8_len(b.as_ptr().cast(), 4) };
        assert_eq!(back, n);
    }
    let mut kept = [0xabu8; 4];
    // SAFETY: kept a 4 octets. Les refus n'écrivent pas.
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_u8_encode(0xd800, kept.as_mut_ptr().cast()) },
        0
    );
    assert_eq!(kept[0], 0xab);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_u8_encode(0xdfff, kept.as_mut_ptr().cast()) },
        0
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_u8_encode(0x110000, kept.as_mut_ptr().cast()) },
        0
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_u8_encode(0xffff_ffff, kept.as_mut_ptr().cast()) },
        0
    );
    assert_eq!(kept, [0xab; 4]);
    let s = "aé€😀z".as_bytes();
    assert_eq!(s.len(), 11);
    // SAFETY: s couvre 11 octets.
    assert_eq!(unsafe { ctl_u8_count(s.as_ptr().cast(), 11) }, 5);
    let mut pos = 0u32;
    let mut n = 0u32;
    while pos < 11 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let next = unsafe { ctl_u8_next(s.as_ptr().cast(), 11, pos) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        assert_eq!(unsafe { ctl_u8_prev(s.as_ptr().cast(), next) }, pos);
        pos = next;
        n += 1;
    }
    assert_eq!(n, 5);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_u8_next(s.as_ptr().cast(), 11, 11) }, 11);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_u8_prev(s.as_ptr().cast(), 0) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_u8_prev(b"\x80\x80".as_ptr().cast(), 2) }, 1);
}

#[test]
fn texte_strip_mnemonic_et_copie() {
    let mut out = [0u8; CTL_TEXT_MAX];
    let mut mn = 0i32;
    // SAFETY: out a CTL_TEXT_MAX octets, mn est local.
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_text_strip(c"&Fichier".as_ptr(), out.as_mut_ptr().cast(), &mut mn) },
        7
    );
    assert_eq!(&out[..7], b"Fichier");
    assert_eq!(mn, 0);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_text_strip(c"Ou&vrir".as_ptr(), out.as_mut_ptr().cast(), &mut mn) },
        6
    );
    assert_eq!(mn, 2);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_text_strip(c"A&&B".as_ptr(), out.as_mut_ptr().cast(), &mut mn) };
    assert_eq!(&out[..3], b"A&B");
    assert_eq!(mn, -1);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_text_strip(c"a&b&c".as_ptr(), out.as_mut_ptr().cast(), &mut mn) };
    assert_eq!(&out[..3], b"abc");
    assert_eq!(mn, 1);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_text_strip(c"fin&".as_ptr(), out.as_mut_ptr().cast(), &mut mn) };
    assert_eq!(&out[..3], b"fin");
    assert_eq!(mn, -1);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_mnemonic(c"&Ok".as_ptr()) }, i32::from(b'O'));
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnemonic(c"a&nnuler".as_ptr()) },
        i32::from(b'N')
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_mnemonic(c"&1 un".as_ptr()) }, i32::from(b'1'));
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnemonic(c"&\xc3\xa9t\xc3\xa9".as_ptr()) },
        0
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_mnemonic(c"&.x".as_ptr()) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_mnemonic(c"Ok".as_ptr()) }, 0);
    let mut big = [b'a'; 400];
    big[399] = 0;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_copy_text(out.as_mut_ptr().cast(), big.as_ptr().cast()) };
    assert_eq!(slen(out.as_ptr().cast()), 255);
    big[254..257].copy_from_slice("€".as_bytes());
    big[257] = 0;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_copy_text(out.as_mut_ptr().cast(), big.as_ptr().cast()) };
    assert_eq!(slen(out.as_ptr().cast()), 254);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_copy_text(out.as_mut_ptr().cast(), core::ptr::null()) };
    assert_eq!(out[0], 0);
    let mut same = *b"ab\0";
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_copy_text(same.as_mut_ptr().cast(), same.as_ptr().cast()) };
    assert_eq!(&same, b"ab\0");
    let mut src = [b'a'; CTL_TEXT_MAX + 8];
    src[254..257].copy_from_slice("€".as_bytes());
    src[257] = 0;
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_text_strip(src.as_ptr().cast(), out.as_mut_ptr().cast(), &mut mn) },
        254
    );
    src[253..255].copy_from_slice("é".as_bytes());
    src[255] = 0;
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_text_strip(src.as_ptr().cast(), out.as_mut_ptr().cast(), &mut mn) },
        255
    );
    src[252..256].copy_from_slice("😀".as_bytes());
    src[256] = 0;
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_text_strip(src.as_ptr().cast(), out.as_mut_ptr().cast(), &mut mn) },
        252
    );
    src[0] = b'&';
    src[1..254].fill(b'a');
    src[254..257].copy_from_slice("€".as_bytes());
    src[257] = 0;
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_text_strip(src.as_ptr().cast(), out.as_mut_ptr().cast(), &mut mn) },
        253
    );
    assert_eq!(mn, 0);
}

#[test]
fn clip_bornes_et_collage() {
    let mut out = [0u8; CTL_TEXT_MAX];
    let big = [b'z'; 300];
    // SAFETY: les tampons sont locaux. Un seul test touche le presse-papiers.
    unsafe { ctl_clip_clear() };
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_clip_get(out.as_mut_ptr().cast(), CTL_TEXT_MAX as u32) },
        0
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_set(big.as_ptr().cast(), 255) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_set(big.as_ptr().cast(), 256) }, E_RANGE);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_clip_get(out.as_mut_ptr().cast(), CTL_TEXT_MAX as u32) },
        255
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_set(core::ptr::null(), 3) }, E_INVAL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_set(core::ptr::null(), 0) }, 0);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_clip_get(out.as_mut_ptr().cast(), CTL_TEXT_MAX as u32) },
        0
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_get(core::ptr::null_mut(), 4) }, E_INVAL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_get(out.as_mut_ptr().cast(), 0) }, E_INVAL);
    let euro = "é€".as_bytes();
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_set(euro.as_ptr().cast(), 5) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_get(out.as_mut_ptr().cast(), 6) }, 5);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_get(out.as_mut_ptr().cast(), 5) }, 2);
    assert_eq!(&out[..2], "é".as_bytes());
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_get(out.as_mut_ptr().cast(), 3) }, 2);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_get(out.as_mut_ptr().cast(), 2) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_clip_get(out.as_mut_ptr().cast(), 1) }, 0);
    assert_eq!(out[0], 0);
    let mut changed = 0u32;
    let mut d = desk(200, 80, &mut changed);
    let empty = b"\0";
    let sp_a = spec(CT_EDIT, 1, rect_make(5, 5, 150, 20), empty.as_ptr().cast());
    let a = add(&mut d, &sp_a);
    // SAFETY: a est l'edit. Le texte est recopié avant le collage.
    unsafe { ctl_set_text(&mut d.root, a, c"abc".as_ptr()) };
    let sp_b = spec(CT_EDIT, 2, rect_make(5, 40, 100, 20), empty.as_ptr().cast());
    let b = add(&mut d, &sp_b);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_focus(&mut d.root, a) };
    press(&mut d, b'A' as u32, INPM_CTRL);
    press(&mut d, b'C' as u32, INPM_CTRL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_focus(&mut d.root, b) };
    press(&mut d, b'V' as u32, INPM_CTRL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(bytes_at(unsafe { (*b).text.as_ptr().cast() }), b"abc");
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(bytes_at(unsafe { (*a).text.as_ptr().cast() }), b"abc");
    shut(&mut d);
}

#[test]
fn edit_limite_caracteres_et_octets() {
    let mut changed = 0u32;
    let mut d = desk(200, 40, &mut changed);
    let empty = b"\0";
    let sp = spec(CT_EDIT, 1, rect_make(5, 5, 150, 20), empty.as_ptr().cast());
    let e = add(&mut d, &sp);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_focus(&mut d.root, e) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_edit_set_limit(&mut d.root, e, 5) }, 0);
    type_utf8(&mut d, "a\u{e9}\u{20ac}\u{1f600}z!!!".as_bytes());
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(chars_at(unsafe { (*e).text.as_ptr().cast() }), 5);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        bytes_at(unsafe { (*e).text.as_ptr().cast() }),
        "a\u{e9}\u{20ac}\u{1f600}z".as_bytes()
    );
    assert_eq!(changed, 5);
    assert!(type_cp(&mut d, u32::from(b'x')));
    assert_eq!(changed, 5);
    // SAFETY: le privé d'un edit est un t_edit.
    unsafe {
        let ed = (*e).r#priv.cast::<t_edit>();
        (*ed).anchor = 0;
        (*ed).caret = 1;
    }
    type_utf8(&mut d, b"Q");
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(chars_at(unsafe { (*e).text.as_ptr().cast() }), 5);
    shut(&mut d);

    changed = 0;
    let mut d = desk(200, 40, &mut changed);
    let e = add(&mut d, &sp);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_focus(&mut d.root, e) };
    for _ in 0..70 {
        type_utf8(&mut d, "😀".as_bytes());
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(slen(unsafe { (*e).text.as_ptr().cast() }), 252);
    type_utf8(&mut d, b"abcdef");
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(slen(unsafe { (*e).text.as_ptr().cast() }), 255);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(chars_at(unsafe { (*e).text.as_ptr().cast() }), 63 + 3);
    shut(&mut d);

    let six = b"\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\0";
    let sp6 = spec(CT_EDIT, 1, rect_make(5, 5, 150, 20), six.as_ptr().cast());
    let mut d = desk(200, 40, &mut changed);
    let e = add(&mut d, &sp6);
    let btn = spec(CT_BUTTON, 2, rect_make(0, 30, 10, 10), c"b".as_ptr());
    let b = add(&mut d, &btn);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_edit_set_limit(&mut d.root, e, 0) }, E_INVAL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_edit_set_limit(&mut d.root, b, 3) }, E_INVAL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_edit_set_limit(&mut d.root, e, 3) }, 0);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        bytes_at(unsafe { (*e).text.as_ptr().cast() }),
        "ééé".as_bytes()
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { (*(*e).r#priv.cast::<t_edit>()).caret } <= 6);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_edit_set_limit(&mut d.root, e, 1_000_000) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { (*(*e).r#priv.cast::<t_edit>()).max_chars }, 255);
    let mut big = [b'k'; 400];
    big[399] = 0;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_set_text(&mut d.root, e, big.as_ptr().cast()) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(slen(unsafe { (*e).text.as_ptr().cast() }), 255);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { (*(*e).r#priv.cast::<t_edit>()).caret }, 255);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_edit_set_limit(&mut d.root, e, 4) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe {
        ctl_set_text(
            &mut d.root,
            e,
            c"\xc3\xa9\xe2\x82\xac\xc3\xa9\xe2\x82\xac\xc3\xa9".as_ptr(),
        )
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(chars_at(unsafe { (*e).text.as_ptr().cast() }), 4);
    changed = 0;
    d.root.user = (&mut changed as *mut u32).cast();
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_set_text(&mut d.root, e, c"\xff\xfe".as_ptr()) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_focus(&mut d.root, e) };
    press(&mut d, VK_BACK, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(slen(unsafe { (*e).text.as_ptr().cast() }), 1);
    assert_eq!(changed, 1);
    shut(&mut d);
}

#[test]
fn arguments_nuls_et_mauvais_type() {
    let sp = spec(CT_BUTTON, 1, rect_make(0, 0, 5, 5), c"".as_ptr());
    let mouse = t_wmmouse {
        h: crate::abi::t_wmhdr {
            magic: 0,
            r#type: 0,
            size: 0,
            window: 0,
            seq: 0,
            status: 0,
        },
        r#type: 0,
        x: 0,
        y: 0,
        buttons: 0,
        wheel: 0,
        hit: 0,
    };
    let ev = t_inpevent {
        r#type: 0,
        code: 0,
        x: 0,
        y: 0,
        mods: 0,
        scancode: 0,
        time_ns: 0,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_add(core::ptr::null_mut(), core::ptr::null_mut(), &sp) }.is_null());
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_find(core::ptr::null_mut(), 1) }.is_null());
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_mouse(core::ptr::null_mut(), &mouse) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_key(core::ptr::null_mut(), &ev) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_paint(core::ptr::null_mut()).w }, 0);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_set_rect(core::ptr::null_mut(), core::ptr::null_mut(), sp.rect) },
        E_INVAL
    );
    let mut changed = 0u32;
    let mut d = desk(50, 50, &mut changed);
    let list = spec(CT_LIST, 1, rect_make(0, 0, 30, 30), c"".as_ptr());
    let c = add(&mut d, &list);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_add(&mut d.root, core::ptr::null_mut(), core::ptr::null()) }.is_null());
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_mouse(&mut d.root, core::ptr::null()) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_key(&mut d.root, core::ptr::null()) });
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_list_add(&mut d.root, c, core::ptr::null()) },
        E_INVAL
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_list_add(&mut d.root, core::ptr::null_mut(), c"x".as_ptr()) },
        E_INVAL
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_count(core::ptr::null()) }, E_INVAL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_selected(core::ptr::null()) }, -1);
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_list_text(core::ptr::null(), 0, core::ptr::null_mut(), 0) },
        E_INVAL
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_scroll_pos(core::ptr::null()) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_menu_result(core::ptr::null()) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_menu_popup(&mut d.root, core::ptr::null()) }.is_null());
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_menu_close(&mut d.root, core::ptr::null_mut()) };
    shut(&mut d);
    let mut d = desk(50, 50, &mut changed);
    let b = add(
        &mut d,
        &spec(CT_BUTTON, 1, rect_make(0, 0, 30, 30), c"b".as_ptr()),
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_list_add(&mut d.root, b, c"x".as_ptr()) },
        E_INVAL
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_edit_set_limit(&mut d.root, b, 3) }, E_INVAL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_count(b) }, E_INVAL);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_progress_set(&mut d.root, b, 50) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_scroll_set(&mut d.root, b, 10, 2) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_scroll_move(&mut d.root, b, 3) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_scroll_pos(b) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_menu_close(&mut d.root, b) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_find(&mut d.root, 1) }, b);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_select(&mut d.root, b, 0) }, E_INVAL);
    shut(&mut d);
}

#[test]
fn molette_rect_couleurs_et_barre() {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_wheel_delta(1025) }, 3072);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_wheel_delta(-1025) }, -3072);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_wheel_delta(0) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_wheel_delta(1) }, 3);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_wheel_delta(-1) }, -3);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_rect_ok(rect_make(CTL_COORD_MAX, -CTL_COORD_MAX, 0, CTL_COORD_MAX)) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_rect_ok(rect_make(CTL_COORD_MAX + 1, 0, 1, 1)) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_rect_ok(rect_make(0, 0, -1, 1)) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_color_shadow() }, 0xffff_ffff);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_color_selected_text() }, 0xffff_ffff);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_color_back(true) }, 0xffff_ffff);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_color_text(true) }, 0xff00_0000);
    let bar = rect_make(0, 0, 17, 100);
    let mut st = t_scrst {
        pos: 0,
        max: 90,
        page: 10,
        grab: 0,
        zone: 0,
    };
    let mut lo = t_sblo {
        dec: rect_make(0, 0, 0, 0),
        inc: rect_make(0, 0, 0, 0),
        track: rect_make(0, 0, 0, 0),
        thumb: rect_make(0, 0, 0, 0),
        vertical: false,
    };
    // SAFETY: st et lo sont locaux.
    unsafe { ctl_sb_layout(bar, &st, &mut lo) };
    assert_eq!(lo.thumb.y, lo.track.y);
    assert_eq!(lo.thumb.h, CTL_THUMB_MIN);
    st.pos = 90;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_layout(bar, &st, &mut lo) };
    assert_eq!(lo.thumb.y + lo.thumb.h, lo.track.y + lo.track.h);
    st = t_scrst {
        pos: 0,
        max: 0,
        page: 10,
        grab: 0,
        zone: 0,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_layout(bar, &st, &mut lo) };
    assert!(rect_empty(lo.thumb));
    st = t_scrst {
        pos: 0,
        max: 1,
        page: 100,
        grab: 0,
        zone: 0,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_layout(bar, &st, &mut lo) };
    assert!(lo.thumb.h > lo.track.h * 9 / 10);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_sb_vertical(rect_make(0, 0, 17, 17)) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_sb_vertical(rect_make(0, 0, 100, 17)) });
    st = t_scrst {
        pos: 0,
        max: 90,
        page: 10,
        grab: 0,
        zone: 0,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_layout(bar, &st, &mut lo) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_sb_pos_from(&lo, &st, 17) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_sb_pos_from(&lo, &st, 75) }, 90);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_sb_pos_from(&lo, &st, 46) }, 45);
    let zbar = rect_make(5, 7, 17, 100);
    st.pos = 40;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_layout(zbar, &st, &mut lo) };
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_zone(&lo, t_point { x: 5, y: 7 }) },
        crate::abi::CTL_SBZ_DEC
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_zone(&lo, t_point { x: 5, y: 90 }) },
        crate::abi::CTL_SBZ_INC
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_zone(&lo, t_point { x: 5, y: 49 }) },
        crate::abi::CTL_SBZ_THUMB
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_zone(&lo, t_point { x: 5, y: 24 }) },
        crate::abi::CTL_SBZ_PAGE_DEC
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_zone(&lo, t_point { x: 5, y: 57 }) },
        crate::abi::CTL_SBZ_PAGE_INC
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_zone(&lo, t_point { x: 0, y: 0 }) },
        crate::abi::CTL_SBZ_NONE
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_sb_set(&mut st, 5) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(!unsafe { ctl_sb_set(&mut st, 5) });
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_sb_set(&mut st, -9) } && st.pos == 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_sb_set(&mut st, 999) } && st.pos == 90);
    let mut mt = t_lunametrics {
        caption_h: 0,
        frame_w: 0,
        frame_bottom: 0,
        btn_w: 0,
        btn_h: 0,
        taskbar_h: 0,
        start_w: 0,
        tray_w: 0,
        scroll_w: 0,
        menu_item_h: 0,
        icon_small: 0,
        icon_large: 0,
    };
    luna_metrics(&mut mt);
    assert_eq!(mt.menu_item_h, 22);
    let sep = t_ctlmenuitem {
        text: core::ptr::null(),
        id: 0,
        flags: CTL_MI_SEPARATOR,
        sub: core::ptr::null(),
        nsub: 0,
    };
    let row = t_ctlmenuitem { flags: 0, ..sep };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_mnu_item_h(&sep, &mt) }, 6);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_mnu_item_h(&row, &mt) }, 22);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_mnu_pool(core::ptr::null(), 0, 0) }, 0);
    let _ = rect_contains(bar, t_point { x: 0, y: 0 });
}

#[test]
fn liste_erreurs_et_menu_vide() {
    let mut changed = 0u32;
    let mut d = desk(80, 80, &mut changed);
    let list = add(
        &mut d,
        &spec(CT_LIST, 3, rect_make(0, 0, 60, 40), c"".as_ptr()),
    );
    assert_eq!(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_list_add(&mut d.root, list, c"un".as_ptr()) },
        0
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_count(list) }, 1);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_select(&mut d.root, list, -1) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_select(&mut d.root, list, -2) }, E_RANGE);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_list_select(&mut d.root, list, 4) }, E_RANGE);
    let item = t_ctlmenuitem {
        text: c"A".as_ptr(),
        id: 3,
        flags: 0,
        sub: core::ptr::null(),
        nsub: 0,
    };
    let ms = t_ctlmenuspec {
        id: 9,
        at: t_point { x: 4, y: 4 },
        items: &item,
        count: 1,
    };
    // SAFETY: items vit le temps du popup.
    let menu = unsafe { ctl_menu_popup(&mut d.root, &ms) };
    assert!(!menu.is_null());
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert_eq!(unsafe { ctl_menu_result(menu) }, 0);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_menu_close(&mut d.root, menu) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    assert!(unsafe { ctl_find(&mut d.root, 9) }.is_null());
    shut(&mut d);
}
