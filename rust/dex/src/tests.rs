use crate::ffi::{
    dex_adler32, dex_cdata_next, dex_cdata_open, dex_class, dex_code, dex_field, dex_find_class,
    dex_handler_next, dex_handler_open, dex_list_at, dex_method, dex_mutf8_check, dex_open,
    dex_proto, dex_sleb, dex_static_value, dex_string, dex_try_find, dex_type, dex_u16, dex_u32,
    dex_uleb, dex_ulebp1, mutf8_to_utf8,
};
use crate::{
    ACC_FINAL, ACC_PUBLIC, DEX_M_DIRECT, DEX_NO_INDEX, DEX_T_CLASS, DEX_T_FIELD, DEX_T_METHOD,
    DEX_T_PROTO, DEX_T_STRING, DEX_T_TYPE, DEX_V_LONG, DEX_V_NULL, DEX_V_STRING, E_INVAL, E_NOENT,
    E_NOTSUP, E_OK, E_OVERFLOW, E_RANGE, t_dex, t_dexcatch, t_dexcdata, t_dexclass, t_dexcode,
    t_dexcur, t_dexfield, t_dexhit, t_dexlist, t_dexmember, t_dexmethod, t_dexproto, t_dexstr,
    t_dexvalue, t_span, t_text,
};
use core::ffi::{c_char, c_int};
use core::mem::{align_of, offset_of, size_of};

const FIXTURE: &[u8] = include_bytes!("../fixture/classes.dex");
const NAME_MUTF8: &[u8] = b"V\xc3\xa9lum \xe2\x82\xac \xed\xa0\xbd\xed\xb8\x80";
const NAME_UTF8: &[u8] = b"V\xc3\xa9lum \xe2\x82\xac \xf0\x9f\x98\x80";
const IDX: [u32; 12] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 2, 3, 4];
const KIND: [u32; 12] = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3];

fn blank() -> t_dex {
    t_dex {
        p: core::ptr::null(),
        len: 0,
        n: [0; 6],
        off: [0; 6],
    }
}

fn blank_str() -> t_dexstr {
    t_dexstr {
        p: core::ptr::null(),
        size: 0,
        utf16: 0,
    }
}

fn blank_class() -> t_dexclass {
    t_dexclass {
        class_idx: 0,
        access: 0,
        superclass_idx: 0,
        interfaces: t_dexlist {
            p: core::ptr::null(),
            n: 0,
        },
        source_file_idx: 0,
        class_data_off: 0,
        static_values_off: 0,
    }
}

fn blank_code() -> t_dexcode {
    t_dexcode {
        insns: core::ptr::null(),
        registers: 0,
        ins: 0,
        outs: 0,
        tries: 0,
        insns_size: 0,
        off: 0,
        tries_off: 0,
        handlers_off: 0,
    }
}

fn open_at(d: &mut t_dex, bytes: &[u8]) -> c_int {
    let file = t_span {
        p: bytes.as_ptr(),
        len: bytes.len(),
    };
    // SAFETY: d est exclusif, bytes vit le temps de l'appel.
    unsafe { dex_open(d, file) }
}

fn seal(p: &mut [u8]) {
    if p.len() < 12 {
        return;
    }
    // SAFETY: p a au moins 12 octets, la somme part de l'indice 12.
    let sum = unsafe { dex_adler32(p.as_ptr().add(12), p.len() - 12) };
    p[8] = sum as u8;
    p[9] = (sum >> 8) as u8;
    p[10] = (sum >> 16) as u8;
    p[11] = (sum >> 24) as u8;
}

fn patched(base: &[u8], off: usize, v: u32, width: u32) -> c_int {
    let mut p = base.to_vec();
    let mut i = 0usize;
    while i < (width & 7) as usize && off + i < p.len() {
        p[off + i] = (v >> (8 * i)) as u8;
        i += 1;
    }
    if width & 8 == 0 {
        seal(&mut p);
    }
    let mut d = blank();
    open_at(&mut d, &p)
}

fn hdr(base: &[u8], off: usize) -> u32 {
    // SAFETY: off est dans l'en-tête du fixture, quatre octets lisibles.
    unsafe { dex_u32(base.as_ptr().add(off)) }
}

fn u16p(p: *const u8) -> u32 {
    // SAFETY: p pointe deux octets d'un code_item déjà accepté.
    unsafe { dex_u16(p) }
}

fn cur_of(bytes: &[u8]) -> t_dexcur {
    t_dexcur {
        p: bytes.as_ptr(),
        len: bytes.len(),
        pos: 0,
        err: 0,
    }
}

fn str_eq(s: &t_dexstr, want: &[u8]) -> bool {
    let Ok(n) = usize::try_from(s.size) else {
        return false;
    };
    if s.p.is_null() || n != want.len() {
        return false;
    }
    let Some(end) = n.checked_add(1) else {
        return false;
    };
    // SAFETY: dex_string garantit size octets et le NUL qui suit.
    let bytes = unsafe { core::slice::from_raw_parts(s.p.cast::<u8>(), end) };
    bytes[..n] == *want && bytes[n] == 0
}

fn named(d: &t_dex, idx: u32, want: &[u8]) -> bool {
    let mut s = blank_str();
    // SAFETY: d est ouvert, s est exclusif.
    unsafe { dex_string(d, idx, &mut s) == E_OK && str_eq(&s, want) }
}

fn string_idx(d: &t_dex, want: &[u8]) -> Option<u32> {
    let mut i = 0u32;
    while i < d.n[DEX_T_STRING as usize] {
        if named(d, i, want) {
            return Some(i);
        }
        i += 1;
    }
    None
}

