use core::ffi::{c_char, c_int};
use core::mem::{align_of, offset_of, size_of};

use crate::client::harness;
use crate::{
    E_INVAL, E_IO, E_NOENT, E_TIMEOUT, NOTIF_APP_MAX, NOTIF_BADGE_MAX, NOTIF_CAP_BADGE,
    NOTIF_MAGIC, NOTIF_MAX, NOTIF_TEXT_MAX, NOTIF_TITLE_MAX, NOTIFC_CLEAR, NOTIFC_COUNT,
    NOTIFC_DISMISS, NOTIFC_POST, NOTIFC_TOGGLE, NOTIFS_OK, notif_age_text, notif_badge_text,
    notif_client_clear, notif_client_close, notif_client_count, notif_client_dismiss,
    notif_client_toggle, notif_fields_valid, notif_msg_decode, notif_msg_init, notif_msg_post,
    notif_post, notif_store_clear, notif_store_count, notif_store_dismiss, notif_store_get,
    notif_store_init, notif_store_post, t_notif, t_notifmsg, t_notifstore,
};

const KEYS: &[&str] = &[
    "sizeof t_notif",
    "alignof t_notif",
    "offsetof t_notif.id",
    "offsetof t_notif.icon",
    "offsetof t_notif.stamp_ns",
    "offsetof t_notif.app",
    "offsetof t_notif.title",
    "offsetof t_notif.text",
    "sizeof t_notifstore",
    "alignof t_notifstore",
    "offsetof t_notifstore.items",
    "offsetof t_notifstore.count",
    "offsetof t_notifstore.next_id",
    "offsetof t_notifstore.dropped",
    "sizeof t_notifmsg",
    "alignof t_notifmsg",
    "offsetof t_notifmsg.magic",
    "offsetof t_notifmsg.type",
    "offsetof t_notifmsg.size",
    "offsetof t_notifmsg.seq",
    "offsetof t_notifmsg.status",
    "offsetof t_notifmsg.id",
    "offsetof t_notifmsg.count",
    "offsetof t_notifmsg.icon",
    "offsetof t_notifmsg.stamp_ns",
    "offsetof t_notifmsg.app",
    "offsetof t_notifmsg.title",
    "offsetof t_notifmsg.text",
    "sizeof t_chansend",
    "alignof t_chansend",
    "offsetof t_chansend.msg",
    "offsetof t_chansend.len",
    "offsetof t_chansend.handles",
    "offsetof t_chansend.nhandles",
    "offsetof t_chansend.reserved",
    "sizeof t_chanrecv",
    "alignof t_chanrecv",
    "offsetof t_chanrecv.buf",
    "offsetof t_chanrecv.buf_len",
    "offsetof t_chanrecv.handles",
    "offsetof t_chanrecv.handles_max",
    "offsetof t_chanrecv.reserved",
    "offsetof t_chanrecv.timeout_ns",
    "offsetof t_chanrecv.out_len",
    "offsetof t_chanrecv.out_nhandles",
    "offsetof t_chanrecv.reserved2",
];

