use core::ffi::{c_char, c_int};
use core::mem::size_of;
use core::sync::atomic::Ordering;
use std::sync::{Mutex, MutexGuard};

use crate::sys::{
    CLOSE_H, CLOSE_N, CONNECT_RET, LUNA_DARK, LUNA_N, LUNA_RED, NAME_WM, SEND_RET, SENT_LEN,
    SENT_MAGIC, SENT_N, SENT_SEQ, SENT_SIZE, SENT_TYPE, SENT_VALUE, SENT_WIN,
};
use crate::{
    E_BADF, E_BUSY, E_INVAL, E_IO, E_NOSPC, E_OK, E_PIPE, E_PROTO, ICON_IDS, WM_MAGIC,
    WM_TITLE_MAX, WM_WIN_PER_CLIENT, WMC_ACTIVATE, WMC_SET_STATE, WMS_CREATED, WMS_KEY, WMS_THEME,
    WSTATE_HIDDEN, g_wmc, t_rect, t_wmcmsg, t_wmcreate, t_wmhdr, t_wmtheme, t_wmwin, wmc_activate,
    wmc_attach, wmc_capture, wmc_channel, wmc_connect, wmc_create, wmc_destroy, wmc_hdr, wmc_next,
    wmc_on_message, wmc_present, wmc_send, wmc_set_cursor, wmc_set_icon, wmc_set_rect,
    wmc_set_state, wmc_set_state_id, wmc_set_theme, wmc_set_title, wmc_set_workarea, wmc_slot,
    wmc_subscribe, wmc_theme_apply, wmc_title_copy, wmc_valid,
};

const E_NODEV: c_int = -19;
const E_NOSYS: c_int = -38;

fn lock() -> MutexGuard<'static, ()> {
    static LOCK: Mutex<()> = Mutex::new(());
    match LOCK.lock() {
        Ok(g) => g,
        Err(e) => e.into_inner(),
    }
}

fn reset_spy() {
    SEND_RET.store(E_NOSYS, Ordering::Relaxed);
    CONNECT_RET.store(E_IO, Ordering::Relaxed);
    SENT_N.store(0, Ordering::Relaxed);
    SENT_LEN.store(0, Ordering::Relaxed);
    SENT_MAGIC.store(0, Ordering::Relaxed);
    SENT_TYPE.store(0, Ordering::Relaxed);
    SENT_SIZE.store(0, Ordering::Relaxed);
    SENT_WIN.store(0, Ordering::Relaxed);
    SENT_SEQ.store(0, Ordering::Relaxed);
    SENT_VALUE.store(0, Ordering::Relaxed);
    NAME_WM.store(0, Ordering::Relaxed);
    LUNA_N.store(0, Ordering::Relaxed);
    LUNA_DARK.store(0, Ordering::Relaxed);
    LUNA_RED.store(0, Ordering::Relaxed);
    CLOSE_N.store(0, Ordering::Relaxed);
    CLOSE_H.store(0, Ordering::Relaxed);
}

struct Clean;

impl Drop for Clean {
    fn drop(&mut self) {
        unsafe {
            // SAFETY: le verrou de test rend g_wmc exclusif. On le remet à zéro.
            core::ptr::write(g_wmc.get(), crate::t_wmcstate::ZERO);
        }
        reset_spy();
    }
}

fn guard() -> (MutexGuard<'static, ()>, Clean) {
    let g = lock();
    unsafe {
        // SAFETY: le verrou est pris. g_wmc est remis à l'état C initial.
        core::ptr::write(g_wmc.get(), crate::t_wmcstate::ZERO);
    }
    reset_spy();
    (g, Clean)
}

fn win(id: u32) -> t_wmwin {
    let mut w = t_wmwin::ZERO;
    w.id = id;
    w
}

fn put(buf: &mut [u64; crate::WM_BUF_WORDS], src: &[u8]) {
    unsafe {
        // SAFETY: src tient dans les 320 octets du tampon, les deux sont alignés sur l'octet.
        core::ptr::copy_nonoverlapping(src.as_ptr(), buf.as_mut_ptr().cast::<u8>(), src.len());
    }
}