fn cstr(bytes: &[u8]) -> *const c_char {
    bytes.as_ptr().cast()
}

fn opened() -> t_dex {
    let mut d = blank();
    assert_eq!(open_at(&mut d, FIXTURE), E_OK);
    d
}

fn code_of(d: &t_dex, def_idx: u32, method_idx: u32) -> u32 {
    let mut c = blank_class();
    // SAFETY: d est ouvert, c est exclusif.
    if unsafe { dex_class(d, def_idx, &mut c) } != E_OK {
        return 0;
    }
    let mut it = t_dexcdata {
        d: core::ptr::null(),
        pos: 0,
        left: [0; 4],
        kind: 0,
        last: 0,
    };
    // SAFETY: d et c vivent, it est exclusif.
    if unsafe { dex_cdata_open(d, &c, &mut it) } != E_OK {
        return 0;
    }
    let mut m = t_dexmember {
        kind: 0,
        idx: 0,
        access: 0,
        code_off: 0,
    };
    loop {
        // SAFETY: it porte d, m est exclusif.
        if unsafe { dex_cdata_next(&mut it, &mut m) } != E_OK {
            return 0;
        }
        if m.kind >= DEX_M_DIRECT && m.idx == method_idx {
            return m.code_off;
        }
    }
}

fn adler_ref(p: &[u8]) -> u32 {
    let mut a = 1u32;
    let mut b = 0u32;
    for byte in p {
        a = (a + u32::from(*byte)) % 65521;
        b = (b + a) % 65521;
    }
    (b << 16) | a
}

#[test]
fn abi_sizes() {
    assert_eq!(size_of::<t_dex>(), 64);
    assert_eq!(align_of::<t_dex>(), 8);
    assert_eq!(size_of::<t_dexstr>(), 16);
    assert_eq!(align_of::<t_dexstr>(), 8);
    assert_eq!(size_of::<t_dexlist>(), 16);
    assert_eq!(align_of::<t_dexlist>(), 8);
    assert_eq!(size_of::<t_dexproto>(), 24);
    assert_eq!(align_of::<t_dexproto>(), 8);
    assert_eq!(size_of::<t_dexfield>(), 12);
    assert_eq!(align_of::<t_dexfield>(), 4);
    assert_eq!(size_of::<t_dexmethod>(), 12);
    assert_eq!(align_of::<t_dexmethod>(), 4);
    assert_eq!(size_of::<t_dexclass>(), 48);
    assert_eq!(align_of::<t_dexclass>(), 8);
    assert_eq!(offset_of!(t_dexclass, interfaces), 16);
    assert_eq!(offset_of!(t_dexclass, source_file_idx), 32);
    assert_eq!(size_of::<t_dexcdata>(), 40);
    assert_eq!(align_of::<t_dexcdata>(), 8);
    assert_eq!(offset_of!(t_dexcdata, left), 12);
    assert_eq!(size_of::<t_dexmember>(), 16);
    assert_eq!(align_of::<t_dexmember>(), 4);
    assert_eq!(size_of::<t_dexcode>(), 40);
    assert_eq!(align_of::<t_dexcode>(), 8);
    assert_eq!(size_of::<t_dexhit>(), 24);
    assert_eq!(align_of::<t_dexhit>(), 8);
    assert_eq!(size_of::<t_dexcatch>(), 8);
    assert_eq!(align_of::<t_dexcatch>(), 4);
    assert_eq!(size_of::<t_dexvalue>(), 16);
    assert_eq!(align_of::<t_dexvalue>(), 8);
    assert_eq!(offset_of!(t_dexvalue, i), 8);
    assert_eq!(size_of::<t_span>(), 16);
    assert_eq!(align_of::<t_span>(), 8);
    assert_eq!(size_of::<t_text>(), 16);
    assert_eq!(align_of::<t_text>(), 8);
    assert_eq!(size_of::<t_dexcur>(), 32);
    assert_eq!(align_of::<t_dexcur>(), 8);
    assert_eq!(offset_of!(t_dexcur, err), 24);
}

#[test]
fn uleb_sleb_limits() {
    let cases_u: &[(&[u8], u32, c_int)] = &[
        (b"\x00", 0, 0),
        (b"\x7f", 127, 0),
        (b"\x80\x01", 128, 0),
        (b"\xff\x7f", 16383, 0),
        (b"\x80\x80\x01", 16384, 0),
        (b"\xff\xff\xff\x7f", 0x0fff_ffff, 0),
        (b"\x80\x80\x80\x80\x01", 0x1000_0000, 0),
        (b"\xff\xff\xff\xff\x0f", 0xffff_ffff, 0),
    ];
    for (bytes, want, err) in cases_u {
        let mut c = cur_of(bytes);
        // SAFETY: c est exclusif et décrit bytes.
        let v = unsafe { dex_uleb(&mut c) };
        assert_eq!(v, *want);
        assert_eq!(c.err, *err);
    }
    let mut c = cur_of(b"\xff\xff\xff\xff\x10");
    // SAFETY: c décrit ces cinq octets.
    let _ = unsafe { dex_uleb(&mut c) };
    assert_eq!(c.err, 1);
    c = cur_of(b"\x80\x80\x80\x80\x80\x01");
    // SAFETY: c décrit ces six octets.
    let _ = unsafe { dex_uleb(&mut c) };
    assert_eq!(c.err, 1);
    c = cur_of(b"\x80\x80");
    // SAFETY: c décrit ces deux octets.
    let _ = unsafe { dex_uleb(&mut c) };
    assert_eq!(c.err, 1);
    c = cur_of(b"");
    // SAFETY: tampon vide, aucun octet lu au-delà.
    let _ = unsafe { dex_uleb(&mut c) };
    assert_eq!(c.err, 1);

    let cases_s: &[(&[u8], i32)] = &[
        (b"\x00", 0),
        (b"\x7f", -1),
        (b"\x3f", 63),
        (b"\x40", -64),
        (b"\xc0\x00", 64),
        (b"\xbf\x7f", -65),
        (b"\xff\xff\xff\xff\x07", i32::MAX),
        (b"\x80\x80\x80\x80\x78", i32::MIN),
    ];
    for (bytes, want) in cases_s {
        let mut c = cur_of(bytes);
        // SAFETY: c décrit bytes.
        let v = unsafe { dex_sleb(&mut c) };
        assert_eq!(v, *want);
        assert_eq!(c.err, 0);
    }
    c = cur_of(b"\x80\x80\x80\x80\x80\x00");
    // SAFETY: c décrit ces six octets.
    let _ = unsafe { dex_sleb(&mut c) };
    assert_eq!(c.err, 1);
    c = cur_of(b"\xff\xff");
    // SAFETY: c décrit ces deux octets.
    let _ = unsafe { dex_sleb(&mut c) };
    assert_eq!(c.err, 1);
    c = cur_of(b"");
    // SAFETY: tampon vide.
    let _ = unsafe { dex_sleb(&mut c) };
    assert_eq!(c.err, 1);
}