fn layout(key: &str) -> Option<u64> {
    let v = match key {
        "sizeof t_notif" => size_of::<t_notif>(),
        "alignof t_notif" => align_of::<t_notif>(),
        "offsetof t_notif.id" => offset_of!(t_notif, id),
        "offsetof t_notif.icon" => offset_of!(t_notif, icon),
        "offsetof t_notif.stamp_ns" => offset_of!(t_notif, stamp_ns),
        "offsetof t_notif.app" => offset_of!(t_notif, app),
        "offsetof t_notif.title" => offset_of!(t_notif, title),
        "offsetof t_notif.text" => offset_of!(t_notif, text),
        "sizeof t_notifstore" => size_of::<t_notifstore>(),
        "alignof t_notifstore" => align_of::<t_notifstore>(),
        "offsetof t_notifstore.items" => offset_of!(t_notifstore, items),
        "offsetof t_notifstore.count" => offset_of!(t_notifstore, count),
        "offsetof t_notifstore.next_id" => offset_of!(t_notifstore, next_id),
        "offsetof t_notifstore.dropped" => offset_of!(t_notifstore, dropped),
        "sizeof t_notifmsg" => size_of::<t_notifmsg>(),
        "alignof t_notifmsg" => align_of::<t_notifmsg>(),
        "offsetof t_notifmsg.magic" => offset_of!(t_notifmsg, magic),
        "offsetof t_notifmsg.type" => offset_of!(t_notifmsg, r#type),
        "offsetof t_notifmsg.size" => offset_of!(t_notifmsg, size),
        "offsetof t_notifmsg.seq" => offset_of!(t_notifmsg, seq),
        "offsetof t_notifmsg.status" => offset_of!(t_notifmsg, status),
        "offsetof t_notifmsg.id" => offset_of!(t_notifmsg, id),
        "offsetof t_notifmsg.count" => offset_of!(t_notifmsg, count),
        "offsetof t_notifmsg.icon" => offset_of!(t_notifmsg, icon),
        "offsetof t_notifmsg.stamp_ns" => offset_of!(t_notifmsg, stamp_ns),
        "offsetof t_notifmsg.app" => offset_of!(t_notifmsg, app),
        "offsetof t_notifmsg.title" => offset_of!(t_notifmsg, title),
        "offsetof t_notifmsg.text" => offset_of!(t_notifmsg, text),
        "sizeof t_chansend" => size_of::<crate::t_chansend>(),
        "alignof t_chansend" => align_of::<crate::t_chansend>(),
        "offsetof t_chansend.msg" => offset_of!(crate::t_chansend, msg),
        "offsetof t_chansend.len" => offset_of!(crate::t_chansend, len),
        "offsetof t_chansend.handles" => offset_of!(crate::t_chansend, handles),
        "offsetof t_chansend.nhandles" => offset_of!(crate::t_chansend, nhandles),
        "offsetof t_chansend.reserved" => offset_of!(crate::t_chansend, reserved),
        "sizeof t_chanrecv" => size_of::<crate::t_chanrecv>(),
        "alignof t_chanrecv" => align_of::<crate::t_chanrecv>(),
        "offsetof t_chanrecv.buf" => offset_of!(crate::t_chanrecv, buf),
        "offsetof t_chanrecv.buf_len" => offset_of!(crate::t_chanrecv, buf_len),
        "offsetof t_chanrecv.handles" => offset_of!(crate::t_chanrecv, handles),
        "offsetof t_chanrecv.handles_max" => offset_of!(crate::t_chanrecv, handles_max),
        "offsetof t_chanrecv.reserved" => offset_of!(crate::t_chanrecv, reserved),
        "offsetof t_chanrecv.timeout_ns" => offset_of!(crate::t_chanrecv, timeout_ns),
        "offsetof t_chanrecv.out_len" => offset_of!(crate::t_chanrecv, out_len),
        "offsetof t_chanrecv.out_nhandles" => offset_of!(crate::t_chanrecv, out_nhandles),
        "offsetof t_chanrecv.reserved2" => offset_of!(crate::t_chanrecv, reserved2),
        _ => return None,
    };
    Some(v as u64)
}

fn parse(text: &str) -> Vec<(String, u64)> {
    let mut rows = Vec::new();
    for (n, line) in text.lines().enumerate() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        let Some((key, val)) = line.rsplit_once(' ') else {
            panic!("ligne {n} sans valeur");
        };
        let Ok(val) = val.parse::<u64>() else {
            panic!("valeur illisible {line}");
        };
        rows.push((key.to_string(), val));
    }
    rows
}

#[test]
fn tailles_abi() {
    assert_eq!(size_of::<usize>(), 8);
    let host = parse(include_str!("../../../build/notif-sizes/sizes-host.txt"));
    let elf = parse(include_str!("../../../build/notif-sizes/sizes-elf.txt"));
    assert_eq!(host, elf, "gcc hôte et x86_64-elf-gcc -ffreestanding");
    assert_eq!(host.len(), KEYS.len());
    for key in KEYS {
        let measured = host.iter().find(|(k, _)| k == key).map(|(_, v)| *v);
        let Some(measured) = measured else {
            panic!("mesure absente {key}");
        };
        let Some(got) = layout(key) else {
            panic!("clé rust inconnue {key}");
        };
        assert_eq!(got, measured, "{key}");
    }
    for (key, _) in &host {
        assert!(KEYS.contains(&key.as_str()), "mesure en trop {key}");
    }
}

fn cstr(s: &str) -> Vec<u8> {
    let mut v = s.as_bytes().to_vec();
    v.push(0);
    v
}

fn bytes_of(buf: &[u8]) -> &[u8] {
    match buf.iter().position(|b| *b == 0) {
        Some(n) => &buf[..n],
        None => buf,
    }
}

fn copy_field(dst: &mut [u8], src: &[u8]) {
    let n = src.len().min(dst.len().saturating_sub(1));
    dst[..n].copy_from_slice(&src[..n]);
    if n < dst.len() {
        dst[n] = 0;
    }
}

fn make(title: &str) -> t_notif {
    let mut n = t_notif::zero();
    copy_field(&mut n.app, b"Stockage");
    copy_field(&mut n.title, title.as_bytes());
    copy_field(&mut n.text, b"Le disque (C:) est plein.");
    n
}

fn init(s: &mut t_notifstore) {
    unsafe {
        // SAFETY: s est un objet de test aligné et exclusif.
        notif_store_init(s);
    }
}

fn post(s: &mut t_notifstore, n: &t_notif) -> c_int {
    unsafe {
        // SAFETY: s et n sont alignés, distincts, s exclusif.
        notif_store_post(s, n)
    }
}

fn dismiss(s: &mut t_notifstore, id: u32) -> c_int {
    unsafe {
        // SAFETY: s est aligné et exclusif.
        notif_store_dismiss(s, id)
    }
}