fn theme_msg(dark: u32, reduced: u32, reserved: u32) -> t_wmcmsg {
    let mut t = t_wmtheme::ZERO;
    t.h.magic = WM_MAGIC;
    t.h.r#type = WMS_THEME;
    t.h.size = size_of::<t_wmtheme>() as u32;
    t.dark = dark;
    t.reduced = reduced;
    t.reserved = reserved;
    let mut m = t_wmcmsg::ZERO;
    put(&mut m.buf, unsafe {
        // SAFETY: t est un t_wmtheme local, sa représentation tient dans le tampon.
        core::slice::from_raw_parts(core::ptr::from_ref(&t).cast::<u8>(), size_of::<t_wmtheme>())
    });
    m.len = size_of::<t_wmtheme>() as u32;
    m
}

#[test]
fn pointeurs_nuls_et_bornes() {
    let _g = guard();
    let w = win(4);
    let bare = t_wmwin::ZERO;
    let rq = t_wmcreate::ZERO;
    let rect = t_rect::ZERO;
    let title = b"titre\0";
    unsafe {
        // SAFETY: objets alignés sur la pile, ou pointeurs nuls. Canal fermé.
        assert_eq!(wmc_set_theme(core::ptr::null(), 0, 0), E_INVAL);
        assert_eq!(wmc_set_theme(&bare, 1, 0), E_INVAL);
        assert_eq!(wmc_set_theme(&w, 2, 0), E_INVAL);
        assert_eq!(wmc_set_theme(&w, 0, 2), E_INVAL);
        assert_eq!(wmc_set_theme(&w, u32::MAX, 1), E_INVAL);
        assert_eq!(wmc_set_theme(&w, 1, 1), E_BADF);
        assert_eq!(wmc_set_state(core::ptr::null_mut(), 0), E_INVAL);
        assert_eq!(wmc_set_state(&mut win(1), WSTATE_HIDDEN + 1), E_INVAL);
        assert_eq!(wmc_set_state(&mut win(1), WSTATE_HIDDEN), E_BADF);
        assert_eq!(wmc_set_state_id(0, 0), E_INVAL);
        assert_eq!(wmc_set_state_id(1, WSTATE_HIDDEN + 1), E_INVAL);
        assert_eq!(wmc_set_state_id(1, 0), E_BADF);
        assert_eq!(wmc_set_cursor(core::ptr::null_mut(), 0), E_INVAL);
        assert_eq!(wmc_set_cursor(&mut win(1), 8), E_INVAL);
        assert_eq!(wmc_set_cursor(&mut win(1), 7), E_BADF);
        assert_eq!(wmc_set_icon(core::ptr::null_mut(), 0), E_INVAL);
        assert_eq!(wmc_set_icon(&mut win(1), ICON_IDS), E_INVAL);
        assert_eq!(wmc_set_icon(&mut win(1), ICON_IDS - 1), E_BADF);
        assert_eq!(wmc_capture(core::ptr::null_mut(), 1), E_INVAL);
        assert_eq!(wmc_capture(&mut win(1), 2), E_BADF);
        assert_eq!(wmc_activate(0), E_INVAL);
        assert_eq!(wmc_activate(4), E_BADF);
        assert_eq!(wmc_create(core::ptr::null_mut(), &rq), E_INVAL);
        assert_eq!(wmc_create(&mut win(0), core::ptr::null()), E_INVAL);
        assert_eq!(wmc_destroy(core::ptr::null_mut()), E_INVAL);
        assert_eq!(wmc_destroy(&mut bare.clone()), E_INVAL);
        assert_eq!(
            wmc_present(core::ptr::null_mut(), core::ptr::null(), 0),
            E_INVAL
        );
        assert_eq!(wmc_present(&mut win(1), core::ptr::null(), 1), E_INVAL);
        assert_eq!(wmc_next(core::ptr::null_mut(), 0, 0), E_INVAL);
        assert_eq!(
            wmc_set_title(core::ptr::null_mut(), title.as_ptr().cast()),
            E_INVAL
        );
        assert_eq!(wmc_set_title(&mut win(1), core::ptr::null()), E_INVAL);
        assert_eq!(wmc_set_rect(core::ptr::null_mut(), rect), E_INVAL);
        assert_eq!(wmc_set_workarea(rect), E_BADF);
        assert_eq!(wmc_subscribe(1), E_BADF);
        assert_eq!(wmc_channel(), 0);
        assert_eq!(wmc_send(core::ptr::null_mut()), E_INVAL);
        assert_eq!(SENT_N.load(Ordering::Relaxed), 0);
    }
}

