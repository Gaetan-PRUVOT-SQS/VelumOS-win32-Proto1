use core::ffi::c_int;
use core::mem::{align_of, offset_of, size_of};
use std::sync::Mutex;

use super::{
    fake, set_dark_at, set_default, set_format, set_load, set_mode_get, set_parse, set_save,
    t_setmode, t_settings,
};

const PATH: &[u8] = b"/data/reglages";
const PATH_C: &[u8] = b"/data/reglages\0";
const TMP: &[u8] = b"/data/reglages.tmp";
const G_DARK: &[u8] = b"theme=sombre\ntransparence=reduite\nclavier=qwerty\nresolution=1920x1080\n";
const G_LIGHT: &[u8] = b"theme=clair\ntransparence=normale\nclavier=azerty\nresolution=1280x720\n";
const G_AUTO: &[u8] = b"theme=auto\ntransparence=normale\nclavier=azerty\nresolution=1024x768\n";

fn guard() -> std::sync::MutexGuard<'static, ()> {
    static LOCK: Mutex<()> = Mutex::new(());
    match LOCK.lock() {
        Ok(g) => g,
        Err(e) => e.into_inner(),
    }
}

fn dirty() -> t_settings {
    t_settings {
        theme: 9,
        reduced: 9,
        layout: 9,
        mode: 9,
    }
}

fn mode(index: u32) -> (u32, u32) {
    // SAFETY: aucun pointeur.
    let m = unsafe { set_mode_get(index) };
    (m.w, m.h)
}

fn dark(s: *const t_settings, hour: u32) -> bool {
    // SAFETY: s est nul, ou bien aligné et lisible le temps de l'appel.
    unsafe { set_dark_at(s, hour) }
}

fn load_ptr(path: *const core::ffi::c_char, s: *mut t_settings) -> c_int {
    // SAFETY: s est nul ou aligné. path est nul ou une chaîne C.
    unsafe { set_load(path, s) }
}

fn save_ptr(path: *const core::ffi::c_char, s: *const t_settings) -> c_int {
    // SAFETY: path est nul ou une chaîne C. s est nul ou aligné.
    unsafe { set_save(path, s) }
}

fn expect_default(s: &t_settings) {
    assert_eq!(s.theme, 0, "theme par defaut clair");
    assert_eq!(s.reduced, 0, "transparence normale");
    assert_eq!(s.layout, 0, "clavier azerty");
    assert_eq!(s.mode, 1);
    assert_eq!(mode(s.mode), (1280, 720), "resolution 1280x720");
}

fn parse_at(text: &[u8], s: &mut t_settings) -> c_int {
    // SAFETY: text vit pendant l'appel. s est aligné et exclusif.
    unsafe { set_parse(text.as_ptr().cast(), text.len(), s) }
}

fn format_into(s: &t_settings, out: &mut [u8]) -> c_int {
    let max = out.len();
    let ptr = if out.is_empty() {
        core::ptr::null_mut()
    } else {
        out.as_mut_ptr()
    };
    // SAFETY: s aligné. out est vide, ou bien ses octets sont exclusifs.
    unsafe { set_format(s, ptr.cast(), max) }
}

fn format_cap(s: &t_settings, out: *mut u8, max: usize) -> c_int {
    // SAFETY: s aligné. out est nul, ou bien max octets exclusifs.
    unsafe { set_format(s, out.cast(), max) }
}

fn load_at(path: &[u8], s: &mut t_settings) -> c_int {
    // SAFETY: path contient un NUL. s est aligné et exclusif.
    unsafe { set_load(path.as_ptr().cast(), s) }
}

fn save_at(path: &[u8], s: &t_settings) -> c_int {
    // SAFETY: path contient un NUL, ou bien 92 octets lisibles sans NUL. s est aligné.
    unsafe { set_save(path.as_ptr().cast(), s) }
}

fn raw(s: &t_settings) -> &[u8] {
    // SAFETY: s est un objet complet. La tranche couvre sa représentation.
    unsafe { core::slice::from_raw_parts(core::ptr::from_ref(s).cast(), size_of::<t_settings>()) }
}

fn same(path: &[u8], want: &[u8]) -> bool {
    let mut buf = [0u8; 512];
    let n = fake::slurp(path, &mut buf);
    let Ok(need) = c_int::try_from(want.len()) else {
        return false;
    };
    n == need && buf[..want.len()] == *want
}