fn get(s: &t_notifstore, index: u32) -> Option<&t_notif> {
    let p = unsafe {
        // SAFETY: s est aligné et reste vivant le temps du pointeur.
        notif_store_get(s, index)
    };
    if p.is_null() {
        None
    } else {
        Some(unsafe {
            // SAFETY: pointeur non nul à l'intérieur de s.
            &*p
        })
    }
}

fn count(s: &t_notifstore) -> u32 {
    unsafe {
        // SAFETY: s est aligné.
        notif_store_count(s)
    }
}

#[test]
fn store_ordre_et_nul() {
    let mut s = t_notifstore::zero();
    init(&mut s);
    assert_eq!(s.next_id, 1);
    assert_eq!(s.count, 0);
    assert_eq!(s.dropped, 0);
    let a = make("A");
    let b = make("B");
    assert_eq!(post(&mut s, &a), 1);
    assert_eq!(post(&mut s, &b), 2);
    assert_eq!(count(&s), 2);
    assert_eq!(bytes_of(&get(&s, 0).unwrap().title), b"B");
    assert_eq!(bytes_of(&get(&s, 1).unwrap().title), b"A");
    assert!(get(&s, 2).is_none());
    unsafe {
        // SAFETY: pointeurs nuls, aucun déréférencement attendu.
        notif_store_init(core::ptr::null_mut());
        notif_store_clear(core::ptr::null_mut());
    }
    assert_eq!(
        unsafe {
            // SAFETY: magasin nul.
            notif_store_count(core::ptr::null())
        },
        0
    );
    assert!(
        unsafe {
            // SAFETY: magasin nul.
            notif_store_get(core::ptr::null(), 0)
        }
        .is_null()
    );
    assert_eq!(
        unsafe {
            // SAFETY: magasin nul, notification valide.
            notif_store_post(core::ptr::null_mut(), &a)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            // SAFETY: notification nulle.
            notif_store_post(&mut s, core::ptr::null())
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            // SAFETY: magasin nul.
            notif_store_dismiss(core::ptr::null_mut(), 1)
        },
        E_INVAL
    );
    assert_eq!(count(&s), 2);
}

#[test]
fn store_plafond_et_retrait() {
    let mut s = t_notifstore::zero();
    init(&mut s);
    let mut i = 1u32;
    while i <= NOTIF_MAX as u32 + 1 {
        let title = format!("n{i}");
        let n = make(&title);
        assert!(post(&mut s, &n) > 0, "depot accepte");
        i += 1;
    }
    assert_eq!(count(&s), NOTIF_MAX as u32);
    assert_eq!(s.dropped, 1);
    assert_eq!(bytes_of(&get(&s, 0).unwrap().title), b"n17");
    assert_eq!(get(&s, 0).unwrap().id, 17);
    assert_eq!(
        bytes_of(&get(&s, NOTIF_MAX as u32 - 1).unwrap().title),
        b"n2"
    );
    assert!(get(&s, NOTIF_MAX as u32).is_none());
    let mut s = t_notifstore::zero();
    init(&mut s);
    let a = make("A");
    let b = make("B");
    let c = make("C");
    post(&mut s, &a);
    post(&mut s, &b);
    post(&mut s, &c);
    assert_eq!(dismiss(&mut s, 2), 0);
    assert_eq!(count(&s), 2);
    assert_eq!(bytes_of(&get(&s, 0).unwrap().title), b"C");
    assert_eq!(bytes_of(&get(&s, 1).unwrap().title), b"A");
    assert_eq!(dismiss(&mut s, 2), E_NOENT);
    assert_eq!(dismiss(&mut s, 0), E_INVAL);
    assert_eq!(dismiss(&mut s, 1), 0);
    assert_eq!(bytes_of(&get(&s, 0).unwrap().title), b"C");
    assert_eq!(dismiss(&mut s, 3), 0);
    assert_eq!(count(&s), 0);
    assert!(get(&s, 0).is_none());
    let n = make("D");
    assert_eq!(post(&mut s, &n), 4, "id non reutilise");
}

