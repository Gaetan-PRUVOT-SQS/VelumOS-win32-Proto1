use core::ffi::c_int;
use core::mem::{align_of, offset_of, size_of};

use super::{
    E_INVAL, E_NOENT, E_NOTSUP, E_OVERFLOW, E_RANGE, arsc_lookup, arsc_open, arsc_string,
    axml_attr, axml_attr_count, axml_attr_find, axml_name, axml_next, axml_open, axml_string,
    axml_text, respool_get, respool_open, t_arsc, t_axml, t_axmlattr, t_reschunk, t_respool,
    t_resquery, t_resvalue, t_span, t_text,
};
use crate::fix;

const AXML_DONE: c_int = 0;
const AXML_START: c_int = 1;
const AXML_END: c_int = 2;
const AXML_TEXT: c_int = 3;
const RES_T_REFERENCE: u32 = 0x01;
const RES_T_STRING: u32 = 0x03;
const RES_T_INT_DEC: u32 = 0x10;
const RES_T_INT_BOOLEAN: u32 = 0x12;
const ATTR_VERSION_CODE: u32 = 0x0101_021b;
const ATTR_VERSION_NAME: u32 = 0x0101_021c;
const ATTR_MIN_SDK: u32 = 0x0101_020c;
const ATTR_TARGET_SDK: u32 = 0x0101_0270;
const ATTR_LABEL: u32 = 0x0101_0001;
const ATTR_ICON: u32 = 0x0101_0002;
const ATTR_NAME: u32 = 0x0101_0003;
const ATTR_DEBUGGABLE: u32 = 0x0101_000f;

fn span_of(buf: &[u8]) -> t_span {
    t_span {
        p: buf.as_ptr(),
        len: buf.len(),
    }
}

fn text_of(buf: &mut [u8]) -> t_text {
    t_text {
        p: buf.as_mut_ptr().cast(),
        cap: buf.len(),
    }
}

fn c_str(buf: &[u8]) -> &[u8] {
    match buf.iter().position(|b| *b == 0) {
        Some(n) => &buf[..n],
        None => buf,
    }
}

fn blank_pool() -> t_respool {
    t_respool {
        p: core::ptr::null(),
        size: 0,
        hsize: 0,
        count: 0,
        strings: 0,
        utf8: 0,
    }
}

fn blank_axml() -> t_axml {
    t_axml {
        file: t_span {
            p: core::ptr::null(),
            len: 0,
        },
        pool: blank_pool(),
        map_off: 0,
        map_count: 0,
        pos: 0,
        depth: 0,
        ev: 0,
        ev_off: 0,
        attr_off: 0,
        attr_size: 0,
        attr_count: 0,
    }
}

fn blank_arsc() -> t_arsc {
    t_arsc {
        file: t_span {
            p: core::ptr::null(),
            len: 1,
        },
        strings: blank_pool(),
        body: 1,
        has_pool: 1,
        packages: 7,
    }
}

fn open_pool(buf: &[u8]) -> (c_int, t_respool) {
    let mut pool = blank_pool();
    // SAFETY: pool est local, buf vit le temps de l'appel.
    let rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        respool_open(&mut pool, span_of(buf))
    };
    (rc, pool)
}

fn get_str(pool: &t_respool, index: u32, cap: usize) -> (c_int, Vec<u8>) {
    let mut buf = vec![0x5a; cap];
    let rc = if cap == 0 {
        let out = t_text {
            p: buf.as_mut_ptr().cast(),
            cap: 0,
        };
        // SAFETY: pool est aligné, le tampon vit, il ne chevauche pas la table.
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            respool_get(pool, index, out)
        }
    } else {
        // SAFETY: pool est aligné, le tampon vit, il ne chevauche pas la table.
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            respool_get(pool, index, text_of(&mut buf))
        }
    };
    (rc, buf)
}