fn kept(dark: &[u8]) {
    assert_eq!(fake::nfiles(), 1);
    let mut scratch = [0u8; 8];
    assert_eq!(fake::slurp(TMP, &mut scratch), -2);
    assert!(same(PATH, dark), "ancien fichier intact");
}

#[test]
fn tailles_abi() {
    assert_eq!(size_of::<usize>(), 8);
    assert_eq!(size_of::<t_setmode>(), 8);
    assert_eq!(align_of::<t_setmode>(), 4);
    assert_eq!(offset_of!(t_setmode, w), 0);
    assert_eq!(offset_of!(t_setmode, h), 4);
    assert_eq!(size_of::<t_settings>(), 16);
    assert_eq!(align_of::<t_settings>(), 4);
    assert_eq!(offset_of!(t_settings, theme), 0);
    assert_eq!(offset_of!(t_settings, reduced), 4);
    assert_eq!(offset_of!(t_settings, layout), 8);
    assert_eq!(offset_of!(t_settings, mode), 12);
}

#[test]
fn e12_set_defaut() {
    let mut s = dirty();
    // SAFETY: s est aligné et exclusif.
    unsafe { set_default(&mut s) };
    expect_default(&s);
    assert_eq!(mode(0), (1024, 768));
    assert_eq!(mode(2), (1366, 768));
    assert_eq!(mode(3), (1600, 900));
    assert_eq!(mode(4), (1920, 1080));
    assert_eq!(mode(5), (1024, 768));
    assert_eq!(mode(u32::MAX), (1024, 768));
}

#[test]
fn e12_set_parse_valide() {
    let txt = b"theme=sombre\ntransparence=reduite\nclavier=qwerty\nresolution=1920x1080\n";
    let mut s = dirty();
    assert_eq!(parse_at(txt, &mut s), 4, "4 cles reconnues");
    assert_eq!(s.theme, 1, "theme sombre");
    assert_eq!(s.reduced, 1, "transparence reduite");
    assert_eq!(s.layout, 1, "clavier qwerty");
    assert_eq!(mode(s.mode).1, 1080, "hauteur 1080");
    assert_eq!(mode(s.mode).0, 1920);
}

#[test]
fn e12_set_parse_tolerant() {
    let txt =
        b"theme=violet\r\ninconnue=1\nsans egal\nclavier=\n=azerty\ntheme=auto\r\nclavier=qwerty";
    let mut s = dirty();
    assert_eq!(parse_at(txt, &mut s), 2, "seules 2 cles valides");
    assert_eq!(s.theme, 2, "derniere valeur valide gagne");
    assert_eq!(s.layout, 1, "fin sans saut de ligne");
    assert_eq!(s.reduced, 0, "cle absente = defaut");
    assert_eq!(s.mode, 1);
}

#[test]
fn e12_set_parse_limites() {
    let big = [b'a'; 512];
    let more = [b'a'; 513];
    let mut s = dirty();
    assert_eq!(parse_at(b"", &mut s), 0, "vide");
    expect_default(&s);
    s = dirty();
    // SAFETY: texte nul, len non nul. s aligné. Rien n'est écrit.
    let rc = unsafe { set_parse(core::ptr::null(), 4, &mut s) };
    assert_eq!(rc, -22, "NULL refuse");
    assert_eq!(s.theme, 9);
    // SAFETY: s nul.
    let rc = unsafe { set_parse(b"".as_ptr().cast(), 0, core::ptr::null_mut()) };
    assert_eq!(rc, -22, "sortie NULL refuse");
    s = dirty();
    // SAFETY: texte nul et len 0. s aligné.
    let rc = unsafe { set_parse(core::ptr::null(), 0, &mut s) };
    assert_eq!(rc, 0);
    expect_default(&s);
    s = dirty();
    assert_eq!(parse_at(&big, &mut s), 0, "exactement SET_FILE_MAX accepte");
    expect_default(&s);
    s = dirty();
    assert_eq!(parse_at(&more, &mut s), -34, "SET_FILE_MAX + 1 refuse");
    expect_default(&s);
}