#[test]
fn store_clear_et_int_max() {
    let mut s = t_notifstore::zero();
    init(&mut s);
    let n = make("A");
    post(&mut s, &n);
    unsafe {
        // SAFETY: s est aligné et exclusif.
        notif_store_clear(&mut s);
    }
    assert_eq!(count(&s), 0);
    assert_eq!(post(&mut s, &n), 2);
    let mut s = t_notifstore::zero();
    init(&mut s);
    let mut i = 1u32;
    while i <= NOTIF_MAX as u32 + 1 {
        let n = make("x");
        post(&mut s, &n);
        i += 1;
    }
    assert_eq!(s.dropped, 1);
    assert_eq!(s.next_id, 18);
    unsafe {
        // SAFETY: s est aligné et exclusif.
        notif_store_clear(&mut s);
    }
    assert_eq!(count(&s), 0);
    assert_eq!(s.dropped, 1);
    assert_eq!(s.next_id, 18);
    assert_eq!(post(&mut s, &n), 18);

    let mut s = t_notifstore::zero();
    init(&mut s);
    s.next_id = (i32::MAX as u32).wrapping_add(1);
    let n = make("Haut");
    let id = post(&mut s, &n);
    let slot = get(&s, 0).unwrap();
    assert!(id > 0);
    assert_eq!(slot.id, id as u32);
    assert_eq!(id, 1);
    assert_eq!(s.next_id, 2);
    s.next_id = 0;
    let id = post(&mut s, &n);
    let slot = get(&s, 0).unwrap();
    assert!(id > 0 && slot.id == id as u32);
    assert_eq!(id, 2);
    s.next_id = (i32::MAX as u32).wrapping_add(1);
    let id = post(&mut s, &n);
    let slot = get(&s, 0).unwrap();
    assert!(id > 0 && slot.id == id as u32);
    assert_eq!(id, 3);
    init(&mut s);
    s.next_id = i32::MAX as u32;
    let n = make("Borne");
    let id = post(&mut s, &n);
    assert_eq!(id, i32::MAX);
    assert_eq!(s.next_id, 1);
    let slot = get(&s, 0).unwrap();
    assert_eq!(slot.id, i32::MAX as u32);
    let id = post(&mut s, &n);
    assert_eq!(id, 1);
    let slot = get(&s, 0).unwrap();
    assert_eq!(slot.id, id as u32);
}

#[test]
fn store_invalides_et_champs() {
    let mut s = t_notifstore::zero();
    init(&mut s);
    let n = make("");
    assert_eq!(post(&mut s, &n), E_INVAL);
    let mut n = make("ok");
    n.text[0] = b'\n';
    assert_eq!(post(&mut s, &n), E_INVAL);
    assert_eq!(count(&s), 0);
    let mut n = make("ok");
    n.icon = 9;
    n.stamp_ns = 42;
    n.text[0] = 0x7f;
    assert_eq!(post(&mut s, &n), E_INVAL);
    n.text[0] = b'L';
    n.app[1] = 0x1f;
    assert_eq!(post(&mut s, &n), E_INVAL);
    n.app[1] = b't';
    assert_eq!(post(&mut s, &n), 1);
    let slot = get(&s, 0).unwrap();
    assert_eq!(slot.icon, 9);
    assert_eq!(slot.stamp_ns, 42);
    assert_eq!(slot.id, 1);
    let mut n = make("vide");
    n.text = [0; NOTIF_TEXT_MAX];
    assert_eq!(post(&mut s, &n), 2);
    s.count = (NOTIF_MAX as u32).wrapping_add(1);
    let n = make("trop");
    assert_eq!(post(&mut s, &n), E_INVAL);
    assert!(get(&s, 0).is_none());
    assert_eq!(dismiss(&mut s, 1), E_INVAL);
}

fn fields(app: *const c_char, title: *const c_char, text: *const c_char) -> c_int {
    unsafe {
        // SAFETY: pointeurs nuls ou chaînes du test.
        notif_fields_valid(app, title, text)
    }
}

fn msg_post(
    app: *const c_char,
    title: *const c_char,
    text: *const c_char,
    icon: u32,
) -> (c_int, t_notifmsg) {
    let mut m = t_notifmsg::zero();
    m.magic = 0x55;
    let r = unsafe {
        // SAFETY: m exclusif, chaînes nulles ou terminées.
        notif_msg_post(&mut m, app, title, text, icon)
    };
    (r, m)
}

fn msg_decode(src: &t_notifmsg, len: u32) -> (c_int, t_notifmsg) {
    let mut out = t_notifmsg::zero();
    out.magic = 0x2222;
    let r = unsafe {
        // SAFETY: src et out sont alignés, out exclusif.
        notif_msg_decode((src as *const t_notifmsg).cast(), len, &mut out)
    };
    (r, out)
}

