use core::ffi::{c_char, c_int};

use super::{
    E_INVAL, E_NOENT, E_NOTSUP, E_OVERFLOW, E_RANGE, inflate_raw, t_span, t_zip, t_zipent,
    zip_crc32, zip_entry, zip_extract, zip_find, zip_name_safe, zip_open,
};

fn ref_crc(data: &[u8]) -> u32 {
    let mut crc = 0xffff_ffffu32;
    for &b in data {
        crc ^= u32::from(b);
        let mut k = 0;
        while k < 8 {
            let mask = 0u32.wrapping_sub(crc & 1);
            crc = (crc >> 1) ^ (0xedb8_8320 & mask);
            k += 1;
        }
    }
    !crc
}

fn crc_of(crc: u32, data: &[u8]) -> u32 {
    let p = if data.is_empty() {
        core::ptr::null()
    } else {
        data.as_ptr()
    };
    // SAFETY: data vit le temps de l'appel, ou il est vide et p n'est pas lu.
    unsafe { zip_crc32(crc, p, data.len()) }
}

fn inflate(input: &[u8], out: &mut [u8]) -> i64 {
    let span = t_span {
        p: input.as_ptr(),
        len: input.len(),
    };
    // SAFETY: input et out sont distincts et vivants le temps de l'appel.
    unsafe { inflate_raw(span, out.as_mut_ptr(), out.len()) }
}

fn safe(bytes: &[u8]) -> c_int {
    let p = if bytes.is_empty() {
        core::ptr::null()
    } else {
        bytes.as_ptr().cast::<c_char>()
    };
    // SAFETY: bytes vit, ou len vaut 0 et p est nul.
    unsafe { zip_name_safe(p, bytes.len()) }
}

fn open_bytes(bytes: &[u8]) -> (c_int, t_zip) {
    let mut z = t_zip {
        file: t_span {
            p: core::ptr::null(),
            len: 0,
        },
        cd_off: 1,
        cd_size: 1,
        count: 7,
    };
    let span = t_span {
        p: if bytes.is_empty() {
            core::ptr::null()
        } else {
            bytes.as_ptr()
        },
        len: bytes.len(),
    };
    // SAFETY: z est local et aligné, bytes vit le temps de l'appel.
    let rc = unsafe { zip_open(&mut z, span) };
    (rc, z)
}

fn ent_name(e: &t_zipent) -> Vec<u8> {
    if e.name.is_null() || e.name_len == 0 {
        return Vec::new();
    }
    // SAFETY: le pointeur vient de l'archive encore vivante, sur name_len octets.
    let s = unsafe { std::slice::from_raw_parts(e.name.cast::<u8>(), e.name_len as usize) };
    s.to_vec()
}

fn find(z: &t_zip, name: &str) -> (c_int, t_zipent) {
    let mut e = t_zipent {
        name: core::ptr::null(),
        name_len: 0,
        method: 0,
        crc: 0,
        csize: 0,
        r#usize: 0,
        data_off: 0,
        lfh_off: 0,
    };
    let c = std::ffi::CString::new(name).unwrap();
    // SAFETY: z et e sont valides, c est une chaîne C vivante.
    let rc = unsafe { zip_find(z, c.as_ptr(), &mut e) };
    (rc, e)
}

fn entry(z: &t_zip, index: u32) -> (c_int, t_zipent) {
    let mut e = t_zipent {
        name: core::ptr::null(),
        name_len: 0,
        method: 0,
        crc: 0,
        csize: 0,
        r#usize: 0,
        data_off: 0,
        lfh_off: 0,
    };
    // SAFETY: z et e sont valides et alignés.
    let rc = unsafe { zip_entry(z, index, &mut e) };
    (rc, e)
}

fn extract(z: &t_zip, e: &t_zipent, cap: usize) -> (i64, Vec<u8>) {
    let mut buf = vec![0u8; cap];
    // SAFETY: z, e et buf sont distincts, buf a cap octets.
    let n = unsafe { zip_extract(z, e, buf.as_mut_ptr(), cap) };
    (n, buf)
}

fn le16(v: &mut Vec<u8>, n: u16) {
    v.extend_from_slice(&n.to_le_bytes());
}

fn le32(v: &mut Vec<u8>, n: u32) {
    v.extend_from_slice(&n.to_le_bytes());
}

struct Item<'a> {
    name: &'a [u8],
    method: u16,
    payload: &'a [u8],
    raw: &'a [u8],
}