#[test]
fn bornes_ligne() {
    let mut s = dirty();
    let mut line = b"theme=".to_vec();
    line.extend_from_slice(&[b'x'; 16]);
    line.push(b'\n');
    line.extend_from_slice(b"theme=sombre\n");
    assert_eq!(parse_at(&line, &mut s), 1);
    assert_eq!(s.theme, 1);
    line = b"theme=".to_vec();
    line.extend_from_slice(&[b'x'; 15]);
    line.push(b'\n');
    assert_eq!(parse_at(&line, &mut s), 0);
    expect_default(&s);
    assert_eq!(parse_at(b"clavier=\ntheme=auto\n", &mut s), 1);
    assert_eq!(s.layout, 0);
    assert_eq!(s.theme, 2);
    assert_eq!(parse_at(b"theme=sombre\0xx\n", &mut s), 1);
    assert_eq!(s.theme, 1);
    assert_eq!(parse_at(b"theme=clair\ntheme=sombre\n", &mut s), 2);
    assert_eq!(s.theme, 1);
}

#[test]
fn e12_set_aller_retour() {
    let cases = [
        (
            0u32,
            0u32,
            0u32,
            1u32,
            b"theme=clair\ntransparence=normale\nclavier=azerty\nresolution=1280x720\n" as &[u8],
        ),
        (
            1,
            1,
            1,
            2,
            b"theme=sombre\ntransparence=reduite\nclavier=qwerty\nresolution=1366x768\n",
        ),
        (
            2,
            0,
            0,
            3,
            b"theme=auto\ntransparence=normale\nclavier=azerty\nresolution=1600x900\n",
        ),
    ];
    for (theme, reduced, layout, mode_i, text) in cases {
        let a = t_settings {
            theme,
            reduced,
            layout,
            mode: mode_i,
        };
        let mut out = [0u8; 512];
        let n = format_into(&a, &mut out);
        assert!(n > 0, "format reussi");
        let n_usz = match usize::try_from(n) {
            Ok(v) => v,
            Err(_) => panic!("longueur"),
        };
        assert_eq!(&out[..n_usz], text);
        assert_eq!(out[n_usz], 0);
        let mut b = dirty();
        assert_eq!(parse_at(&out[..n_usz], &mut b), 4, "4 cles relues");
        assert_eq!(raw(&a), raw(&b), "aller-retour identique");
    }
}

#[test]
fn e12_set_format_limites() {
    let mut s = dirty();
    // SAFETY: s aligné.
    unsafe { set_default(&mut s) };
    let mut out = [0u8; 8];
    assert_eq!(format_into(&s, &mut out), -34, "tampon trop court");
    assert_eq!(format_cap(&s, out.as_mut_ptr(), 0), -22, "tampon nul");
    // SAFETY: sortie nulle.
    let rc = unsafe { set_format(&s, core::ptr::null_mut(), 8) };
    assert_eq!(rc, -22);
    // SAFETY: reglages nuls.
    let rc = unsafe { set_format(core::ptr::null(), out.as_mut_ptr().cast(), out.len()) };
    assert_eq!(rc, -22);
    s.theme = 3;
    assert_eq!(format_into(&s, &mut out), -22, "theme hors borne");
    let mut wide = [0u8; 128];
    assert_eq!(format_into(&s, &mut wide), -22);
    // SAFETY: s aligné.
    unsafe { set_default(&mut s) };
    s.mode = 5;
    assert_eq!(format_into(&s, &mut out), -22, "mode hors borne");
    // SAFETY: s aligné et exclusif.
    unsafe { set_default(&mut s) };
    s.reduced = 2;
    assert_eq!(format_into(&s, &mut wide), -22);
    // SAFETY: s aligné et exclusif.
    unsafe { set_default(&mut s) };
    s.layout = 2;
    assert_eq!(format_into(&s, &mut wide), -22);
    // SAFETY: s aligné et exclusif.
    unsafe { set_default(&mut s) };
    let mut tight = vec![0u8; G_LIGHT.len()];
    assert_eq!(format_into(&s, &mut tight), -34);
    let mut room = vec![0u8; G_LIGHT.len() + 1];
    let lit = c_int::try_from(G_LIGHT.len()).unwrap_or(-1);
    assert_eq!(format_into(&s, &mut room), lit);
    assert_eq!(&room[..G_LIGHT.len()], G_LIGHT);
    assert_eq!(room[G_LIGHT.len()], 0);
}

#[test]
fn e12_set_heures_auto() {
    let mut s = dirty();
    // SAFETY: s aligné.
    unsafe { set_default(&mut s) };
    s.theme = 2;
    assert!(!dark(&s, 7), "7 h : clair (limite)");
    assert!(dark(&s, 6), "6 h : sombre");
    assert!(!dark(&s, 18), "18 h : clair");
    assert!(dark(&s, 19), "19 h : sombre (limite)");
    assert!(dark(&s, 0), "minuit : sombre");
    s.theme = 1;
    assert!(dark(&s, 12), "sombre force");
    s.theme = 0;
    assert!(!dark(&s, 23), "clair force");
    s.theme = 3;
    assert!(dark(&s, 19));
    assert!(!dark(&s, 12));
    assert!(!dark(core::ptr::null(), 0));
    // SAFETY: pointeur nul, aucune ecriture.
    unsafe { set_default(core::ptr::null_mut()) };
}