fn open_axml(buf: &[u8]) -> (c_int, t_axml) {
    let mut xml = blank_axml();
    xml.file.p = buf.as_ptr();
    xml.file.len = 9;
    // SAFETY: xml est local, buf vit le temps de l'appel.
    let rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        axml_open(&mut xml, span_of(buf))
    };
    (rc, xml)
}

fn next_name(xml: &mut t_axml) -> (c_int, Vec<u8>) {
    // SAFETY: xml est local et aligné.
    let ev = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        axml_next(xml)
    };
    let mut buf = [0u8; 64];
    if ev == AXML_TEXT {
        // SAFETY: xml et buf sont distincts et vivants.
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_text(xml, text_of(&mut buf))
        };
    } else if ev != AXML_DONE {
        // SAFETY: xml et buf sont distincts et vivants.
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_name(xml, text_of(&mut buf))
        };
    }
    (ev, c_str(&buf).to_vec())
}

fn find_attr(xml: &t_axml, id: u32) -> (c_int, t_axmlattr) {
    let mut attr = t_axmlattr {
        name: 0,
        res_id: 0,
        raw: 0,
        r#type: 0,
        data: 0,
        ns: 0,
    };
    // SAFETY: xml et attr sont alignés, distincts, vivants.
    let rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        axml_attr_find(xml, id, &mut attr)
    };
    (rc, attr)
}

fn open_arsc(buf: &[u8]) -> (c_int, t_arsc) {
    let mut arsc = blank_arsc();
    arsc.file.p = buf.as_ptr();
    // SAFETY: arsc est local, buf vit le temps de l'appel.
    let rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        arsc_open(&mut arsc, span_of(buf))
    };
    (rc, arsc)
}

fn arsc_text(arsc: &t_arsc, id: u32, lang: Option<&str>) -> (c_int, Vec<u8>) {
    let kept;
    let lang_ptr = match lang {
        None => core::ptr::null(),
        Some(s) => {
            kept = std::ffi::CString::new(s).unwrap();
            kept.as_ptr()
        }
    };
    let q = t_resquery {
        res_id: id,
        lang: lang_ptr,
    };
    let mut buf = [0u8; 64];
    // SAFETY: arsc, q et buf sont vivants et distincts. lang est une chaîne C ou nul.
    let rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        arsc_string(arsc, &q, text_of(&mut buf))
    };
    (rc, c_str(&buf).to_vec())
}