#[test]
fn ulebp1_and_adler() {
    let mut c = cur_of(b"\x00\x01");
    // SAFETY: c décrit les deux octets.
    assert_eq!(unsafe { dex_ulebp1(&mut c) }, DEX_NO_INDEX);
    // SAFETY: le curseur est avancé d'un octet, un octet reste.
    assert_eq!(unsafe { dex_ulebp1(&mut c) }, 0);
    // SAFETY: plus d'octet, le curseur passe en erreur.
    let _ = unsafe { dex_ulebp1(&mut c) };
    assert_eq!(c.err, 1);

    let big = [0xffu8; 20000];
    // SAFETY: big vit, longueur connue. n = 0 ne lit rien.
    assert_eq!(unsafe { dex_adler32(big.as_ptr(), 0) }, 1);
    // SAFETY: littéral de 9 octets.
    assert_eq!(
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        unsafe { dex_adler32(b"Wikipedia".as_ptr(), 9) },
        0x11e6_0398
    );
    // SAFETY: big vit sur toute sa longueur.
    assert_eq!(
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        unsafe { dex_adler32(big.as_ptr(), 5552) },
        adler_ref(&big[..5552])
    );
    assert_eq!(
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        unsafe { dex_adler32(big.as_ptr(), 5553) },
        adler_ref(&big[..5553])
    );
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_adler32(big.as_ptr(), 20000) }, adler_ref(&big));
    // SAFETY: pointeur nul, aucun octet lu.
    assert_eq!(unsafe { dex_adler32(core::ptr::null(), 4) }, 0);
    // SAFETY: liste nulle.
    assert_eq!(unsafe { dex_list_at(core::ptr::null(), 0) }, DEX_NO_INDEX);
    // SAFETY: dex_u16 et dex_u32 sur pointeur nul.
    assert_eq!(unsafe { dex_u16(core::ptr::null()) }, 0);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_u32(core::ptr::null()) }, 0);
}

#[test]
fn mutf8_vectors() {
    fn conv(buf: &mut [u8], src: &[u8], cap: usize) -> c_int {
        let t = t_text {
            p: buf.as_mut_ptr().cast(),
            cap,
        };
        // SAFETY: src est terminé par NUL, buf a cap octets.
        unsafe { mutf8_to_utf8(src.as_ptr().cast(), t) }
    }
    let mut buf = [0u8; 32];
    assert_eq!(conv(&mut buf, b"abc\0", 32), 3);
    assert_eq!(&buf[..3], b"abc");
    assert_eq!(conv(&mut buf, b"\xc3\xa9\xe2\x82\xac\0", 32), 5);
    assert_eq!(&buf[..5], b"\xc3\xa9\xe2\x82\xac");
    assert_eq!(conv(&mut buf, b"\xed\xa0\xbd\xed\xb8\x80\0", 32), 4);
    assert_eq!(&buf[..4], b"\xf0\x9f\x98\x80");
    assert_eq!(conv(&mut buf, b"\xed\xa0\xbd\0", 32), E_INVAL);
    assert_eq!(conv(&mut buf, b"\xed\xb8\x80\0", 32), E_INVAL);
    assert_eq!(conv(&mut buf, b"a\xc0\x80\0", 32), E_NOTSUP);
    assert_eq!(conv(&mut buf, b"\xf0\x9f\x98\x80\0", 32), E_INVAL);
    assert_eq!(conv(&mut buf, b"\x80\0", 32), E_INVAL);
    assert_eq!(conv(&mut buf, b"\xc1\x81\0", 32), E_INVAL);
    assert_eq!(conv(&mut buf, b"\xe2\x82\0", 32), E_INVAL);
    assert_eq!(conv(&mut buf, b"abc\0", 4), 3);
    assert_eq!(conv(&mut buf, b"abc\0", 3), E_OVERFLOW);
    assert_eq!(conv(&mut buf, b"abc\0", 0), E_INVAL);
    assert_eq!(conv(&mut buf, b"\0", 1), 0);

    let chk = |src: &[u8]| -> (c_int, u32) {
        let span = t_span {
            p: src.as_ptr(),
            len: src.len(),
        };
        let mut units = 0u32;
        let mut size = 0u32;
        // SAFETY: src vit, size et units sont exclusifs.
        let rc = unsafe { dex_mutf8_check(span, &mut size, &mut units) };
        (rc, units)
    };
    assert_eq!(chk(b"ab\0"), (E_OK, 2));
    assert_eq!(chk(b"ab").0, E_INVAL);
    assert_eq!(chk(b"\xc0\x80\0"), (E_OK, 1));
    assert_eq!(chk(b"\xc1\xbf\0").0, E_INVAL);
    assert_eq!(chk(b"\xe0\x9f\xbf\0").0, E_INVAL);
    assert_eq!(chk(b"\xe0\xa0\x80\0").0, E_OK);
    assert_eq!(chk(b"\xf0\x90\x80\x80\0").0, E_INVAL);
    assert_eq!(chk(b"\xff\0").0, E_INVAL);
    assert_eq!(chk(b"\x80\0").0, E_INVAL);
    assert_eq!(chk(b"\xe2\x82").0, E_INVAL);
    assert_eq!(chk(b"\xed\xa0\xbd\0").0, E_OK);
    assert_eq!(chk(b"").0, E_INVAL);
}