fn build_zip(items: &[Item<'_>]) -> Vec<u8> {
    let mut locals = Vec::new();
    let mut offs = Vec::new();
    for it in items {
        offs.push(u32::try_from(locals.len()).unwrap());
        write_local(&mut locals, it);
    }
    let cd_off = u32::try_from(locals.len()).unwrap();
    let mut cd = Vec::new();
    for (it, off) in items.iter().zip(offs) {
        write_central(&mut cd, it, off);
    }
    let cd_size = u32::try_from(cd.len()).unwrap();
    let mut out = locals;
    out.extend_from_slice(&cd);
    le32(&mut out, 0x0605_4b50);
    le16(&mut out, 0);
    le16(&mut out, 0);
    le16(&mut out, u16::try_from(items.len()).unwrap());
    le16(&mut out, u16::try_from(items.len()).unwrap());
    le32(&mut out, cd_size);
    le32(&mut out, cd_off);
    le16(&mut out, 0);
    out
}

fn write_local(buf: &mut Vec<u8>, it: &Item<'_>) {
    let crc = ref_crc(it.raw);
    le32(buf, 0x0403_4b50);
    le16(buf, 20);
    le16(buf, 0);
    le16(buf, it.method);
    le16(buf, 0);
    le16(buf, 0);
    le32(buf, crc);
    le32(buf, u32::try_from(it.payload.len()).unwrap());
    le32(buf, u32::try_from(it.raw.len()).unwrap());
    le16(buf, u16::try_from(it.name.len()).unwrap());
    le16(buf, 0);
    buf.extend_from_slice(it.name);
    buf.extend_from_slice(it.payload);
}

fn write_central(buf: &mut Vec<u8>, it: &Item<'_>, off: u32) {
    let crc = ref_crc(it.raw);
    le32(buf, 0x0201_4b50);
    le16(buf, 20);
    le16(buf, 20);
    le16(buf, 0);
    le16(buf, it.method);
    le16(buf, 0);
    le16(buf, 0);
    le32(buf, crc);
    le32(buf, u32::try_from(it.payload.len()).unwrap());
    le32(buf, u32::try_from(it.raw.len()).unwrap());
    le16(buf, u16::try_from(it.name.len()).unwrap());
    le16(buf, 0);
    le16(buf, 0);
    le16(buf, 0);
    le16(buf, 0);
    le32(buf, 0);
    le32(buf, off);
    buf.extend_from_slice(it.name);
}

fn eocd(count: u16, cd_size: u32, cd_off: u32) -> Vec<u8> {
    let mut v = Vec::new();
    le32(&mut v, 0x0605_4b50);
    le16(&mut v, 0);
    le16(&mut v, 0);
    le16(&mut v, count);
    le16(&mut v, count);
    le32(&mut v, cd_size);
    le32(&mut v, cd_off);
    le16(&mut v, 0);
    v
}

#[test]
fn tailles_abi() {
    assert_eq!(size_of::<t_span>(), 16);
    assert_eq!(size_of::<t_zip>(), 32);
    assert_eq!(size_of::<t_zipent>(), 40);
    assert_eq!(align_of::<t_span>(), 8);
    assert_eq!(align_of::<t_zip>(), 8);
    assert_eq!(align_of::<t_zipent>(), 8);
    assert_eq!(std::mem::offset_of!(t_zip, cd_off), 16);
    assert_eq!(std::mem::offset_of!(t_zip, cd_size), 20);
    assert_eq!(std::mem::offset_of!(t_zip, count), 24);
    assert_eq!(std::mem::offset_of!(t_zipent, name_len), 8);
    assert_eq!(std::mem::offset_of!(t_zipent, r#usize), 24);
    assert_eq!(std::mem::offset_of!(t_zipent, lfh_off), 32);
}

#[test]
fn crc32_vecteur_itu_et_chainage() {
    assert_eq!(ref_crc(b"123456789"), 0xcbf4_3926);
    assert_eq!(crc_of(0, b"123456789"), 0xcbf4_3926);
    assert_eq!(crc_of(0, b"hi"), ref_crc(b"hi"));
    assert_eq!(crc_of(0x1234_5678, b""), 0x1234_5678);
    let part = crc_of(0, b"12345");
    assert_eq!(crc_of(part, b"6789"), 0xcbf4_3926);
    // SAFETY: pointeur nul et longueur nulle, aucun octet lu.
    assert_eq!(unsafe { zip_crc32(0, core::ptr::null(), 0) }, 0);
}

#[test]
fn inflate_stored_manuel_rfc() {
    let stored = [0x01, 0x02, 0x00, 0xfd, 0xff, 0x48, 0x69];
    let mut out = [0u8; 2];
    assert_eq!(inflate(&stored, &mut out), 2);
    assert_eq!(out, *b"Hi");

    let mut small = [0u8; 1];
    assert_eq!(inflate(&stored, &mut small), i64::from(E_OVERFLOW));

    let trunc = [0x01, 0x02, 0x00, 0xfd, 0xff, 0x48];
    let mut buf = [0u8; 8];
    assert_eq!(inflate(&trunc, &mut buf), i64::from(E_INVAL));

    let bad = [0x01, 0x02, 0x00, 0x00, 0x00, 0x48, 0x69];
    assert_eq!(inflate(&bad, &mut buf), i64::from(E_INVAL));

    let empty = [0x01, 0x00, 0x00, 0xff, 0xff];
    assert_eq!(inflate(&empty, &mut buf), 0);

    let two = [
        0x00, 0x01, 0x00, 0xfe, 0xff, 0x41, 0x01, 0x01, 0x00, 0xfe, 0xff, 0x42,
    ];
    assert_eq!(inflate(&two, &mut buf), 2);
    assert_eq!(&buf[..2], b"AB");
}

#[test]
fn inflate_fixed_dynamique_et_distance() {
    let mut buf = [0u8; 64];
    assert_eq!(inflate(&[0x73, 0x04, 0x00], &mut buf), 1);
    assert_eq!(buf[0], b'A');
    assert_eq!(inflate(&[0x03, 0x00], &mut buf), 0);
    assert_eq!(inflate(&[0x03], &mut buf), i64::from(E_INVAL));
    assert_eq!(inflate(&[0x07], &mut buf), i64::from(E_INVAL));
    assert_eq!(inflate(&[0x03, 0x02], &mut buf), i64::from(E_INVAL));
    assert_eq!(inflate(&[], &mut []), i64::from(E_INVAL));

    let overlap = [0x4b, 0x4c, 0x04, 0x03, 0x00];
    assert_eq!(inflate(&overlap, &mut buf), 7);
    assert_eq!(&buf[..7], b"aaaaaaa");

    let back = [0x4b, 0x04, 0x02, 0x00];
    assert_eq!(inflate(&back, &mut buf), 4);
    assert_eq!(&buf[..4], b"aaaa");

    let hello = [
        0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0xd7, 0x51, 0x08, 0x4b, 0xcd, 0x29, 0xcd, 0xf5, 0x0f, 0x56,
        0x48, 0x49, 0x4d, 0xcb, 0x49, 0x2c, 0x49, 0x55, 0x48, 0xa9, 0xcc, 0x4b, 0xcc, 0xcd, 0x4c,
        0x56, 0x48, 0xcc, 0x4b, 0x51, 0x48, 0xcb, 0xac, 0x48, 0x4d, 0x51, 0xc8, 0xcd, 0xac, 0x50,
        0x50, 0x54, 0x04, 0x00,
    ];
    let text = b"Hello, VelumOS deflate dynamic and fixed mix !!";
    assert_eq!(
        inflate(&hello, &mut buf),
        i64::try_from(text.len()).unwrap()
    );
    assert_eq!(&buf[..text.len()], text);
}

#[test]
fn noms_politique_du_c() {
    assert_eq!(safe(b""), 0);
    assert_eq!(safe(b".."), 0);
    assert_eq!(safe(b"/abs"), 0);
    assert_eq!(safe(b"a/../b"), 0);
    assert_eq!(safe(b"/a"), 0);
    assert_eq!(safe(b"a/.."), 0);
    assert_eq!(safe(b"a\\b"), 0);
    assert_eq!(safe(b"a\0b"), 0);
    assert_eq!(safe(b"a..b/c..."), 1);
    assert_eq!(safe(b"res/a.xml"), 1);
    // SAFETY: pointeur nul, aucun octet lu.
    assert_eq!(unsafe { zip_name_safe(core::ptr::null(), 4) }, 0);
}

#[test]
fn zip_ouvertures_refusees() {
    let span = t_span {
        p: core::ptr::null(),
        len: 0,
    };
    // SAFETY: z nul, refus sans déréférencement.
    assert_eq!(unsafe { zip_open(core::ptr::null_mut(), span) }, E_INVAL);

    let (rc, z) = open_bytes(&[0; 10]);
    assert_eq!(rc, E_INVAL);
    assert_eq!(z.count, 0);

    let (rc, z) = open_bytes(&eocd(0, 0, 0));
    assert_eq!(rc, 0);
    assert_eq!(z.count, 0);
    assert_eq!(find(&z, "absent").0, E_NOENT);
    assert_eq!(entry(&z, 0).0, E_RANGE);

    let mut shifted = vec![0xabu8];
    shifted.extend(eocd(0, 0, 1));
    let (rc, z) = open_bytes(&shifted);
    assert_eq!(rc, 0);
    assert_eq!(z.count, 0);

    let (rc, z) = open_bytes(&eocd(4097, 0, 0));
    assert_eq!(rc, E_RANGE);
    assert_eq!(z.count, 0);

    let (rc, _) = open_bytes(&eocd(0xffff, 0, 0));
    assert_eq!(rc, E_NOTSUP);

    let mut disk = eocd(0, 0, 0);
    disk[4] = 1;
    assert_eq!(open_bytes(&disk).0, E_NOTSUP);

    assert_eq!(open_bytes(&eocd(0, 5, 0)).0, E_INVAL);
}

#[test]
fn zip_contenu_stored_et_deflate() {
    let hello = [
        0xf3u8, 0x48, 0xcd, 0xc9, 0xc9, 0xd7, 0x51, 0x08, 0x4b, 0xcd, 0x29, 0xcd, 0xf5, 0x0f, 0x56,
        0x48, 0x49, 0x4d, 0xcb, 0x49, 0x2c, 0x49, 0x55, 0x48, 0xa9, 0xcc, 0x4b, 0xcc, 0xcd, 0x4c,
        0x56, 0x48, 0xcc, 0x4b, 0x51, 0x48, 0xcb, 0xac, 0x48, 0x4d, 0x51, 0xc8, 0xcd, 0xac, 0x50,
        0x50, 0x54, 0x04, 0x00,
    ];
    let text = b"Hello, VelumOS deflate dynamic and fixed mix !!";
    let file = build_zip(&[
        Item {
            name: b"a.txt",
            method: 0,
            payload: b"hi",
            raw: b"hi",
        },
        Item {
            name: b"dir/b.bin",
            method: 8,
            payload: &hello,
            raw: text,
        },
        Item {
            name: b"..",
            method: 0,
            payload: b"x",
            raw: b"x",
        },
    ]);
    let (rc, z) = open_bytes(&file);
    assert_eq!(rc, 0);
    assert_eq!(z.count, 3);

    let (rc, e) = entry(&z, 0);
    assert_eq!(rc, 0);
    assert_eq!(ent_name(&e), b"a.txt");
    assert_eq!(e.method, 0);
    let (n, buf) = extract(&z, &e, 2);
    assert_eq!(n, 2);
    assert_eq!(&buf, b"hi");
    assert_eq!(extract(&z, &e, 1).0, i64::from(E_OVERFLOW));

    let (rc, e) = find(&z, "dir/b.bin");
    assert_eq!(rc, 0);
    assert_eq!(e.method, 8);
    assert_eq!(e.r#usize, u32::try_from(text.len()).unwrap());
    let (n, buf) = extract(&z, &e, text.len());
    assert_eq!(n, i64::try_from(text.len()).unwrap());
    assert_eq!(&buf[..text.len()], text);
    assert_eq!(find(&z, "absent").0, E_NOENT);
    assert_eq!(find(&z, "a.tx").0, E_NOENT);
    assert_eq!(entry(&z, 3).0, E_RANGE);

    let (rc, e) = find(&z, "..");
    assert_eq!(rc, 0);
    assert_eq!(safe(&ent_name(&e)), 0);

    let mut bad = file.clone();
    let at = e.data_off as usize;
    bad[at] ^= 1;
    let (rc, zbad) = open_bytes(&bad);
    assert_eq!(rc, 0);
    let (rc, ent) = find(&zbad, "..");
    assert_eq!(rc, 0);
    assert_eq!(extract(&zbad, &ent, 8).0, i64::from(E_INVAL));
}

#[test]
fn zip_doublon_methode_et_chiffre() {
    let dup = build_zip(&[
        Item {
            name: b"a.txt",
            method: 0,
            payload: b"hi",
            raw: b"hi",
        },
        Item {
            name: b"a.txt",
            method: 0,
            payload: b"yo",
            raw: b"yo",
        },
    ]);
    let (rc, z) = open_bytes(&dup);
    assert_eq!(rc, E_INVAL);
    assert_eq!(z.count, 0);

    let mut file = build_zip(&[Item {
        name: b"a.txt",
        method: 0,
        payload: b"hi",
        raw: b"hi",
    }]);
    let at = file.windows(4).position(|w| w == b"PK\x01\x02").unwrap();
    file[at + 8] = 1;
    assert_eq!(open_bytes(&file).0, E_NOTSUP);

    file[at + 8] = 0;
    file[at + 10] = 1;
    assert_eq!(open_bytes(&file).0, E_NOTSUP);
}