#[test]
fn layout_matches_host_gcc() {
    assert_eq!(size_of::<usize>(), 8);
    assert_eq!(size_of::<t_span>(), 16);
    assert_eq!(align_of::<t_span>(), 8);
    assert_eq!(size_of::<t_text>(), 16);
    assert_eq!(align_of::<t_text>(), 8);
    assert_eq!(size_of::<t_reschunk>(), 16);
    assert_eq!(align_of::<t_reschunk>(), 4);
    assert_eq!(offset_of!(t_reschunk, r#type), 4);
    assert_eq!(offset_of!(t_reschunk, size), 12);
    assert_eq!(size_of::<t_respool>(), 32);
    assert_eq!(align_of::<t_respool>(), 8);
    assert_eq!(offset_of!(t_respool, size), 8);
    assert_eq!(offset_of!(t_respool, utf8), 24);
    assert_eq!(size_of::<t_axmlattr>(), 24);
    assert_eq!(align_of::<t_axmlattr>(), 4);
    assert_eq!(offset_of!(t_axmlattr, ns), 20);
    assert_eq!(size_of::<t_axml>(), 88);
    assert_eq!(align_of::<t_axml>(), 8);
    assert_eq!(offset_of!(t_axml, pool), 16);
    assert_eq!(offset_of!(t_axml, map_off), 48);
    assert_eq!(offset_of!(t_axml, attr_count), 80);
    assert_eq!(size_of::<t_resvalue>(), 8);
    assert_eq!(align_of::<t_resvalue>(), 4);
    assert_eq!(size_of::<t_resquery>(), 16);
    assert_eq!(align_of::<t_resquery>(), 8);
    assert_eq!(offset_of!(t_resquery, lang), 8);
    assert_eq!(size_of::<t_arsc>(), 64);
    assert_eq!(align_of::<t_arsc>(), 8);
    assert_eq!(offset_of!(t_arsc, strings), 16);
    assert_eq!(offset_of!(t_arsc, packages), 56);
}

#[test]
fn respool_tables_ascii_utf8_and_utf16() {
    for utf8 in [true, false] {
        let file = fix::pool_texts(utf8);
        let (rc, pool) = open_pool(&file);
        assert_eq!(rc, 0);
        let (n, buf) = get_str(&pool, 0, 400);
        assert_eq!(n, 3);
        assert_eq!(c_str(&buf), b"abc");
        let (n, buf) = get_str(&pool, 1, 400);
        assert_eq!(n, 6);
        assert_eq!(c_str(&buf), b"\xc3\xa9\xf0\x9f\x98\x80");
        let (n, _) = get_str(&pool, 2, 400);
        assert_eq!(n, 300);
        let (n, buf) = get_str(&pool, 3, 400);
        assert_eq!(n, 0);
        assert_eq!(c_str(&buf), b"");
        let (n, _) = get_str(&pool, 4, 400);
        assert_eq!(n, E_RANGE);
        let (n, _) = get_str(&pool, 0, 3);
        assert_eq!(n, E_OVERFLOW);
        let (n, buf) = get_str(&pool, 0, 4);
        assert_eq!(n, 3);
        assert_eq!(&buf[..4], b"abc\0");
        let (n, _) = get_str(&pool, 0, 0);
        assert_eq!(n, E_INVAL);
    }
}

#[test]
fn respool_long_utf16_and_lone_surrogates() {
    let file = fix::pool_long();
    let (rc, pool) = open_pool(&file);
    assert_eq!(rc, 0);
    let (n, buf) = get_str(&pool, 0, 0x8001);
    assert_eq!(n, 0x8000);
    assert!(buf[..0x8000].iter().all(|b| *b == b'y'));
    assert_eq!(buf[0x8000], 0);
    let (n, _) = get_str(&pool, 0, 0x8000);
    assert_eq!(n, E_OVERFLOW);

    let file = fix::pool_lone();
    let (rc, mut pool) = open_pool(&file);
    assert_eq!(rc, 0);
    let before = pool.utf8;
    let out = t_text {
        p: core::ptr::addr_of_mut!(pool.utf8).cast(),
        cap: 1,
    };
    // SAFETY: l'écriture n'a pas lieu, les trois chaînes sont refusées avant le premier octet.
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            respool_get(&pool, 0, out)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            respool_get(&pool, 1, out)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            respool_get(&pool, 2, out)
        },
        E_INVAL
    );
    assert_eq!(pool.utf8, before);
}

#[test]
fn respool_refus_null_and_patched_bytes() {
    let empty = t_span {
        p: core::ptr::null(),
        len: 0,
    };
    // SAFETY: le pointeur de table est nul, la fonction ne le déréférence pas.
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            respool_open(core::ptr::null_mut(), empty)
        },
        E_INVAL
    );

    let cases = [
        ("type", true, 0usize, 2u8, E_INVAL),
        ("entete court", true, 2, 27, E_INVAL),
        ("taille", true, 7, 1, E_INVAL),
        ("compteur", true, 11, 1, E_INVAL),
        ("debut", true, 22, 1, E_INVAL),
        ("decalage", true, 30, 1, E_INVAL),
        ("utf8 invalide", true, 46, 0xff, E_INVAL),
        ("utf8 sur-long", true, 46, 0xc0, E_INVAL),
        ("nul interne", true, 46, 0, E_INVAL),
        ("fin absente", true, 49, b'x', E_INVAL),
        ("longueur u8", true, 45, 0x7f, E_INVAL),
        ("fin absente u16", false, 52, 1, E_INVAL),
        ("haut u16", false, 47, 0xd8, E_INVAL),
        ("bas u16", false, 47, 0xdc, E_INVAL),
        ("longueur u16", false, 45, 0x7f, E_INVAL),
        ("temoin sain", false, 0, 1, 3),
    ];
    for (_name, utf8, off, value, want) in cases {
        let mut file = fix::pool_texts(utf8);
        file[off] = value;
        let (rc, pool) = open_pool(&file);
        let got = if rc == 0 { get_str(&pool, 0, 64).0 } else { rc };
        assert_eq!(got, want, "{_name}");
    }
}