#[test]
fn header_and_tables() {
    assert!(FIXTURE.len() > 0x70);
    let mut d = blank();
    assert_eq!(open_at(&mut d, FIXTURE), E_OK);
    assert_eq!(patched(FIXTURE, 0, u32::from(b'x'), 1), E_INVAL);
    assert_eq!(patched(FIXTURE, 6, u32::from(b'4'), 1), E_INVAL);
    assert_eq!(patched(FIXTURE, 6, u32::from(b'9'), 1), E_OK);
    assert_eq!(patched(FIXTURE, 5, u32::from(b'4'), 1), E_INVAL);
    assert_eq!(patched(FIXTURE, 7, 1, 1), E_INVAL);
    assert_eq!(patched(FIXTURE, 32, hdr(FIXTURE, 32) + 1, 4), E_INVAL);
    assert_eq!(patched(FIXTURE, 36, 0x78, 4), E_INVAL);
    assert_eq!(patched(FIXTURE, 40, 0x7856_3412, 4), E_NOTSUP);
    assert_eq!(patched(FIXTURE, 40, 0, 4), E_INVAL);
    assert_eq!(patched(FIXTURE, 13, 0xa5, 9), E_INVAL);
    assert_eq!(patched(FIXTURE, 13, 0xa5, 1), E_OK);
    let file = t_span {
        p: FIXTURE.as_ptr(),
        len: FIXTURE.len(),
    };
    // SAFETY: pointeur de dex nul, le fixture n'est pas écrit.
    assert_eq!(unsafe { dex_open(core::ptr::null_mut(), file) }, E_INVAL);
    assert_eq!(patched(FIXTURE, 0, 0, 0), E_OK);
    let mut i = 0usize;
    while i < 6 {
        assert_eq!(patched(FIXTURE, 56 + 8 * i, 0x1000_0000, 4), E_INVAL);
        assert_eq!(patched(FIXTURE, 60 + 8 * i, hdr(FIXTURE, 32), 4), E_INVAL);
        assert_eq!(
            patched(FIXTURE, 60 + 8 * i, hdr(FIXTURE, 60 + 8 * i) + 1, 4),
            E_INVAL
        );
        assert_eq!(patched(FIXTURE, 60 + 8 * i, 0x40, 4), E_INVAL);
        i += 1;
    }
    assert_eq!(patched(FIXTURE, 64, 0x1_0000, 4), E_INVAL);
    assert_eq!(patched(FIXTURE, 104, hdr(FIXTURE, 32), 4), E_INVAL);
    assert_eq!(patched(FIXTURE, 44, hdr(FIXTURE, 32) + 1, 4), E_INVAL);
    assert_eq!(patched(FIXTURE, 52, hdr(FIXTURE, 32), 4), E_INVAL);
    assert_eq!(patched(FIXTURE, 52, hdr(FIXTURE, 52) + 1, 4), E_INVAL);
    assert_eq!(
        patched(
            FIXTURE,
            usize::try_from(hdr(FIXTURE, 52)).unwrap_or(0),
            0x1000_0000,
            4
        ),
        E_INVAL
    );
}

#[test]
fn null_reader() {
    let mut d = blank();
    let empty = t_span {
        p: core::ptr::null(),
        len: 0,
    };
    assert_eq!(open_at(&mut d, &[]), E_INVAL);
    // SAFETY: fichier nul.
    assert_eq!(unsafe { dex_open(&mut d, empty) }, E_INVAL);
    let mut s = blank_str();
    // SAFETY: d a été effacé, s est exclusif.
    assert_eq!(unsafe { dex_string(&d, 0, &mut s) }, E_RANGE);
    // SAFETY: descripteur terminé par NUL, aucune classe.
    assert_eq!(unsafe { dex_find_class(&d, cstr(b"LA;\0")) }, E_NOENT);
    // SAFETY: descripteur nul.
    assert_eq!(unsafe { dex_find_class(&d, core::ptr::null()) }, E_INVAL);
    // SAFETY: sortie nulle.
    assert_eq!(unsafe { dex_string(&d, 0, core::ptr::null_mut()) }, E_INVAL);
}

