const NO: u32 = 0xffff_ffff;
const T_REF: u8 = 0x01;
const T_STR: u8 = 0x03;
const T_DEC: u8 = 0x10;
const T_BOOL: u8 = 0x12;

fn chunk(kind: u16, head: &[u8], body: &[u8]) -> Vec<u8> {
    let hsize = 8 + head.len();
    let size = hsize + body.len();
    let mut out = Vec::with_capacity(size);
    out.extend_from_slice(&kind.to_le_bytes());
    out.extend_from_slice(&u16::try_from(hsize).unwrap().to_le_bytes());
    out.extend_from_slice(&u32::try_from(size).unwrap().to_le_bytes());
    out.extend_from_slice(head);
    out.extend_from_slice(body);
    out
}

fn len8(n: usize) -> Vec<u8> {
    if n < 0x80 {
        vec![u8::try_from(n).unwrap()]
    } else {
        vec![
            0x80 | u8::try_from(n >> 8).unwrap(),
            u8::try_from(n & 0xff).unwrap(),
        ]
    }
}

fn len16(n: usize) -> Vec<u8> {
    if n < 0x8000 {
        u16::try_from(n).unwrap().to_le_bytes().to_vec()
    } else {
        let hi = 0x8000 | u16::try_from(n >> 16).unwrap();
        let lo = u16::try_from(n & 0xffff).unwrap();
        let mut out = hi.to_le_bytes().to_vec();
        out.extend_from_slice(&lo.to_le_bytes());
        out
    }
}

fn enc(text: &str, utf8: bool) -> Vec<u8> {
    let units: Vec<u16> = text.encode_utf16().collect();
    if utf8 {
        let raw = text.as_bytes();
        let mut out = len8(units.len());
        out.extend(len8(raw.len()));
        out.extend_from_slice(raw);
        out.push(0);
        out
    } else {
        let mut out = len16(units.len());
        for unit in units {
            out.extend_from_slice(&unit.to_le_bytes());
        }
        out.extend_from_slice(&[0, 0]);
        out
    }
}