#[test]
fn axml_manifest_utf16_and_utf8() {
    for utf8 in [false, true] {
        let file = fix::manifest(utf8);
        let (rc, mut xml) = open_axml(&file);
        assert_eq!(rc, 0);
        let (ev, name) = next_name(&mut xml);
        assert_eq!(ev, AXML_START);
        assert_eq!(name, b"manifest");
        assert_eq!(
            unsafe {
                /* SAFETY: tampon et structure vivent le temps de l'appel. */
                axml_attr_count(&xml)
            },
            3
        );
        let (rc, attr) = find_attr(&xml, ATTR_VERSION_CODE);
        assert_eq!(rc, 0);
        assert_eq!(attr.r#type, RES_T_INT_DEC);
        assert_eq!(attr.data, 1);
        let (rc, attr) = find_attr(&xml, ATTR_VERSION_NAME);
        assert_eq!(rc, 0);
        assert_eq!(attr.r#type, RES_T_STRING);
        assert_eq!(attr.data, 13);
        let (ev, name) = next_name(&mut xml);
        assert_eq!((ev, name.as_slice()), (AXML_START, b"uses-sdk".as_slice()));
        let (rc, attr) = find_attr(&xml, ATTR_MIN_SDK);
        assert_eq!((rc, attr.r#type, attr.data), (0, RES_T_INT_DEC, 21));
        let (rc, attr) = find_attr(&xml, ATTR_TARGET_SDK);
        assert_eq!((rc, attr.r#type, attr.data), (0, RES_T_INT_DEC, 33));
        let (ev, name) = next_name(&mut xml);
        assert_eq!((ev, name.as_slice()), (AXML_END, b"uses-sdk".as_slice()));
        let (ev, name) = next_name(&mut xml);
        assert_eq!(
            (ev, name.as_slice()),
            (AXML_START, b"application".as_slice())
        );
        let (rc, attr) = find_attr(&xml, ATTR_LABEL);
        assert_eq!(
            (rc, attr.r#type, attr.data),
            (0, RES_T_REFERENCE, 0x7f02_0000)
        );
        let (rc, attr) = find_attr(&xml, ATTR_DEBUGGABLE);
        assert_eq!(
            (rc, attr.r#type, attr.data),
            (0, RES_T_INT_BOOLEAN, 0xffff_ffff)
        );
        let (ev, name) = next_name(&mut xml);
        assert_eq!((ev, name.as_slice()), (AXML_START, b"activity".as_slice()));
        let (rc, attr) = find_attr(&xml, ATTR_NAME);
        assert_eq!((rc, attr.r#type, attr.data), (0, RES_T_STRING, 17));
        let (ev, name) = next_name(&mut xml);
        assert_eq!(ev, AXML_TEXT);
        assert_eq!(name, b"texte \xc3\xa9\xf0\x9f\x98\x80");
        let (ev, name) = next_name(&mut xml);
        assert_eq!((ev, name.as_slice()), (AXML_END, b"activity".as_slice()));
        let (ev, name) = next_name(&mut xml);
        assert_eq!((ev, name.as_slice()), (AXML_END, b"application".as_slice()));
        let (ev, name) = next_name(&mut xml);
        assert_eq!((ev, name.as_slice()), (AXML_END, b"manifest".as_slice()));
        let (ev, name) = next_name(&mut xml);
        assert_eq!(ev, AXML_DONE);
        assert_eq!(name, b"");
        let (ev, name) = next_name(&mut xml);
        assert_eq!(ev, AXML_DONE);
        assert_eq!(name, b"");
    }
}

#[test]
fn axml_attributes_before_and_package() {
    let file = fix::manifest(false);
    let (rc, mut xml) = open_axml(&file);
    assert_eq!(rc, 0);
    let mut buf = [0u8; 64];
    let mut attr = t_axmlattr {
        name: 1,
        res_id: 1,
        raw: 1,
        r#type: 1,
        data: 1,
        ns: 1,
    };
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_attr_count(&xml)
        },
        0
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_attr(&xml, 0, &mut attr)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_name(&xml, text_of(&mut buf))
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_next(&mut xml)
        },
        AXML_START
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_attr(&xml, 2, &mut attr)
        },
        0
    );
    assert_eq!(attr.res_id, 0);
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_string(&xml, attr.name, text_of(&mut buf))
        },
        7
    );
    assert_eq!(c_str(&buf), b"package");
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_string(&xml, attr.raw, text_of(&mut buf))
        },
        17
    );
    assert_eq!(c_str(&buf), b"org.velum.bonjour");
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_attr(&xml, 3, &mut attr)
        },
        E_RANGE
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_attr_find(&xml, ATTR_ICON, &mut attr)
        },
        E_NOENT
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_attr_find(&xml, 0, &mut attr)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_text(&xml, text_of(&mut buf))
        },
        E_INVAL
    );
}