#[test]
fn wire_oracle_e13() {
    let app = cstr("Bonjour");
    let title = cstr("Session");
    let text = cstr("Session ouverte pour Utilisateur.");
    let (r, m) = msg_post(
        app.as_ptr().cast(),
        title.as_ptr().cast(),
        text.as_ptr().cast(),
        7,
    );
    assert_eq!(r, 0);
    assert_eq!(m.magic, NOTIF_MAGIC);
    assert_eq!(m.r#type, NOTIFC_POST);
    assert_eq!(m.size, size_of::<t_notifmsg>() as u32);
    assert_eq!(m.icon, 7);
    assert_eq!(m.seq, 0);
    assert_eq!(m.id, 0);
    assert_eq!(bytes_of(&m.app), b"Bonjour");
    assert_eq!(bytes_of(&m.title), b"Session");
    assert_eq!(bytes_of(&m.text), b"Session ouverte pour Utilisateur.");
    let (r, out) = msg_decode(&m, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, 0);
    assert_eq!(bytes_of(&out.title), b"Session");
    assert_eq!(out.icon, 7);

    let (r, out) = msg_decode(&m, (size_of::<t_notifmsg>() as u32) - 1);
    assert_eq!(r, E_INVAL);
    assert_eq!(out.magic, 0x2222);
    let (r, out) = msg_decode(&m, (size_of::<t_notifmsg>() as u32) + 1);
    assert_eq!(r, E_INVAL);
    assert_eq!(out.magic, 0x2222);
    let mut bad = m;
    bad.magic ^= 1;
    let (r, out) = msg_decode(&bad, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    assert_eq!(out.magic, bad.magic);
    assert_eq!(bytes_of(&out.title), b"Session");
    bad.magic ^= 1;
    bad.size = 1;
    let (r, _) = msg_decode(&bad, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    let mut out = t_notifmsg::zero();
    out.magic = 0x2222;
    let r = unsafe {
        // SAFETY: tampon nul, out exclusif. Rien n'est lu.
        notif_msg_decode(core::ptr::null(), size_of::<t_notifmsg>() as u32, &mut out)
    };
    assert_eq!(r, E_INVAL);
    assert_eq!(out.magic, 0x2222);
    let r = unsafe {
        // SAFETY: sortie nulle. Rien n'est écrit.
        notif_msg_decode(
            (&m as *const t_notifmsg).cast(),
            size_of::<t_notifmsg>() as u32,
            core::ptr::null_mut(),
        )
    };
    assert_eq!(r, E_INVAL);

    let mut m = t_notifmsg::zero();
    unsafe {
        // SAFETY: m exclusif.
        notif_msg_init(&mut m, 0);
    }
    let (r, _) = msg_decode(&m, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    m.r#type = NOTIFC_CLEAR + 1;
    let (r, _) = msg_decode(&m, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    unsafe {
        // SAFETY: m exclusif.
        notif_msg_init(&mut m, NOTIFC_COUNT);
    }
    let (r, _) = msg_decode(&m, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, 0);
    unsafe {
        // SAFETY: m exclusif.
        notif_msg_init(&mut m, NOTIFC_DISMISS);
    }
    let (r, _) = msg_decode(&m, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    m.id = 1;
    let (r, _) = msg_decode(&m, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, 0);
    unsafe {
        // SAFETY: m exclusif.
        notif_msg_init(&mut m, NOTIFC_TOGGLE);
    }
    let (r, _) = msg_decode(&m, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, 0);
    unsafe {
        // SAFETY: pointeur nul, rien n'est écrit.
        notif_msg_init(core::ptr::null_mut(), NOTIFC_CLEAR);
    }

    let (r, m) = msg_post(
        cstr("A").as_ptr().cast(),
        cstr("T").as_ptr().cast(),
        cstr("x").as_ptr().cast(),
        0,
    );
    assert_eq!(r, 0);
    let mut bare = m;
    bare.title = [b'a'; NOTIF_TITLE_MAX];
    let (r, _) = msg_decode(&bare, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    let mut bare = m;
    bare.text = [b'b'; NOTIF_TEXT_MAX];
    let (r, _) = msg_decode(&bare, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    let mut bare = m;
    bare.app[0] = 0;
    let (r, _) = msg_decode(&bare, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, E_INVAL);
    bare.app[0] = b'A';
    bare.text = [0; NOTIF_TEXT_MAX];
    let (r, _) = msg_decode(&bare, size_of::<t_notifmsg>() as u32);
    assert_eq!(r, 0);

    let mut big = [b'z'; NOTIF_TITLE_MAX + 8];
    big[NOTIF_TITLE_MAX + 7] = 0;
    let (r, m) = msg_post(
        cstr("A").as_ptr().cast(),
        big.as_ptr().cast(),
        cstr("x").as_ptr().cast(),
        0,
    );
    assert_eq!(r, E_INVAL);
    assert_eq!(m.magic, 0x55);
    let (r, m) = msg_post(
        cstr("A").as_ptr().cast(),
        core::ptr::null(),
        cstr("x").as_ptr().cast(),
        0,
    );
    assert_eq!(r, E_INVAL);
    assert_eq!(m.magic, 0x55);
    assert_eq!(
        fields(
            core::ptr::null(),
            cstr("T").as_ptr().cast(),
            cstr("x").as_ptr().cast()
        ),
        0
    );
}

#[test]
fn wire_bornes_octets() {
    let mut title = vec![b'a'; NOTIF_TITLE_MAX - 1];
    title.push(0);
    let (r, _) = msg_post(
        cstr("A").as_ptr().cast(),
        title.as_ptr().cast(),
        cstr("x").as_ptr().cast(),
        u32::MAX,
    );
    assert_eq!(r, 0);
    let mut title = [b'a'; NOTIF_TITLE_MAX + 1];
    title[NOTIF_TITLE_MAX] = 0;
    let (r, _) = msg_post(
        cstr("A").as_ptr().cast(),
        title.as_ptr().cast(),
        cstr("x").as_ptr().cast(),
        0,
    );
    assert_eq!(r, E_INVAL);
    let mut app = vec![b'b'; NOTIF_APP_MAX - 1];
    app.push(0);
    assert_eq!(
        fields(
            app.as_ptr().cast(),
            cstr("T").as_ptr().cast(),
            cstr("").as_ptr().cast()
        ),
        1
    );
    let mut app = [b'b'; NOTIF_APP_MAX + 1];
    app[NOTIF_APP_MAX] = 0;
    assert_eq!(
        fields(
            app.as_ptr().cast(),
            cstr("T").as_ptr().cast(),
            cstr("x").as_ptr().cast()
        ),
        0
    );
    let mut text = vec![b'c'; NOTIF_TEXT_MAX - 1];
    text.push(0);
    assert_eq!(
        fields(
            cstr("A").as_ptr().cast(),
            cstr("T").as_ptr().cast(),
            text.as_ptr().cast()
        ),
        1
    );
    let mut text = [b'c'; NOTIF_TEXT_MAX + 1];
    text[NOTIF_TEXT_MAX] = 0;
    assert_eq!(
        fields(
            cstr("A").as_ptr().cast(),
            cstr("T").as_ptr().cast(),
            text.as_ptr().cast()
        ),
        0
    );
    assert_eq!(
        fields(
            cstr(" ").as_ptr().cast(),
            cstr("~").as_ptr().cast(),
            c"\x80\xff".as_ptr()
        ),
        1
    );
    assert_eq!(
        fields(
            c"\x1f".as_ptr(),
            cstr("T").as_ptr().cast(),
            cstr("x").as_ptr().cast()
        ),
        0
    );
    assert_eq!(
        fields(
            cstr("A").as_ptr().cast(),
            c"\x7f".as_ptr(),
            cstr("x").as_ptr().cast()
        ),
        0
    );
    assert_eq!(
        fields(
            c"".as_ptr(),
            cstr("T").as_ptr().cast(),
            cstr("x").as_ptr().cast()
        ),
        0
    );
}

fn badge(count: u32, size: usize) -> (usize, Vec<u8>) {
    let mut buf = vec![0x5a; size.max(1)];
    let n = if size == 0 {
        unsafe {
            // SAFETY: taille nulle, aucune écriture.
            notif_badge_text(count, buf.as_mut_ptr().cast(), 0)
        }
    } else {
        unsafe {
            // SAFETY: buf couvre size octets exclusifs.
            notif_badge_text(count, buf.as_mut_ptr().cast(), size)
        }
    };
    (n, buf)
}

fn age(now: u64, stamp: u64, size: usize) -> (usize, Vec<u8>) {
    let mut buf = vec![0x5a; size.max(1)];
    let n = if size == 0 {
        unsafe {
            // SAFETY: taille nulle, aucune écriture.
            notif_age_text(now, stamp, buf.as_mut_ptr().cast(), 0)
        }
    } else {
        unsafe {
            // SAFETY: buf couvre size octets exclusifs.
            notif_age_text(now, stamp, buf.as_mut_ptr().cast(), size)
        }
    };
    (n, buf)
}

#[test]
fn texte_oracle_e13() {
    const S: u64 = 1_000_000_000;
    let (n, buf) = badge(0, NOTIF_BADGE_MAX);
    assert_eq!(n, 0);
    assert_eq!(bytes_of(&buf), b"");
    let (n, buf) = badge(3, NOTIF_BADGE_MAX);
    assert_eq!(n, 1);
    assert_eq!(bytes_of(&buf), b"3");
    let (n, buf) = badge(99, NOTIF_BADGE_MAX);
    assert_eq!(n, 2);
    assert_eq!(bytes_of(&buf), b"99");
    let (n, buf) = badge(100, NOTIF_BADGE_MAX);
    assert_eq!(n, 3);
    assert_eq!(bytes_of(&buf), b"99+");
    let (n, buf) = badge(4_000_000_000, NOTIF_BADGE_MAX);
    assert_eq!(n, 3);
    assert_eq!(bytes_of(&buf), b"99+");
    let (n, buf) = badge(5, 0);
    assert_eq!(n, 0);
    assert_eq!(buf[0], 0x5a);
    let (n, buf) = badge(99, 2);
    assert_eq!(n, 1);
    assert_eq!(bytes_of(&buf), b"9");
    let (n, buf) = badge(NOTIF_CAP_BADGE, 1);
    assert_eq!(n, 0);
    assert_eq!(bytes_of(&buf), b"");
    assert_eq!(
        unsafe {
            // SAFETY: pointeur nul, taille non nulle, pas de déréférencement.
            notif_badge_text(5, core::ptr::null_mut(), 4)
        },
        0
    );

    let (n, buf) = age(100 * S + 14 * S, 100 * S, 24);
    assert_eq!(bytes_of(&buf), b"il y a 14 s");
    assert_eq!(n, b"il y a 14 s".len());
    let (_, buf) = age(2 * 60 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 2 min");
    let (_, buf) = age(59 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 59 s");
    let (_, buf) = age(60 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 1 min");
    let (_, buf) = age(3_600 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 1 h");
    let (_, buf) = age(86_400 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 1 j");
    let (_, buf) = age(S - 1, 0, 24);
    assert_eq!(bytes_of(&buf), "à l'instant".as_bytes());
    let (_, buf) = age(5, 10, 24);
    assert_eq!(bytes_of(&buf), "à l'instant".as_bytes());
    let (_, buf) = age(S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 1 s");
    let (_, buf) = age(0, 0, 24);
    assert_eq!(bytes_of(&buf), "à l'instant".as_bytes());
    let (_, buf) = age(3_599 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 59 min");
    let (_, buf) = age(3_600 * S - 1, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 59 min");
    let (_, buf) = age(86_399 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 23 h");
    let (_, buf) = age(2 * 86_400 * S, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 2 j");
    let (n, buf) = age(u64::MAX, 0, 24);
    assert_eq!(bytes_of(&buf), b"il y a 213503 j");
    assert_eq!(n, b"il y a 213503 j".len());
    let (n, buf) = age(14 * S, 0, 4);
    assert_eq!(n, 3);
    assert_eq!(bytes_of(&buf), b"il ");
    let (n, buf) = age(14 * S, 0, 0);
    assert_eq!(n, 0);
    assert_eq!(buf[0], 0x5a);
    assert_eq!(
        unsafe {
            // SAFETY: pointeur nul.
            notif_age_text(1, 0, core::ptr::null_mut(), 8)
        },
        0
    );
}

fn client_post(title: *const c_char) -> c_int {
    let app = cstr("Bonjour");
    let text = cstr("corps");
    unsafe {
        // SAFETY: chaînes du test, ou title nul.
        notif_post(app.as_ptr().cast(), title, text.as_ptr().cast(), 7)
    }
}

fn sends() -> usize {
    harness::read(|st| st.trace.sends.len())
}

#[test]
fn client_oracle() {
    harness::reset();
    unsafe {
        // SAFETY: pas de canal encore.
        notif_client_close();
    }
    harness::read(|st| assert!(st.trace.closed.is_empty()));

    harness::reset();
    let title = cstr("Session");
    assert_eq!(client_post(title.as_ptr().cast()), 1);
    harness::read(|st| {
        assert_eq!(st.trace.names, vec![b"notif\0".to_vec()]);
        assert_eq!(st.trace.flags, vec![0]);
        assert_eq!(st.trace.send_chan, vec![4]);
        assert_eq!(st.trace.sends.len(), 1);
        assert_eq!(st.trace.sends[0].len, size_of::<t_notifmsg>() as u64);
        assert_eq!(st.trace.sends[0].handles, 0);
        assert_eq!(st.trace.sends[0].nhandles, 0);
        assert_eq!(st.trace.sends[0].reserved, 0);
        assert_eq!(st.trace.msgs.len(), 1);
        assert_eq!(st.trace.msgs[0].seq, 1);
        assert_eq!(st.trace.msgs[0].r#type, NOTIFC_POST);
        assert_eq!(st.trace.msgs[0].icon, 7);
        assert_eq!(bytes_of(&st.trace.msgs[0].title), b"Session");
        assert_eq!(st.trace.recvs.len(), 1);
        assert_eq!(st.trace.recvs[0].timeout_ns, 30_000_000);
        assert_eq!(st.trace.recvs[0].buf_len, size_of::<t_notifmsg>() as u64);
        assert_eq!(st.trace.recv_chan, vec![4]);
    });
    assert_eq!(client_post(title.as_ptr().cast()), 1);
    harness::read(|st| {
        assert_eq!(st.trace.names.len(), 1, "canal reutilise");
        assert_eq!(st.trace.msgs[1].seq, 2);
    });

    harness::reset();
    assert_eq!(client_post(core::ptr::null()), E_INVAL);
    assert_eq!(sends(), 0);

    harness::reset();
    harness::update(|st| st.script.connect = E_NOENT as i64);
    assert_eq!(client_post(title.as_ptr().cast()), E_NOENT);
    assert_eq!(sends(), 0);

    harness::reset();
    harness::update(|st| st.script.send = E_IO);
    assert_eq!(client_post(title.as_ptr().cast()), E_IO);
    harness::read(|st| assert_eq!(st.trace.closed, vec![4]));
    harness::update(|st| {
        st.script.send = 0;
        st.trace.names.clear();
    });
    assert_eq!(client_post(title.as_ptr().cast()), 1);
    harness::read(|st| assert_eq!(st.trace.names.len(), 1, "reconnexion"));

    harness::reset();
    harness::update(|st| st.script.recv = E_TIMEOUT);
    assert_eq!(client_post(title.as_ptr().cast()), E_TIMEOUT);
    harness::read(|st| assert!(st.trace.closed.is_empty()));
    harness::update(|st| st.script.recv = 0);
    let before = harness::read(|st| st.trace.names.len());
    assert_eq!(client_post(title.as_ptr().cast()), 1);
    harness::read(|st| assert_eq!(st.trace.names.len(), before, "canal garde"));

    harness::reset();
    harness::update(|st| st.script.reply.magic = 0);
    assert_eq!(client_post(title.as_ptr().cast()), E_INVAL);
    harness::read(|st| assert!(st.trace.closed.is_empty()));

    harness::reset();
    harness::update(|st| st.script.out_len = (size_of::<t_notifmsg>() as u64) - 1);
    assert_eq!(client_post(title.as_ptr().cast()), E_INVAL);

    harness::reset();
    harness::update(|st| {
        st.script.reply.id = 9;
        st.script.reply.status = NOTIFS_OK as i32;
    });
    assert_eq!(client_post(title.as_ptr().cast()), 9);

    harness::reset();
    harness::update(|st| {
        st.script.reply.id = 3;
        st.script.reply.status = E_NOENT;
    });
    assert_eq!(client_post(title.as_ptr().cast()), E_NOENT);

    harness::reset();
    harness::update(|st| st.script.connect = 0);
    assert_eq!(client_post(title.as_ptr().cast()), 1);
    assert_eq!(client_post(title.as_ptr().cast()), 1);
    harness::read(|st| {
        assert_eq!(st.trace.send_chan, vec![0, 0]);
        assert_eq!(st.trace.names.len(), 2, "handle 0 ne reste pas ouvert");
    });

    harness::reset();
    harness::update(|st| {
        st.script.mono = 1_000;
        st.script.reply.count = 6;
    });
    let mut n = 0u32;
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        0
    );
    assert_eq!(n, 6);
    assert_eq!(sends(), 1);
    harness::update(|st| st.script.mono = 1_000 + 249_999_999);
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        0
    );
    assert_eq!(sends(), 1, "cache");
    harness::update(|st| st.script.mono = 500);
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        0
    );
    assert_eq!(sends(), 1, "horloge en arriere, cache");
    harness::update(|st| st.script.mono = 1_000 + 250_000_000);
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        0
    );
    assert_eq!(sends(), 2, "fenetre expiree");
    assert_eq!(client_post(title.as_ptr().cast()), 1);
    assert_eq!(sends(), 3);
    harness::update(|st| st.script.mono = 1_000 + 250_000_000);
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        0
    );
    assert_eq!(sends(), 3, "post ne vide pas le cache");

    assert_eq!(
        unsafe {
            // SAFETY: pas d'argument.
            notif_client_toggle()
        },
        0
    );
    harness::read(|st| {
        let m = st.trace.msgs.last().unwrap();
        assert_eq!(m.r#type, NOTIFC_TOGGLE);
        assert_eq!(m.id, 0);
    });
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        0
    );
    harness::read(|st| assert_eq!(st.trace.msgs.last().unwrap().r#type, NOTIFC_COUNT));

    assert_eq!(
        unsafe {
            // SAFETY: id recopié dans le message.
            notif_client_dismiss(8)
        },
        0
    );
    harness::read(|st| {
        let m = st.trace.msgs.last().unwrap();
        assert_eq!(m.r#type, NOTIFC_DISMISS);
        assert_eq!(m.id, 8);
    });
    assert_eq!(
        unsafe {
            // SAFETY: pas d'argument.
            notif_client_clear()
        },
        0
    );
    harness::read(|st| {
        let m = st.trace.msgs.last().unwrap();
        assert_eq!(m.r#type, NOTIFC_CLEAR);
        assert_eq!(m.id, 0);
    });

    harness::reset();
    harness::update(|st| {
        st.script.mono = 50;
        st.script.reply.status = E_NOENT;
    });
    let mut n = 0xffff_ffff;
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        E_NOENT
    );
    assert_eq!(n, 0xffff_ffff);
    harness::update(|st| {
        st.script.reply.status = 0;
        st.script.reply.count = 2;
    });
    assert_eq!(
        unsafe {
            // SAFETY: n est aligné et exclusif.
            notif_client_count(&mut n)
        },
        0
    );
    assert_eq!(n, 2);
    assert_eq!(sends(), 2, "echec ne arme pas le cache");
    assert_eq!(
        unsafe {
            // SAFETY: pointeur nul.
            notif_client_count(core::ptr::null_mut())
        },
        E_INVAL
    );

    harness::reset();
    harness::update(|st| {
        st.script.mono = 10;
        st.script.reply.count = 1;
    });
    let mut n = 0u32;
    unsafe {
        // SAFETY: n est aligné et exclusif.
        notif_client_count(&mut n);
    }
    unsafe {
        // SAFETY: ferme le canal ouvert par le comptage.
        notif_client_close();
    }
    harness::read(|st| assert_eq!(st.trace.closed, vec![4]));
    unsafe {
        // SAFETY: canal déjà fermé, v_close ne doit pas être rappelé.
        notif_client_close();
    }
    harness::read(|st| assert_eq!(st.trace.closed, vec![4]));
    let sent = sends();
    unsafe {
        // SAFETY: n est aligné et exclusif.
        notif_client_count(&mut n);
    }
    assert_eq!(sends(), sent + 1, "close vide le cache");
}