#[test]
fn strings_protos_members() {
    let d = opened();
    let idx = string_idx(&d, NAME_MUTF8).expect("chaine");
    let mut s = blank_str();
    // SAFETY: d ouvert, s exclusif.
    assert_eq!(unsafe { dex_string(&d, idx, &mut s) }, E_OK);
    assert_eq!(s.utf16, 10);
    assert_eq!(s.size, 17);
    let mut buf = [0u8; 64];
    let t = t_text {
        p: buf.as_mut_ptr().cast(),
        cap: buf.len(),
    };
    // SAFETY: s.p est une chaîne MUTF-8 terminée, buf a 64 octets.
    assert_eq!(unsafe { mutf8_to_utf8(s.p, t) }, 15);
    assert_eq!(&buf[..15], NAME_UTF8);
    let z = string_idx(&d, b"a\xc0\x80b").expect("nul");
    // SAFETY: d ouvert.
    assert_eq!(unsafe { dex_string(&d, z, &mut s) }, E_OK);
    assert_eq!(s.utf16, 3);
    // SAFETY: la chaîne contient U+0000 en MUTF-8.
    assert_eq!(unsafe { mutf8_to_utf8(s.p, t) }, E_NOTSUP);
    // SAFETY: indice égal au nombre de chaînes.
    assert_eq!(unsafe { dex_string(&d, d.n[0], &mut s) }, E_RANGE);

    let mut p = t_dexproto {
        shorty_idx: 0,
        return_idx: 0,
        params: t_dexlist {
            p: core::ptr::null(),
            n: 0,
        },
    };
    // SAFETY: d ouvert, p exclusif.
    assert_eq!(unsafe { dex_proto(&d, 0, &mut p) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_string(&d, p.shorty_idx, &mut s) }, E_OK);
    assert!(str_eq(&s, b"III"));
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_type(&d, p.return_idx, &mut s) }, E_OK);
    assert!(str_eq(&s, b"I"));
    assert_eq!(p.params.n, 2);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    let second = unsafe { dex_list_at(&p.params, 1) };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_type(&d, second, &mut s) }, E_OK);
    assert!(str_eq(&s, b"I"));
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_list_at(&p.params, 2) }, DEX_NO_INDEX);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_proto(&d, 1, &mut p) }, E_OK);
    assert_eq!(p.params.n, 0);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_proto(&d, 2, &mut p) }, E_RANGE);
    assert_eq!(
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        unsafe { dex_type(&d, d.n[DEX_T_TYPE as usize], &mut s) },
        E_RANGE
    );

    let mut f = t_dexfield {
        class_idx: 0,
        type_idx: 0,
        name_idx: 0,
    };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_field(&d, 2, &mut f) }, E_OK);
    assert!(named(&d, f.name_idx, b"COUNT"));
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_type(&d, f.class_idx, &mut s) }, E_OK);
    assert!(str_eq(&s, b"Lvelum/Derived;"));
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_type(&d, f.type_idx, &mut s) }, E_OK);
    assert!(str_eq(&s, b"I"));
    let mut m = t_dexmethod {
        class_idx: 0,
        proto_idx: 0,
        name_idx: 0,
    };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_method(&d, 3, &mut m) }, E_OK);
    assert!(named(&d, m.name_idx, b"add"));
    assert_eq!(m.proto_idx, 0);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_field(&d, 10, &mut f) }, E_RANGE);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_method(&d, 5, &mut m) }, E_RANGE);
}