#[test]
fn axml_refus_and_limits() {
    let refused = [
        "bad_end.axml",
        "no_end.axml",
        "zero_chunk.axml",
        "bad_name.axml",
        "bad_end_name.axml",
        "attr_over.axml",
        "attr_small.axml",
        "text_bad.axml",
        "two_pools.axml",
        "two_maps.axml",
        "no_pool.axml",
        "chunk_over.axml",
        "short_node.axml",
    ];
    for name in refused {
        assert_eq!(walk_axml(&fix::bad(name)), E_INVAL, "{name}");
    }
    assert_eq!(walk_axml(&fix::deep(64)), AXML_DONE);
    assert_eq!(walk_axml(&fix::deep(65)), E_RANGE);
    for name in ["attr_name.axml", "attr_raw.axml", "attr_str.axml"] {
        let file = fix::bad(name);
        let (rc, mut xml) = open_axml(&file);
        assert_eq!(rc, 0, "{name}");
        assert_eq!(
            unsafe {
                /* SAFETY: tampon et structure vivent le temps de l'appel. */
                axml_next(&mut xml)
            },
            AXML_START,
            "{name}"
        );
        let mut attr = t_axmlattr {
            name: 0,
            res_id: 0,
            raw: 0,
            r#type: 0,
            data: 0,
            ns: 0,
        };
        assert_eq!(
            unsafe {
                /* SAFETY: tampon et structure vivent le temps de l'appel. */
                axml_attr(&xml, 0, &mut attr)
            },
            E_INVAL,
            "{name}"
        );
    }
    let mut file = fix::manifest(false);
    file[0] = 2;
    let (rc, mut xml) = open_axml(&file);
    assert_eq!(rc, E_INVAL);
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_next(&mut xml)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_open(core::ptr::null_mut(), span_of(&file))
        },
        E_INVAL
    );
    let file = fix::bad("bad_end.axml");
    let (rc, mut xml) = open_axml(&file);
    assert_eq!(rc, 0);
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_next(&mut xml)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_next(&mut xml)
        },
        E_INVAL
    );
}