#[test]
fn message_valide() {
    let _g = guard();
    let mut good = theme_msg(1, 1, 0);
    unsafe {
        // SAFETY: good est aligné et local.
        assert!(wmc_valid(&good));
        assert!(!wmc_valid(core::ptr::null()));
    }
    good.len -= 4;
    unsafe {
        // SAFETY: good reste aligné.
        assert!(!wmc_valid(&good));
    }
    good = theme_msg(1, 0, 0);
    good.handle = 4;
    unsafe {
        // SAFETY: un thème ne porte pas de descripteur.
        assert!(!wmc_valid(&good));
    }
    let mut created = t_wmcmsg::ZERO;
    let mut hdr = t_wmhdr::ZERO;
    hdr.magic = WM_MAGIC;
    hdr.r#type = WMS_CREATED;
    hdr.size = size_of::<crate::t_wmcreated>() as u32;
    put(&mut created.buf, unsafe {
        // SAFETY: hdr tient dans le tampon. Le reste du message créé est à zéro.
        core::slice::from_raw_parts(core::ptr::from_ref(&hdr).cast::<u8>(), size_of::<t_wmhdr>())
    });
    created.len = hdr.size;
    unsafe {
        // SAFETY: created est aligné. Sans descripteur, WMS_CREATED est refusé.
        assert!(!wmc_valid(&created));
    }
    created.handle = 1;
    unsafe {
        // SAFETY: le descripteur non nul rend WMS_CREATED valide.
        assert!(wmc_valid(&created));
    }
}