enum Item<'a> {
    Text(&'a str),
    Raw(&'a [u8]),
}

fn pool(items: &[Item<'_>], utf8: bool) -> Vec<u8> {
    let mut data = Vec::new();
    let mut offs = Vec::new();
    for item in items {
        offs.extend_from_slice(&u32::try_from(data.len()).unwrap().to_le_bytes());
        match *item {
            Item::Text(text) => data.extend(enc(text, utf8)),
            Item::Raw(raw) => data.extend_from_slice(raw),
        }
    }
    let pad = (4 - (data.len() % 4)) % 4;
    data.extend(core::iter::repeat_n(0, pad));
    let start = 28 + 4 * u32::try_from(items.len()).unwrap();
    let flags = if utf8 { 0x100u32 } else { 0 };
    let mut head = Vec::new();
    for word in [u32::try_from(items.len()).unwrap(), 0, flags, start, 0] {
        head.extend_from_slice(&word.to_le_bytes());
    }
    let mut body = offs;
    body.extend(data);
    chunk(1, &head, &body)
}

fn names() -> Vec<Item<'static>> {
    [
        "versionCode",
        "versionName",
        "minSdkVersion",
        "targetSdkVersion",
        "label",
        "name",
        "debuggable",
        "android",
        "http://schemas.android.com/apk/res/android",
        "",
        "package",
        "manifest",
        "org.velum.bonjour",
        "1.0",
        "uses-sdk",
        "application",
        "activity",
        ".Main",
        "texte é\u{1F600}",
    ]
    .into_iter()
    .map(Item::Text)
    .collect()
}

fn node(kind: u16, ext: &[u8]) -> Vec<u8> {
    let mut head = Vec::new();
    head.extend_from_slice(&1u32.to_le_bytes());
    head.extend_from_slice(&NO.to_le_bytes());
    chunk(kind, &head, ext)
}

fn attr(name: u32, kind: u8, data: u32, raw: u32) -> Vec<u8> {
    let mut out = Vec::new();
    for word in [NO, name, raw] {
        out.extend_from_slice(&word.to_le_bytes());
    }
    out.extend_from_slice(&8u16.to_le_bytes());
    out.push(0);
    out.push(kind);
    out.extend_from_slice(&data.to_le_bytes());
    out
}

fn start(name: u32, attrs: &[Vec<u8>], count: Option<u16>, size: u16) -> Vec<u8> {
    let count = count.unwrap_or_else(|| u16::try_from(attrs.len()).unwrap());
    let mut ext = Vec::new();
    ext.extend_from_slice(&NO.to_le_bytes());
    ext.extend_from_slice(&name.to_le_bytes());
    for half in [20u16, size, count, 0, 0, 0] {
        ext.extend_from_slice(&half.to_le_bytes());
    }
    for item in attrs {
        ext.extend_from_slice(item);
    }
    node(0x102, &ext)
}

fn end(name: u32) -> Vec<u8> {
    let mut ext = Vec::new();
    ext.extend_from_slice(&NO.to_le_bytes());
    ext.extend_from_slice(&name.to_le_bytes());
    node(0x103, &ext)
}

fn text_node(index: u32) -> Vec<u8> {
    let mut ext = Vec::new();
    ext.extend_from_slice(&index.to_le_bytes());
    ext.extend_from_slice(&8u16.to_le_bytes());
    ext.push(0);
    ext.push(0);
    ext.extend_from_slice(&0u32.to_le_bytes());
    node(0x104, &ext)
}

fn resmap() -> Vec<u8> {
    let attrs = [
        0x0101_021bu32,
        0x0101_021c,
        0x0101_020c,
        0x0101_0270,
        0x0101_0001,
        0x0101_0003,
        0x0101_000f,
    ];
    let mut body = Vec::new();
    for id in attrs {
        body.extend_from_slice(&id.to_le_bytes());
    }
    chunk(0x180, &[], &body)
}

fn xml(parts: &[Vec<u8>], utf8: bool, head: Option<Vec<Vec<u8>>>) -> Vec<u8> {
    let owned = head.unwrap_or_else(|| vec![pool(&names(), utf8), resmap()]);
    let mut body = Vec::new();
    for part in &owned {
        body.extend_from_slice(part);
    }
    for part in parts {
        body.extend_from_slice(part);
    }
    chunk(3, &[], &body)
}

fn ii(a: u32, b: u32) -> Vec<u8> {
    let mut out = Vec::new();
    out.extend_from_slice(&a.to_le_bytes());
    out.extend_from_slice(&b.to_le_bytes());
    out
}

pub(crate) fn manifest(utf8: bool) -> Vec<u8> {
    let top = vec![
        attr(0, T_DEC, 1, NO),
        attr(1, T_STR, 13, 13),
        attr(10, T_STR, 12, 12),
    ];
    let sdk = vec![attr(2, T_DEC, 21, NO), attr(3, T_DEC, 33, NO)];
    let app = vec![attr(4, T_REF, 0x7f02_0000, NO), attr(6, T_BOOL, NO, NO)];
    let act = vec![attr(5, T_STR, 17, 17)];
    let parts = vec![
        node(0x100, &ii(7, 8)),
        start(11, &top, None, 20),
        start(14, &sdk, None, 20),
        end(14),
        chunk(0x1ff, &[0; 8], b"inconnu!"),
        start(15, &app, None, 20),
        start(16, &act, None, 20),
        text_node(18),
        end(16),
        end(15),
        end(11),
        node(0x101, &ii(7, 8)),
    ];
    xml(&parts, utf8, None)
}

pub(crate) fn deep(levels: usize) -> Vec<u8> {
    let mut parts = Vec::new();
    for _ in 0..levels {
        parts.push(start(11, &[], None, 20));
    }
    for _ in 0..levels {
        parts.push(end(11));
    }
    xml(&parts, false, None)
}

pub(crate) fn bad(name: &str) -> Vec<u8> {
    let one = vec![attr(0, T_DEC, 1, NO)];
    match name {
        "bad_end.axml" => xml(&[end(11)], false, None),
        "no_end.axml" => xml(&[start(11, &[], None, 20)], false, None),
        "zero_chunk.axml" => {
            let mut zero = end(11);
            zero[4..8].copy_from_slice(&[0, 0, 0, 0]);
            xml(&[start(11, &[], None, 20), zero], false, None)
        }
        "bad_name.axml" => xml(&[start(999, &[], None, 20), end(11)], false, None),
        "bad_end_name.axml" => xml(&[start(11, &[], None, 20), end(999)], false, None),
        "attr_over.axml" => xml(&[start(11, &one, Some(50), 20), end(11)], false, None),
        "attr_small.axml" => xml(&[start(11, &one, None, 8), end(11)], false, None),
        "attr_name.axml" => xml(
            &[start(11, &[attr(999, T_DEC, 1, NO)], None, 20), end(11)],
            false,
            None,
        ),
        "attr_raw.axml" => xml(
            &[start(11, &[attr(0, T_DEC, 1, 999)], None, 20), end(11)],
            false,
            None,
        ),
        "attr_str.axml" => xml(
            &[start(11, &[attr(0, T_STR, 999, NO)], None, 20), end(11)],
            false,
            None,
        ),
        "text_bad.axml" => xml(
            &[start(11, &[], None, 20), text_node(999), end(11)],
            false,
            None,
        ),
        "two_pools.axml" => xml(
            &[pool(&names(), false), start(11, &[], None, 20), end(11)],
            false,
            None,
        ),
        "two_maps.axml" => xml(&[resmap(), start(11, &[], None, 20), end(11)], false, None),
        "no_pool.axml" => xml(
            &[start(11, &[], None, 20), end(11)],
            false,
            Some(vec![resmap()]),
        ),
        "chunk_over.axml" => {
            let mut over = chunk(0x1ff, &[], b"abcd");
            over[4..8].copy_from_slice(&4096u32.to_le_bytes());
            xml(&[start(11, &[], None, 20), end(11), over], false, None)
        }
        "short_node.axml" => xml(&[chunk(0x102, &[], &[0; 40])], false, None),
        _ => panic!("fixture {name}"),
    }
}

pub(crate) fn pool_texts(utf8: bool) -> Vec<u8> {
    let long = "x".repeat(300);
    pool(
        &[
            Item::Text("abc"),
            Item::Text("é\u{1F600}"),
            Item::Text(&long),
            Item::Text(""),
        ],
        utf8,
    )
}

pub(crate) fn pool_long() -> Vec<u8> {
    let text = "y".repeat(0x8000);
    pool(&[Item::Text(&text)], false)
}

pub(crate) fn pool_lone() -> Vec<u8> {
    let mut high = len16(1);
    high.extend_from_slice(&0xd800u16.to_le_bytes());
    high.extend_from_slice(&[0, 0]);
    let mut low = len16(1);
    low.extend_from_slice(&0xdc00u16.to_le_bytes());
    low.extend_from_slice(&[0, 0]);
    let mut pair = len16(2);
    pair.extend_from_slice(&0xd800u16.to_le_bytes());
    pair.extend_from_slice(&0x0041u16.to_le_bytes());
    pair.extend_from_slice(&[0, 0]);
    pool(
        &[Item::Raw(&high), Item::Raw(&low), Item::Raw(&pair)],
        false,
    )
}

fn config(lang: &[u8; 2]) -> Vec<u8> {
    let mut out = Vec::new();
    out.extend_from_slice(&64u32.to_le_bytes());
    out.extend_from_slice(&0u16.to_le_bytes());
    out.extend_from_slice(&0u16.to_le_bytes());
    out.extend_from_slice(lang);
    out.extend_from_slice(&[0, 0]);
    out.extend(core::iter::repeat_n(0, 52));
    out
}

fn simple(key: u32, kind: u8, data: u32) -> Vec<u8> {
    let mut out = Vec::new();
    out.extend_from_slice(&8u16.to_le_bytes());
    out.extend_from_slice(&0u16.to_le_bytes());
    out.extend_from_slice(&key.to_le_bytes());
    out.extend_from_slice(&8u16.to_le_bytes());
    out.push(0);
    out.push(kind);
    out.extend_from_slice(&data.to_le_bytes());
    out
}

fn bag(key: u32) -> Vec<u8> {
    let mut out = Vec::new();
    out.extend_from_slice(&16u16.to_le_bytes());
    out.extend_from_slice(&1u16.to_le_bytes());
    out.extend_from_slice(&key.to_le_bytes());
    out.extend_from_slice(&0u32.to_le_bytes());
    out.extend_from_slice(&0u32.to_le_bytes());
    out
}

fn rtype(tid: u8, entries: &[Option<Vec<u8>>], lang: &[u8; 2], flags: u8) -> Vec<u8> {
    let mut offs = Vec::new();
    let mut data = Vec::new();
    for item in entries {
        let rel = match item {
            None => NO,
            Some(_) => u32::try_from(data.len()).unwrap(),
        };
        offs.extend_from_slice(&rel.to_le_bytes());
        if let Some(item) = item {
            data.extend_from_slice(item);
        }
    }
    let start_at = 84 + 4 * u32::try_from(entries.len()).unwrap();
    let mut head = vec![tid, flags];
    head.extend_from_slice(&0u16.to_le_bytes());
    head.extend_from_slice(&u32::try_from(entries.len()).unwrap().to_le_bytes());
    head.extend_from_slice(&start_at.to_le_bytes());
    head.extend(config(lang));
    let mut body = offs;
    body.extend(data);
    chunk(0x201, &head, &body)
}

fn spec(tid: u8, count: usize) -> Vec<u8> {
    let mut head = vec![tid, 0];
    head.extend_from_slice(&0u16.to_le_bytes());
    head.extend_from_slice(&u32::try_from(count).unwrap().to_le_bytes());
    chunk(0x202, &head, &vec![0; 4 * count])
}

fn package(parts: &[Vec<u8>], type_off: u32) -> Vec<u8> {
    let tpool = pool(&[Item::Text("id"), Item::Text("string")], true);
    let keys: Vec<String> = (0..16).map(|i| format!("cle{i}")).collect();
    let key_items: Vec<Item<'_>> = keys.iter().map(|s| Item::Text(s)).collect();
    let kpool = pool(&key_items, true);
    let mut name: Vec<u8> = "org.velum.bonjour"
        .encode_utf16()
        .flat_map(u16::to_le_bytes)
        .collect();
    name.resize(256, 0);
    let mut head = Vec::new();
    head.extend_from_slice(&0x7fu32.to_le_bytes());
    head.extend_from_slice(&name);
    let key_at = 288 + u32::try_from(tpool.len()).unwrap();
    for word in [type_off, 0, key_at, 0, 0] {
        head.extend_from_slice(&word.to_le_bytes());
    }
    let mut body = tpool;
    body.extend(kpool);
    for part in parts {
        body.extend_from_slice(part);
    }
    chunk(0x200, &head, &body)
}

fn entries() -> Vec<Option<Vec<u8>>> {
    let refer = 0x7f02_0000;
    let mut out = vec![
        Some(simple(0, T_STR, 0)),
        Some(simple(1, T_REF, refer)),
        Some(simple(2, T_REF, refer + 3)),
        Some(simple(3, T_REF, refer + 2)),
        Some(simple(4, T_DEC, 42)),
        Some(bag(5)),
    ];
    for i in 0..7u32 {
        out.push(Some(simple(6 + i, T_REF, refer + 7 + i)));
    }
    out.push(Some(simple(13, T_REF, refer)));
    out.push(Some(simple(14, T_REF, refer + 6)));
    out
}

fn table(parts: &[Vec<u8>]) -> Vec<u8> {
    let mut body = Vec::new();
    for part in parts {
        body.extend_from_slice(part);
    }
    chunk(2, &1u32.to_le_bytes(), &body)
}

pub(crate) fn resources(flags: u8, with_pool: usize, type_off: u32) -> Vec<u8> {
    let glob = pool(&[Item::Text("Hello"), Item::Text("Bonjour é")], true);
    let base = entries();
    let mut french = vec![Some(simple(0, T_STR, 1))];
    french.extend(core::iter::repeat_with(|| None).take(base.len() - 1));
    let types = vec![
        spec(2, base.len()),
        rtype(2, &base, &[0, 0], flags),
        rtype(2, &french, b"fr", flags),
    ];
    let mut parts = Vec::new();
    for _ in 0..with_pool {
        parts.push(glob.clone());
    }
    parts.push(package(&types, type_off));
    table(&parts)
}