#[test]
fn arsc_strings_values_and_refus() {
    let file = fix::resources(0, 1, 288);
    let (rc, arsc) = open_arsc(&file);
    assert_eq!(rc, 0);
    assert_eq!(arsc.packages, 1);
    let (n, buf) = arsc_text(&arsc, 0x7f02_0000, None);
    assert_eq!(n, 5);
    assert_eq!(buf, b"Hello");
    let (n, buf) = arsc_text(&arsc, 0x7f02_0000, Some("fr"));
    assert_eq!(n, 10);
    assert_eq!(buf, b"Bonjour \xc3\xa9");
    let (n, buf) = arsc_text(&arsc, 0x7f02_0000, Some("de"));
    assert_eq!(n, 5);
    assert_eq!(buf, b"Hello");
    let (n, buf) = arsc_text(&arsc, 0x7f02_0001, Some("fr"));
    assert_eq!(n, 10);
    assert_eq!(buf, b"Bonjour \xc3\xa9");
    assert_eq!(arsc_text(&arsc, 0x7f02_0002, None).0, E_RANGE);
    assert_eq!(arsc_text(&arsc, 0x7f02_0006, None).0, 5);
    assert_eq!(arsc_text(&arsc, 0x7f02_000e, None).0, E_RANGE);
    assert_eq!(arsc_text(&arsc, 0x7f02_0004, None).0, E_INVAL);
    assert_eq!(arsc_text(&arsc, 0x7f02_0005, None).0, E_NOTSUP);
    assert_eq!(arsc_text(&arsc, 0x7f02_0040, None).0, E_NOENT);
    assert_eq!(arsc_text(&arsc, 0x7f05_0000, None).0, E_NOENT);
    assert_eq!(arsc_text(&arsc, 0x0101_0001, None).0, E_NOENT);

    let mut value = t_resvalue { r#type: 0, data: 0 };
    let fr = std::ffi::CString::new("fr").unwrap();
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            arsc_lookup(&arsc, 0x7f02_0004, fr.as_ptr(), &mut value)
        },
        0
    );
    assert_eq!(value.r#type, RES_T_INT_DEC);
    assert_eq!(value.data, 42);
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            arsc_lookup(&arsc, 0x7f02_0000, fr.as_ptr(), &mut value)
        },
        0
    );
    assert_eq!(value.data, 1);
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            arsc_lookup(&arsc, 0x7f02_0000, core::ptr::null(), &mut value)
        },
        0
    );
    assert_eq!(value.data, 0);
    assert_eq!(
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            arsc_lookup(&arsc, 0x7f02_0000, core::ptr::null(), core::ptr::null_mut())
        },
        E_INVAL
    );

    let refus = [
        (fix::resources(0, 0, 288), E_INVAL, E_INVAL),
        (fix::resources(0, 2, 288), E_INVAL, E_INVAL),
        (fix::resources(0, 1, 1 << 20), E_INVAL, E_INVAL),
        (fix::resources(1, 1, 288), 0, E_NOTSUP),
        (fix::manifest(false), E_INVAL, E_INVAL),
    ];
    for (file, open, want) in refus {
        let (rc, arsc) = open_arsc(&file);
        assert_eq!(rc, open);
        assert_eq!(arsc_text(&arsc, 0x7f02_0000, None).0, want);
    }
}

fn walk_attrs(xml: &t_axml) {
    let mut buf = [0u8; 48];
    let mut attr = t_axmlattr {
        name: 0,
        res_id: 0,
        raw: 0,
        r#type: 0,
        data: 0,
        ns: 0,
    };
    unsafe {
        // SAFETY: tampon et structure vivent le temps de l'appel.
        axml_name(xml, text_of(&mut buf));
        axml_text(xml, text_of(&mut buf));
        let mut i = 0u32;
        while i < axml_attr_count(xml) {
            if axml_attr(xml, i, &mut attr) == 0 && attr.raw != 0xffff_ffff {
                axml_string(xml, attr.raw, text_of(&mut buf));
            }
            i = i.saturating_add(1);
        }
        axml_attr_find(xml, ATTR_NAME, &mut attr);
    }
}