#[test]
fn e12_persist_aller_retour() {
    let _g = guard();
    fake::reset();
    assert_eq!(fake::plant(PATH, G_DARK), 0, "fichier depose");
    let mut s = dirty();
    assert_eq!(load_at(PATH_C, &mut s), 4, "4 cles relues");
    assert_eq!(s.theme, 1, "sombre relu");
    assert_eq!(s.reduced, 1, "transparence relue");
    assert_eq!(s.layout, 1, "qwerty relu");
    assert_eq!(mode(s.mode).0, 1920, "1920 relu");
    assert!(dark(&s, 12), "midi reste sombre");
    s = t_settings {
        theme: 0,
        reduced: 0,
        layout: 0,
        mode: 1,
    };
    assert_eq!(save_at(PATH_C, &s), 0, "ecriture clair");
    assert_eq!(fake::nfiles(), 1, "un seul fichier");
    let mut scratch = [0u8; 16];
    assert_eq!(fake::slurp(TMP, &mut scratch), -2, "temporaire efface");
    assert!(same(PATH, G_LIGHT), "octets clair sur le fichier");
    let mut back = dirty();
    assert_eq!(load_at(PATH_C, &mut back), 4, "rechargement");
    assert_eq!(raw(&s), raw(&back), "clair identique apres relecture");
    assert!(!dark(&back, 23), "clair force apres relecture");
    s.theme = 2;
    s.mode = 0;
    assert_eq!(save_at(PATH_C, &s), 0, "ecriture auto");
    assert!(same(PATH, G_AUTO), "octets auto sur le fichier");
    assert_eq!(load_at(PATH_C, &mut back), 4, "auto relu");
    assert_eq!(back.theme, 2, "theme auto");
    assert!(dark(&back, 6), "6 h sombre apres relecture");
    assert!(!dark(&back, 7), "7 h clair apres relecture");
    assert!(!dark(&back, 18), "18 h clair apres relecture");
    assert!(dark(&back, 19), "19 h sombre apres relecture");
    assert!(dark(&back, 0), "minuit sombre apres relecture");
}

#[test]
fn drapeaux_open() {
    let _g = guard();
    fake::reset();
    let mut s = dirty();
    // SAFETY: s aligné.
    unsafe { set_default(&mut s) };
    assert_eq!(save_at(PATH_C, &s), 0);
    let ops = fake::opened();
    assert_eq!(ops.len(), 1);
    assert_eq!(ops[0].flags, 0x0001 | 0x0040 | 0x0200);
    assert_eq!(ops[0].mode, 0o644);
    assert_eq!(ops[0].path, b"/data/reglages.tmp");
    fake::reset();
    assert_eq!(fake::plant(PATH, G_DARK), 0);
    assert_eq!(load_at(PATH_C, &mut s), 4);
    let ops = fake::opened();
    assert_eq!(ops.len(), 1);
    assert_eq!(ops[0].flags, 0);
    assert_eq!(ops[0].mode, 0);
    assert_eq!(ops[0].path, PATH);
}