#[test]
fn classes_values_and_code() {
    let d = opened();
    // SAFETY: descripteurs terminés par NUL.
    assert_eq!(unsafe { dex_find_class(&d, cstr(b"Lvelum/Derived;\0")) }, 1);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_find_class(&d, cstr(b"Lvelum/Base;\0")) }, 0);
    assert_eq!(
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        unsafe { dex_find_class(&d, cstr(b"Lvelum/None;\0")) },
        E_NOENT
    );
    let mut c = blank_class();
    let mut s = blank_str();
    // SAFETY: d ouvert.
    assert_eq!(unsafe { dex_class(&d, 1, &mut c) }, E_OK);
    assert_eq!(c.access, ACC_PUBLIC | ACC_FINAL);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_type(&d, c.superclass_idx, &mut s) }, E_OK);
    assert!(str_eq(&s, b"Lvelum/Base;"));
    assert_eq!(c.interfaces.n, 1);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    let iface = unsafe { dex_list_at(&c.interfaces, 0) };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_type(&d, iface, &mut s) }, E_OK);
    assert!(str_eq(&s, b"Ljava/lang/Runnable;"));
    assert!(named(&d, c.source_file_idx, b"Demo.java"));
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_class(&d, 0, &mut c) }, E_OK);
    assert_eq!(c.interfaces.n, 0);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_class(&d, 2, &mut c) }, E_RANGE);

    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_class(&d, 1, &mut c) }, E_OK);
    let mut it = t_dexcdata {
        d: core::ptr::null(),
        pos: 0,
        left: [0; 4],
        kind: 0,
        last: 0,
    };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_open(&d, &c, &mut it) }, E_OK);
    let mut m = t_dexmember {
        kind: 0,
        idx: 0,
        access: 0,
        code_off: 0,
    };
    let mut i = 0usize;
    while i < 12 {
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_OK);
        assert_eq!(m.kind, KIND[i]);
        assert_eq!(m.idx, IDX[i]);
        assert_eq!(m.code_off != 0, m.kind >= DEX_M_DIRECT);
        i += 1;
    }
    assert_eq!(m.access, ACC_PUBLIC);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_NOENT);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_NOENT);
    c.class_data_off = 0;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_open(&d, &c, &mut it) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_NOENT);

    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_class(&d, 1, &mut c) }, E_OK);
    let mut v = t_dexvalue { kind: 0, i: 0 };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 0, &mut v) }, E_OK);
    assert_eq!(v.kind, DEX_V_LONG);
    assert_eq!(v.i, -2);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 1, &mut v) }, E_OK);
    assert_eq!(v.i, 300);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 2, &mut v) }, E_OK);
    assert_eq!(v.i, 1);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 3, &mut v) }, E_NOTSUP);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 4, &mut v) }, E_OK);
    assert_eq!(v.kind, DEX_V_STRING);
    assert!(named(
        &d,
        u32::try_from(v.i).unwrap_or(DEX_NO_INDEX),
        NAME_MUTF8
    ));
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 5, &mut v) }, E_OK);
    assert_eq!(v.kind, DEX_V_NULL);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 6, &mut v) }, E_OK);
    assert_eq!(v.i as u64, 0x4009_21fb_5444_2d18);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 7, &mut v) }, E_OK);
    assert_eq!(v.i as u64, 0x3fc0_0000);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 8, &mut v) }, E_NOENT);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_class(&d, 0, &mut c) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&d, &c, 0, &mut v) }, E_OK);
    assert_eq!(v.i, -2);

    let off = code_of(&d, 1, 3);
    let mut code = blank_code();
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&d, off, &mut code) }, E_OK);
    assert_eq!(code.registers, 3);
    assert_eq!(code.ins, 2);
    assert_eq!(code.outs, 0);
    assert_eq!(code.tries, 3);
    assert_eq!(code.insns_size, 5);
    assert_eq!(u16p(code.insns), 0x0090);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(u16p(unsafe { code.insns.add(8) }), 0x000f);
    assert_eq!(code.off, off);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&d, 0, &mut code) }, E_INVAL);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&d, off + 2, &mut code) }, E_INVAL);
    let end = (d.len as u32) & !3;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&d, end, &mut code) }, E_INVAL);
    let simple = code_of(&d, 0, 1);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&d, simple, &mut code) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_try_find(&d, &code, 0) }, E_NOENT);

    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&d, off, &mut code) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_try_find(&d, &code, 0) }, 0);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_try_find(&d, &code, 1) }, 0);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_try_find(&d, &code, 2) }, 1);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_try_find(&d, &code, 3) }, 2);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_try_find(&d, &code, 4) }, E_NOENT);
    let mut hit = t_dexhit {
        d: core::ptr::null(),
        pos: 0,
        left: 0,
        catch_all: 0,
        insns_size: 0,
    };
    let mut k = t_dexcatch {
        type_idx: 0,
        addr: 0,
    };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_open(&d, &code, 0, &mut hit) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_type(&d, k.type_idx, &mut s) }, E_OK);
    assert!(str_eq(&s, b"Ljava/lang/Exception;"));
    assert_eq!(k.addr, 3);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_OK);
    assert_eq!(k.type_idx, DEX_NO_INDEX);
    assert_eq!(k.addr, 4);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_NOENT);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_open(&d, &code, 1, &mut hit) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_OK);
    assert_ne!(k.type_idx, DEX_NO_INDEX);
    assert_eq!(k.addr, 3);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_NOENT);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_open(&d, &code, 2, &mut hit) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_OK);
    assert_eq!(k.type_idx, DEX_NO_INDEX);
    assert_eq!(k.addr, 4);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_NOENT);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_open(&d, &code, 3, &mut hit) }, E_RANGE);
    code.insns_size = 4;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_open(&d, &code, 2, &mut hit) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_INVAL);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_NOENT);
}

#[test]
fn cross_checks() {
    let d = opened();
    let end = (d.len as u32) & !3;
    // SAFETY: d.p + off est dans le fichier ouvert, quatre octets d'un id.
    let str_off = unsafe { dex_u32(d.p.add(d.off[DEX_T_STRING as usize] as usize)) };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    let list = unsafe { dex_u32(d.p.add(d.off[DEX_T_PROTO as usize] as usize + 8)) };
    let rej = |off: u32, v: u32, width: u32| {
        assert_eq!(patched(FIXTURE, off as usize, v, width), E_INVAL);
    };
    rej(d.off[DEX_T_STRING as usize], d.len as u32, 4);
    rej(str_off + 1, 0xff, 1);
    rej(str_off, u32::from(FIXTURE[str_off as usize]) + 1, 1);
    rej(d.off[DEX_T_TYPE as usize], d.n[DEX_T_STRING as usize], 4);
    rej(d.off[DEX_T_PROTO as usize], d.n[DEX_T_STRING as usize], 4);
    rej(d.off[DEX_T_PROTO as usize] + 4, d.n[DEX_T_TYPE as usize], 4);
    rej(d.off[DEX_T_PROTO as usize] + 8, end, 4);
    rej(d.off[DEX_T_PROTO as usize] + 8, list + 2, 4);
    rej(list + 4, d.n[DEX_T_TYPE as usize], 2);
    rej(list, 0x7fff_ffff, 4);
    let f = d.off[DEX_T_FIELD as usize];
    let m = d.off[DEX_T_METHOD as usize];
    rej(f, d.n[DEX_T_TYPE as usize], 2);
    rej(f + 2, d.n[DEX_T_TYPE as usize], 2);
    rej(f + 4, d.n[DEX_T_STRING as usize], 4);
    rej(m, d.n[DEX_T_TYPE as usize], 2);
    rej(m + 2, d.n[DEX_T_PROTO as usize], 2);
    rej(m + 4, d.n[DEX_T_STRING as usize], 4);
    let c = d.off[DEX_T_CLASS as usize];
    rej(c, d.n[DEX_T_TYPE as usize], 4);
    rej(c + 8, d.n[DEX_T_TYPE as usize], 4);
    assert_eq!(patched(FIXTURE, (c + 8) as usize, DEX_NO_INDEX, 4), E_OK);
    rej(c + 12, end, 4);
    rej(c + 16, d.n[DEX_T_STRING as usize], 4);
    assert_eq!(patched(FIXTURE, (c + 16) as usize, DEX_NO_INDEX, 4), E_OK);
    rej(c + 20, d.len as u32, 4);
    rej(c + 24, d.len as u32, 4);
    rej(c + 28, d.len as u32, 4);
    rej(c + 32, d.n[DEX_T_TYPE as usize], 4);
}