fn walk_axml(buf: &[u8]) -> c_int {
    let mut xml = blank_axml();
    let rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        axml_open(&mut xml, span_of(buf))
    };
    if rc < 0 {
        return rc;
    }
    let mut n = 0i32;
    let mut rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        axml_next(&mut xml)
    };
    while rc > 0 && n < 1_000_000 {
        walk_attrs(&xml);
        n += 1;
        rc = unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            axml_next(&mut xml)
        };
    }
    rc
}

fn walk_pool(buf: &[u8]) -> c_int {
    let mut pool = blank_pool();
    let rc = unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        respool_open(&mut pool, span_of(buf))
    };
    let mut buf_out = [0u8; 64];
    let mut i = 0u32;
    while rc == 0 && i < pool.count && i < 4096 {
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            respool_get(&pool, i, text_of(&mut buf_out))
        };
        i = i.saturating_add(1);
    }
    rc
}

fn walk_arsc(buf: &[u8]) -> c_int {
    let mut arsc = blank_arsc();
    if unsafe {
        /* SAFETY: tampon et structure vivent le temps de l'appel. */
        arsc_open(&mut arsc, span_of(buf))
    } < 0
    {
        return E_INVAL;
    }
    let mut out = [0u8; 64];
    let mut id = 0x7f02_0000u32;
    let fr = std::ffi::CString::new("fr").unwrap();
    while id < 0x7f02_0012 {
        let mut q = t_resquery {
            res_id: id,
            lang: core::ptr::null(),
        };
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            arsc_string(&arsc, &q, text_of(&mut out))
        };
        q.lang = fr.as_ptr();
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            arsc_string(&arsc, &q, text_of(&mut out))
        };
        let mut value = t_resvalue { r#type: 0, data: 0 };
        unsafe {
            /* SAFETY: tampon et structure vivent le temps de l'appel. */
            arsc_lookup(&arsc, id, fr.as_ptr(), &mut value)
        };
        id = id.wrapping_add(1);
    }
    0
}

fn walk(kind: u8, buf: &[u8]) -> c_int {
    match kind {
        0 => walk_pool(buf),
        1 => walk_axml(buf),
        _ => walk_arsc(buf),
    }
}

fn fx_rnd(seed: &mut u32) -> u32 {
    *seed ^= seed.wrapping_shl(13);
    *seed ^= seed.wrapping_shr(17);
    *seed ^= seed.wrapping_shl(5);
    *seed
}

#[test]
fn prefixes_rejected_and_mutations_finish() {
    let samples = [
        (fix::pool_texts(true), 0u8),
        (fix::pool_texts(false), 0),
        (fix::manifest(false), 1),
        (fix::manifest(true), 1),
        (fix::resources(0, 1, 288), 2),
    ];
    for (file, kind) in &samples {
        let mut bad = 0i64;
        for n in 0..file.len() {
            if walk(*kind, &file[..n]) >= 0 {
                bad += 1;
            }
        }
        assert_eq!(bad, 0);
        assert_eq!(walk(*kind, file), 0);
    }
    let seeds = [
        (fix::pool_texts(true), 0u8, 0x9e37_79b9u32),
        (fix::pool_texts(false), 0, 0x1234_5678),
        (fix::manifest(false), 1, 0xdead_beef),
        (fix::manifest(true), 1, 0x0bad_cafe),
        (fix::resources(0, 1, 288), 2, 0x00c0_ffee),
    ];
    for (file, kind, mut seed) in seeds {
        let mut stuck = 0i32;
        let mut i = 0u32;
        while i < 4000 && !file.is_empty() {
            let mut m = file.clone();
            let mut k = 1 + fx_rnd(&mut seed) % 3;
            while k > 0 {
                k -= 1;
                let off = usize::try_from(fx_rnd(&mut seed)).unwrap() % m.len();
                m[off] = u8::try_from(fx_rnd(&mut seed) & 0xff).unwrap();
            }
            if walk(kind, &m) > 0 {
                stuck += 1;
            }
            i += 1;
        }
        assert_eq!(i, 4000);
        assert_eq!(stuck, 0);
    }
}