#[test]
fn en_tete_et_titre() {
    let _g = guard();
    let mut h = t_wmhdr::ZERO;
    h.seq = 9;
    h.status = 4;
    unsafe {
        // SAFETY: h est aligné et exclusif.
        wmc_hdr(&mut h, WMS_KEY, 24, 7);
    }
    assert_eq!(h.magic, WM_MAGIC);
    assert_eq!(h.r#type, WMS_KEY);
    assert_eq!(h.size, 24);
    assert_eq!(h.window, 7);
    assert_eq!(h.seq, 0);
    assert_eq!(h.status, 0);

    let mut dst = [1u8; WM_TITLE_MAX];
    unsafe {
        // SAFETY: dst a 64 octets. src est nul.
        wmc_title_copy(dst.as_mut_ptr().cast(), core::ptr::null());
    }
    assert!(dst.iter().all(|b| *b == 0));

    let src = b"ok\0";
    unsafe {
        // SAFETY: src est une chaîne C, dst a 64 octets.
        wmc_title_copy(dst.as_mut_ptr().cast(), src.as_ptr().cast());
    }
    assert_eq!(&dst[..3], b"ok\0");

    let mut long = [0u8; WM_TITLE_MAX];
    long[..61].fill(b'a');
    long[61] = 0xe2;
    long[62] = 0x82;
    long[63] = 0xac;
    unsafe {
        // SAFETY: long a 64 octets sans NUL avant l'indice 63, dst a 64 octets.
        wmc_title_copy(dst.as_mut_ptr().cast(), long.as_ptr().cast());
    }
    assert_eq!(&dst[..61], &[b'a'; 61]);
    assert!(dst[61..].iter().all(|b| *b == 0));
}

#[test]
fn theme_hors_bornes_ignore() {
    let _g = guard();
    let bad = theme_msg(5, 0, 0);
    let reserved = theme_msg(1, 0, 1);
    let short = {
        let mut m = theme_msg(1, 1, 0);
        m.len -= 4;
        m
    };
    let dark = theme_msg(1, 1, 0);
    let light = theme_msg(0, 0, 0);
    unsafe {
        // SAFETY: messages alignés. Le garde précède wm_luna_apply.
        wmc_theme_apply(&bad);
        wmc_on_message(&mut reserved.clone());
        wmc_theme_apply(&short);
        assert_eq!(LUNA_N.load(Ordering::Relaxed), 0);
        wmc_on_message(&mut dark.clone());
        assert_eq!(LUNA_N.load(Ordering::Relaxed), 1);
        assert_eq!(LUNA_DARK.load(Ordering::Relaxed), 1);
        assert_eq!(LUNA_RED.load(Ordering::Relaxed), 1);
        wmc_theme_apply(&light);
        assert_eq!(LUNA_N.load(Ordering::Relaxed), 2);
        assert_eq!(LUNA_DARK.load(Ordering::Relaxed), 0);
        assert_eq!(LUNA_RED.load(Ordering::Relaxed), 0);
    }
}

#[test]
fn envoi_taille_et_perte() {
    let _g = guard();
    unsafe {
        // SAFETY: le verrou rend g_wmc exclusif. Le canal factice ne quitte pas le test.
        (*g_wmc.get()).chan = 7;
        SEND_RET.store(E_OK, Ordering::Relaxed);
        assert_eq!(wmc_activate(3), E_OK);
        assert_eq!(SENT_N.load(Ordering::Relaxed), 1);
        assert_eq!(
            SENT_LEN.load(Ordering::Relaxed),
            size_of::<t_wmhdr>() as u32
        );
        assert_eq!(
            SENT_SIZE.load(Ordering::Relaxed),
            size_of::<t_wmhdr>() as u32
        );
        assert_eq!(SENT_TYPE.load(Ordering::Relaxed), WMC_ACTIVATE);
        assert_eq!(SENT_WIN.load(Ordering::Relaxed), 3);
        assert_eq!(SENT_SEQ.load(Ordering::Relaxed), 1);
        assert_eq!(SENT_MAGIC.load(Ordering::Relaxed), WM_MAGIC);
        assert_eq!(wmc_set_state_id(9, 0), E_OK);
        assert_eq!(
            SENT_LEN.load(Ordering::Relaxed),
            size_of::<crate::t_wmarg>() as u32
        );
        assert_eq!(SENT_TYPE.load(Ordering::Relaxed), WMC_SET_STATE);
        assert_eq!(SENT_VALUE.load(Ordering::Relaxed), 0);
        assert_eq!(SENT_SEQ.load(Ordering::Relaxed), 2);
        (*g_wmc.get()).seq = 0x7fff_ffff;
        assert_eq!(wmc_activate(1), E_OK);
        assert_eq!((*g_wmc.get()).seq, 1);
        assert_eq!(SENT_SEQ.load(Ordering::Relaxed), 1);
        SEND_RET.store(E_PIPE, Ordering::Relaxed);
        let before = SENT_N.load(Ordering::Relaxed);
        assert_eq!(wmc_activate(1), E_PIPE);
        assert!((*g_wmc.get()).lost);
        assert_eq!(wmc_activate(1), E_PIPE);
        assert_eq!(SENT_N.load(Ordering::Relaxed), before + 1);
    }
}

#[test]
fn connexion_refusee() {
    let _g = guard();
    unsafe {
        // SAFETY: info nul. Le connecteur factice renvoie le code demandé.
        CONNECT_RET.store(E_NODEV, Ordering::Relaxed);
        assert_eq!(wmc_connect(core::ptr::null_mut()), E_NODEV);
        assert_eq!(NAME_WM.load(Ordering::Relaxed), 1);
        assert_eq!(wmc_channel(), 0);
        CONNECT_RET.store(0, Ordering::Relaxed);
        assert_eq!(wmc_connect(core::ptr::null_mut()), E_IO);
        assert_eq!(wmc_channel(), 0);
        (*g_wmc.get()).chan = 3;
        assert_eq!(wmc_connect(core::ptr::null_mut()), E_BUSY);
    }
}

#[test]
fn descripteur_ferme_si_inutile() {
    let _g = guard();
    let mut m = t_wmcmsg::ZERO;
    m.handle = 42;
    let mut hdr = t_wmhdr::ZERO;
    hdr.r#type = WMS_KEY;
    put(&mut m.buf, unsafe {
        // SAFETY: hdr tient dans le tampon.
        core::slice::from_raw_parts(core::ptr::from_ref(&hdr).cast::<u8>(), size_of::<t_wmhdr>())
    });
    unsafe {
        // SAFETY: m est aligné. Aucune fenêtre ne porte ce descripteur.
        wmc_on_message(&mut m);
    }
    assert_eq!(m.handle, 0);
    assert_eq!(CLOSE_N.load(Ordering::Relaxed), 1);
    assert_eq!(CLOSE_H.load(Ordering::Relaxed), 42);

    let mut w = t_wmwin::ZERO;
    let mut bad = t_wmcmsg::ZERO;
    bad.handle = 9;
    unsafe {
        // SAFETY: largeur nulle, le descripteur doit être fermé.
        assert_eq!(wmc_attach(&mut w, &bad), E_PROTO);
    }
    assert_eq!(CLOSE_H.load(Ordering::Relaxed), 9);
    assert_eq!(w.section, 0);
}

#[test]
fn present_et_places() {
    let _g = guard();
    let mut w = win(3);
    w.surface.w = 100;
    w.surface.h = 80;
    let outside = [t_rect {
        x: 200,
        y: 200,
        w: 4,
        h: 4,
    }];
    unsafe {
        // SAFETY: w et outside sont alignés. Le rectangle est hors surface.
        assert_eq!(wmc_present(&mut w, outside.as_ptr(), 1), E_OK);
        assert_eq!(SENT_N.load(Ordering::Relaxed), 0);
        let mut buf = [0u8; 8];
        assert_eq!(wmc_next(buf.as_mut_ptr().cast(), 8, 0), E_BADF);
        assert_eq!(wmc_slot(0), 0);
        assert_eq!(wmc_slot(3), -1);
    }
    let mut many = [t_rect {
        x: 0,
        y: 0,
        w: 1,
        h: 1,
    }; 17];
    let _ = &mut many;
    unsafe {
        // SAFETY: canal factice, 17 rectangles dans la surface, deux envois.
        (*g_wmc.get()).chan = 1;
        SEND_RET.store(E_OK, Ordering::Relaxed);
        assert_eq!(wmc_present(&mut w, many.as_ptr(), 17), E_OK);
        assert_eq!(SENT_N.load(Ordering::Relaxed), 2);
        let p = core::ptr::from_mut(&mut w);
        let st = g_wmc.get();
        let mut i = 0;
        while i < WM_WIN_PER_CLIENT {
            (*st).wins[i] = p;
            i += 1;
        }
        let rq = t_wmcreate::ZERO;
        assert_eq!(wmc_create(&mut w, &rq), E_NOSPC);
    }
}

#[test]
fn titre_copie_dans_creation() {
    let _g = guard();
    let mut w = t_wmwin::ZERO;
    let mut rq = t_wmcreate::ZERO;
    rq.title[..2].copy_from_slice(b"ab");
    unsafe {
        // SAFETY: canal fermé, la copie du titre se fait avant le refus d'envoi.
        assert_eq!(wmc_create(&mut w, &rq), E_BADF);
        assert_eq!(wmc_channel(), 0);
        let mut dst = [9u8; WM_TITLE_MAX];
        let src: &[u8] = &rq.title;
        let mut tmp = [0u8; WM_TITLE_MAX + 1];
        tmp[..WM_TITLE_MAX].copy_from_slice(src);
        wmc_title_copy(dst.as_mut_ptr().cast(), tmp.as_ptr().cast::<c_char>());
        assert_eq!(dst[0], b'a');
        assert_eq!(dst[1], b'b');
        assert_eq!(dst[2], 0);
    }
}