#[test]
fn hostile_bytes() {
    let d = opened();
    let mut buf = FIXTURE.to_vec();
    let off = code_of(&d, 1, 3);
    assert_ne!(off, 0);
    let mut live = blank();
    assert_eq!(open_at(&mut live, &buf), E_OK);
    let at = off as usize;
    buf[at + 15] = 0x7f;
    live.p = buf.as_ptr();
    let mut code = blank_code();
    // SAFETY: live pointe buf, code est exclusif.
    assert_eq!(unsafe { dex_code(&live, off, &mut code) }, E_INVAL);
    buf[at + 15] = 0;
    buf[at + 2] = 4;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&live, off, &mut code) }, E_INVAL);
    buf[at + 2] = 2;
    buf[at + 7] = 0xff;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&live, off, &mut code) }, E_INVAL);
    buf[at + 7] = 0;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&live, off, &mut code) }, E_OK);
    code.tries = 0x1_0000;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_try_find(&live, &code, 0) }, E_INVAL);

    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_code(&live, off, &mut code) }, E_OK);
    buf[code.tries_off as usize + 6] = 0xff;
    buf[code.tries_off as usize + 7] = 0xff;
    let mut hit = t_dexhit {
        d: core::ptr::null(),
        pos: 0,
        left: 0,
        catch_all: 0,
        insns_size: 0,
    };
    let mut k = t_dexcatch {
        type_idx: 0,
        addr: 0,
    };
    assert_eq!(
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        unsafe { dex_handler_open(&live, &code, 0, &mut hit) },
        E_INVAL
    );
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_NOENT);
    buf[code.tries_off as usize + 6] = 1;
    buf[code.tries_off as usize + 7] = 0;
    buf[code.handlers_off as usize + 2] = 0x7f;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_open(&live, &code, 0, &mut hit) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_INVAL);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_handler_next(&mut hit, &mut k) }, E_NOENT);

    let mut c = blank_class();
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_class(&live, 1, &mut c) }, E_OK);
    let base = c.class_data_off as usize;
    buf[base + 4] = 0x7f;
    let mut it = t_dexcdata {
        d: core::ptr::null(),
        pos: 0,
        left: [0; 4],
        kind: 0,
        last: 0,
    };
    let mut m = t_dexmember {
        kind: 0,
        idx: 0,
        access: 0,
        code_off: 0,
    };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_open(&live, &c, &mut it) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_INVAL);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_NOENT);
    buf[base + 4] = 1;
    buf[base + 6] = 9;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_open(&live, &c, &mut it) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_OK);
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_next(&mut it, &mut m) }, E_INVAL);
    buf[base + 6] = 1;
    buf[base] = 0x7f;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_open(&live, &c, &mut it) }, E_OK);
    let mut n = 0u32;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    while n < 1000 && unsafe { dex_cdata_next(&mut it, &mut m) } == E_OK {
        n += 1;
    }
    assert!(n < 127);
    c.class_data_off = live.len as u32;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_cdata_open(&live, &c, &mut it) }, E_INVAL);

    let mut values = FIXTURE.to_vec();
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_class(&d, 1, &mut c) }, E_OK);
    let vo = c.static_values_off as usize;
    values[vo + 1] = 0x1c;
    seal(&mut values);
    let mut vd = blank();
    assert_eq!(open_at(&mut vd, &values), E_OK);
    let mut v = t_dexvalue { kind: 0, i: 0 };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&vd, &c, 1, &mut v) }, E_NOTSUP);
    values[vo + 1] = 0x01;
    vd.p = values.as_ptr();
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&vd, &c, 0, &mut v) }, E_INVAL);
    values[vo + 1] = 0xe4;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&vd, &c, 0, &mut v) }, E_INVAL);
    values[vo + 1] = 0x5f;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&vd, &c, 0, &mut v) }, E_INVAL);
    values[vo] = 0xff;
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    assert_eq!(unsafe { dex_static_value(&vd, &c, 0, &mut v) }, E_INVAL);
}

fn xorshift(state: &mut u32) -> u32 {
    *state ^= state.wrapping_shl(13);
    *state ^= state.wrapping_shr(17);
    *state ^= state.wrapping_shl(5);
    *state
}