#[test]
fn e12_persist_bornes() {
    let _g = guard();
    fake::reset();
    let mut s = dirty();
    assert_eq!(load_at(PATH_C, &mut s), -2, "absent");
    expect_default(&s);
    let opens = fake::opened().len();
    assert_eq!(load_ptr(PATH_C.as_ptr().cast(), core::ptr::null_mut()), -22);
    s.theme = 4;
    assert_eq!(load_ptr(core::ptr::null(), &mut s), -22);
    expect_default(&s);
    assert_eq!(fake::opened().len(), opens);
    // SAFETY: s aligné.
    unsafe { set_default(&mut s) };
    assert_eq!(save_ptr(core::ptr::null(), &s), -22, "sauve NULL");
    assert_eq!(save_ptr(PATH_C.as_ptr().cast(), core::ptr::null()), -22);
    assert_eq!(fake::nfiles(), 0, "rien ecrit");
    s.theme = 3;
    assert_eq!(save_at(PATH_C, &s), -22, "theme hors domaine");
    s.theme = 0;
    s.reduced = 2;
    assert_eq!(save_at(PATH_C, &s), -22, "transparence hors domaine");
    s.reduced = 0;
    s.layout = 2;
    assert_eq!(save_at(PATH_C, &s), -22);
    s.layout = 0;
    s.mode = 5;
    assert_eq!(save_at(PATH_C, &s), -22, "mode hors domaine");
    assert_eq!(fake::nfiles(), 0, "toujours rien");
    let mut p91 = [b'c'; 92];
    p91[91] = 0;
    let mut p92 = [b'c'; 93];
    p92[92] = 0;
    let bare = [b'c'; 92];
    // SAFETY: s aligné.
    unsafe { set_default(&mut s) };
    assert_eq!(save_at(&p92, &s), -34, "chemin 92");
    assert_eq!(save_at(&bare, &s), -34);
    assert_eq!(fake::nfiles(), 0, "92 ne cree rien");
    assert_eq!(save_at(&p91, &s), 0, "chemin 91");
    assert_eq!(fake::nfiles(), 1, "91 cree un fichier");
    assert!(same(&p91[..91], G_LIGHT));
    let big = [b'a'; 512];
    let more = [b'a'; 513];
    fake::reset();
    assert_eq!(fake::plant(PATH, &big), 0, "depot max");
    assert_eq!(load_at(PATH_C, &mut s), 0, "max relu");
    expect_default(&s);
    let mut prefixed = G_DARK.to_vec();
    prefixed.resize(513, b'a');
    assert_eq!(fake::plant(PATH, &prefixed), 0);
    s.theme = 9;
    assert_eq!(load_at(PATH_C, &mut s), -34, "max+1 refuse");
    expect_default(&s);
    fake::reset();
    assert_eq!(fake::plant(PATH, &more), 0, "depot max+1");
    s.theme = 9;
    assert_eq!(load_at(PATH_C, &mut s), -34);
    expect_default(&s);
}

#[test]
fn e12_persist_echecs() {
    let _g = guard();
    fake::reset();
    assert_eq!(fake::plant(PATH, G_DARK), 0, "depot sombre");
    let s = t_settings {
        theme: 0,
        reduced: 0,
        layout: 0,
        mode: 1,
    };
    fake::open_fail(true);
    assert_eq!(save_at(PATH_C, &s), -5, "ouverture refusee");
    kept(G_DARK);
    fake::open_fail(false);
    fake::write_budget(0);
    assert_eq!(save_at(PATH_C, &s), -5, "ecriture refusee");
    kept(G_DARK);
    fake::write_budget(4);
    assert_eq!(save_at(PATH_C, &s), -5, "ecriture partielle");
    kept(G_DARK);
    fake::write_budget(-1);
    fake::fsync_fail(true);
    assert_eq!(save_at(PATH_C, &s), -5, "fsync refuse");
    kept(G_DARK);
    fake::fsync_fail(false);
    fake::rename_fail(true);
    assert_eq!(save_at(PATH_C, &s), -5, "renommage refuse");
    kept(G_DARK);
    fake::rename_fail(false);
    fake::read_budget(0);
    let mut back = t_settings {
        theme: 9,
        reduced: 0,
        layout: 0,
        mode: 0,
    };
    assert_eq!(load_at(PATH_C, &mut back), -5, "lecture refusee");
    assert_eq!(back.theme, 0, "pas de melange");
    fake::read_budget(8);
    back.theme = 9;
    assert_eq!(load_at(PATH_C, &mut back), -5, "lecture partielle");
    assert_eq!(back.theme, 0, "coupure avant analyse");
    fake::read_budget(-1);
    assert_eq!(load_at(PATH_C, &mut back), 4, "fichier encore sombre");
    assert_eq!(back.theme, 1, "sombre conserve");
}

#[test]
fn modes_noms() {
    let rows: [(&[u8], u32, u32, u32); 5] = [
        (b"1024x768", 0, 1024, 768),
        (b"1280x720", 1, 1280, 720),
        (b"1366x768", 2, 1366, 768),
        (b"1600x900", 3, 1600, 900),
        (b"1920x1080", 4, 1920, 1080),
    ];
    for (name, idx, w, h) in rows {
        let mut line = b"resolution=".to_vec();
        line.extend_from_slice(name);
        line.push(b'\n');
        let mut s = dirty();
        assert_eq!(parse_at(&line, &mut s), 1);
        assert_eq!(s.mode, idx);
        assert_eq!(mode(idx), (w, h));
        assert_eq!(s.theme, 0);
    }
}