fn walk(d: &t_dex) -> u32 {
    let Some(file) = (if d.p.is_null() {
        None
    } else {
        // SAFETY: d ouvert ou issu d'un open réussi : p pointe len octets.
        Some(unsafe { core::slice::from_raw_parts(d.p, d.len) })
    }) else {
        return 0;
    };
    let mut n = 0u32;
    let mut i = 0u32;
    let mut s = blank_str();
    while i < d.n[DEX_T_STRING as usize] {
        // SAFETY: d et s sont valides pour cet appel.
        if unsafe { dex_string(d, i, &mut s) } == E_OK {
            let sz = s.size as usize;
            if !s.p.is_null() && sz < d.len {
                // SAFETY: le NUL de la chaîne est dans le fichier.
                let b0 = unsafe { *s.p.cast::<u8>() };
                // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
                let bz = unsafe { *s.p.cast::<u8>().add(sz) };
                n = n.wrapping_add(u32::from(bz == 0) + u32::from(b0));
            }
        }
        i += 1;
    }
    i = 0;
    while i < d.n[DEX_T_TYPE as usize] {
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        n = n.wrapping_add(u32::from(unsafe { dex_type(d, i, &mut s) } == E_OK));
        i += 1;
    }
    i = 0;
    let mut proto = t_dexproto {
        shorty_idx: 0,
        return_idx: 0,
        params: t_dexlist {
            p: core::ptr::null(),
            n: 0,
        },
    };
    while i < d.n[DEX_T_PROTO as usize] {
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        if unsafe { dex_proto(d, i, &mut proto) } == E_OK {
            let at = proto.params.n.wrapping_sub(1);
            n = n.wrapping_add(u32::from(
                // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
                unsafe { dex_list_at(&proto.params, at) } != DEX_NO_INDEX,
            ));
        }
        i += 1;
    }
    i = 0;
    let mut field = t_dexfield {
        class_idx: 0,
        type_idx: 0,
        name_idx: 0,
    };
    while i < d.n[DEX_T_FIELD as usize] {
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        n = n.wrapping_add(u32::from(unsafe { dex_field(d, i, &mut field) } == E_OK));
        i += 1;
    }
    i = 0;
    let mut method = t_dexmethod {
        class_idx: 0,
        proto_idx: 0,
        name_idx: 0,
    };
    while i < d.n[DEX_T_METHOD as usize] {
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        n = n.wrapping_add(u32::from(unsafe { dex_method(d, i, &mut method) } == E_OK));
        i += 1;
    }
    i = 0;
    while i < d.n[DEX_T_CLASS as usize] {
        n = n.wrapping_add(walk_class(d, file, i));
        i += 1;
    }
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    let _ = unsafe { dex_find_class(d, cstr(b"Lvelum/Derived;\0")) };
    n
}

fn walk_class(d: &t_dex, file: &[u8], idx: u32) -> u32 {
    let mut c = blank_class();
    let mut it = t_dexcdata {
        d: core::ptr::null(),
        pos: 0,
        left: [0; 4],
        kind: 0,
        last: 0,
    };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    if unsafe { dex_class(d, idx, &mut c) } != E_OK
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        || unsafe { dex_cdata_open(d, &c, &mut it) } != E_OK
    {
        return 0;
    }
    let mut n = 0u32;
    let mut m = t_dexmember {
        kind: 0,
        idx: 0,
        access: 0,
        code_off: 0,
    };
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    while unsafe { dex_cdata_next(&mut it, &mut m) } == E_OK {
        n = n.wrapping_add(1 + walk_code(d, file, m.code_off));
    }
    let mut v = t_dexvalue { kind: 0, i: 0 };
    let mut k = 0u32;
    while k < 16 {
        n = n.wrapping_add(u32::from(
            // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
            unsafe { dex_static_value(d, &c, k, &mut v) } == E_OK,
        ));
        k += 1;
    }
    n
}

fn walk_code(d: &t_dex, file: &[u8], off: u32) -> u32 {
    if off == 0 {
        return 0;
    }
    let mut code = blank_code();
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    if unsafe { dex_code(d, off, &mut code) } != E_OK {
        return 0;
    }
    let mut n = 0u32;
    if code.insns_size > 0 && !code.insns.is_null() && !d.p.is_null() {
        // SAFETY: insns et d.p viennent du même fichier si dex_code a réussi.
        let delta = unsafe { code.insns.offset_from(d.p) };
        if delta >= 0 {
            let at = delta as usize;
            if at < file.len() {
                n = n.wrapping_add(u32::from(file[at]));
            }
            let last = at + 2 * code.insns_size as usize - 1;
            if last < file.len() {
                n = n.wrapping_add(u32::from(file[last]));
            }
        }
    }
    // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
    let _ = unsafe { dex_try_find(d, &code, off & 7) };
    let mut i = 0u32;
    let mut hit = t_dexhit {
        d: core::ptr::null(),
        pos: 0,
        left: 0,
        catch_all: 0,
        insns_size: 0,
    };
    let mut k = t_dexcatch {
        type_idx: 0,
        addr: 0,
    };
    while i < code.tries {
        // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
        if unsafe { dex_handler_open(d, &code, i, &mut hit) } == E_OK {
            // SAFETY: pointeurs du cas, nuls ou vivants, dans le tampon ouvert.
            while unsafe { dex_handler_next(&mut hit, &mut k) } == E_OK {
                n = n.wrapping_add(1);
            }
        }
        i += 1;
    }
    n
}

#[test]
fn truncation_and_mutations() {
    let mut len = 0usize;
    while len < FIXTURE.len() {
        let mut p = FIXTURE[..len].to_vec();
        let mut d = blank();
        assert_ne!(open_at(&mut d, &p), E_OK);
        if len >= 0x70 {
            p[32] = len as u8;
            p[33] = (len >> 8) as u8;
            p[34] = (len >> 16) as u8;
            p[35] = (len >> 24) as u8;
            seal(&mut p);
            if open_at(&mut d, &p) == E_OK {
                let _ = walk(&d);
            }
        }
        len += 1;
    }
    let mut seed = 0x2545_f491u32;
    let mut i = 0u32;
    let mut ok = 0u32;
    while i < 20_000 {
        let mut p = FIXTURE.to_vec();
        let mut k = 1 + xorshift(&mut seed) % 3;
        while k > 0 {
            let at = (xorshift(&mut seed) as usize) % p.len();
            p[at] = xorshift(&mut seed) as u8;
            k -= 1;
        }
        seal(&mut p);
        let mut d = blank();
        if open_at(&mut d, &p) == E_OK {
            let _ = walk(&d);
            ok += 1;
        }
        i += 1;
    }
    assert!(ok > 1000);
    assert!(ok < 20_000);
}
