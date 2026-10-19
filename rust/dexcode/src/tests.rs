use core::ffi::{CStr, c_int};

use crate::decode::blank_insn;
use crate::{
    E_INVAL, E_NOTSUP, E_RANGE, dexcode_array_data, dexcode_decode, dexcode_flags, dexcode_format,
    dexcode_format_len, dexcode_index_kind, dexcode_name, dexcode_rd32, dexcode_switch,
    dexcode_verify, t_darray, t_dcodelimits, t_dinsn, t_dswitch, t_span,
};

const LOOP: &[u16] = &[0x5012, 0x00d8, 0xff00, 0x0039, 0xfffe, 0x000e];
const PACKED: &[u16] = &[0x002b, 6, 0, 0x000e, 0x000e, 0, 0x0100, 2, 0, 0, 3, 0, 4, 0];
const SPARSE: &[u16] = &[
    0x002c, 8, 0, 0x0126, 15, 0, 0x000e, 0x000e, 0x0200, 2, 0xfffb, 0xffff, 10, 0, 6, 0, 7, 0,
    0x0300, 2, 3, 0, 1, 2, 3,
];
const CALLS: &[u16] = &[
    0x0013, 0x7fff, 0x2071, 3, 0x0010, 0x010a, 0x0274, 2, 0, 0x0116, 5, 0x000e,
];
const MASK: [u8; 27] = [
    0, 0, 3, 1, 1, 0, 0, 3, 1, 1, 1, 1, 7, 3, 3, 3, 3, 0, 3, 1, 1, 1, 0, 0, 0, 0, 1,
];
const LENS: [u8; 27] = [
    0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 5,
];

fn lim(regs: u32) -> t_dcodelimits {
    t_dcodelimits {
        registers: regs,
        ins: 0,
        outs: 2,
        strings: 4,
        types: 4,
        fields: 4,
        methods: 4,
        protos: 4,
    }
}

fn units(words: &[u16]) -> Vec<u8> {
    let mut out = Vec::with_capacity(words.len() * 2);
    for word in words {
        out.push((word & 0xff) as u8);
        out.push((word >> 8) as u8);
    }
    out
}

fn span_of(buf: &[u8]) -> t_span {
    let p = if buf.is_empty() {
        core::ptr::NonNull::<u8>::dangling().as_ptr()
    } else {
        buf.as_ptr()
    };
    t_span { p, len: buf.len() }
}

fn format_of(op: u8) -> u8 {
    // SAFETY: dexcode_format n'a pas de précondition.
    unsafe { dexcode_format(op) }
}

fn kind_of(op: u8) -> u8 {
    // SAFETY: dexcode_index_kind n'a pas de précondition.
    unsafe { dexcode_index_kind(op) }
}

fn flags_of(op: u8) -> u16 {
    // SAFETY: dexcode_flags n'a pas de précondition.
    unsafe { dexcode_flags(op) }
}

fn format_len_of(fmt: u8) -> u8 {
    // SAFETY: dexcode_format_len n'a pas de précondition.
    unsafe { dexcode_format_len(fmt) }
}

fn rd32_at(p: *const u8) -> i32 {
    // SAFETY: p est nul, ou bien il pointe 4 octets initialisés.
    unsafe { dexcode_rd32(p) }
}

fn op_name(op: u8) -> &'static str {
    // SAFETY: dexcode_name rend une chaîne statique terminée pour tout u8.
    let p = unsafe { dexcode_name(op) };
    assert!(!p.is_null());
    // SAFETY: p vient de dexcode_name, non nul, terminé par NUL.
    let text = unsafe { CStr::from_ptr(p) };
    text.to_str().expect("nom utf-8")
}

fn decode_buf(buf: &[u8], pc: u32) -> (c_int, t_dinsn) {
    let mut insn = blank_insn();
    // SAFETY: buf vit l'appel. insn est alignée et exclusive.
    let rc = unsafe { dexcode_decode(span_of(buf), pc, &mut insn) };
    (rc, insn)
}

fn decode_words(lo: u64, hi: u32) -> (c_int, t_dinsn) {
    let words = [
        (lo & 0xffff) as u16,
        ((lo >> 16) & 0xffff) as u16,
        ((lo >> 32) & 0xffff) as u16,
        ((lo >> 48) & 0xffff) as u16,
        (hi & 0xffff) as u16,
    ];
    decode_buf(&units(&words), 0)
}

fn verify_buf(buf: &[u8], limits: &t_dcodelimits) -> (c_int, Vec<u8>) {
    let n = buf.len() / 2;
    let mut starts = vec![0x5au8; n.div_ceil(8).max(1)];
    // SAFETY: buf et limits vivent l'appel. starts a la taille (n+7)/8, ou 1 si n vaut 0.
    let rc = unsafe { dexcode_verify(span_of(buf), limits, starts.as_mut_ptr()) };
    (rc, starts)
}

fn insn_bad(insn: &t_dinsn, nreg: u32) -> bool {
    let m = MASK.get(usize::from(insn.fmt)).copied().unwrap_or(0xff);
    let mut bad = (m & 1 != 0 && insn.a >= nreg)
        || (m & 2 != 0 && insn.b >= nreg)
        || (m & 4 != 0 && insn.c >= nreg);
    if insn.fmt == 22 || insn.fmt == 24 {
        let mut i = 0u8;
        while i < insn.argc {
            let over = insn
                .args
                .get(usize::from(i))
                .is_none_or(|r| u32::from(*r) >= nreg);
            bad |= over;
            i += 1;
        }
    }
    if (insn.fmt == 23 || insn.fmt == 25) && insn.argc > 0 {
        bad |= u64::from(insn.c) + u64::from(insn.argc) > u64::from(nreg);
    }
    bad
}

fn ix(v: u32) -> usize {
    usize::try_from(v).expect("index")
}

fn property(buf: &[u8], limits: &t_dcodelimits, starts: &[u8]) -> bool {
    let n = buf.len() / 2;
    let mut pc = 0u32;
    while ix(pc) < n {
        let on = starts
            .get(ix(pc >> 3))
            .is_some_and(|b| (b >> (pc & 7)) & 1 == 1);
        if on {
            let (rc, insn) = decode_buf(buf, pc);
            if rc < 0 || ix(pc) + usize::from(insn.len) > n || insn_bad(&insn, limits.registers) {
                return false;
            }
        }
        pc += 1;
    }
    true
}

fn flag_num(name: &str) -> u16 {
    match name {
        "0" | "F_END" => 0,
        "F_N" => 0x002,
        "F_GO" => 0x001,
        "F_IF" => 0x003,
        "F_SW" => 0x006,
        "F_FILL" => 0x00a,
        "F_INV" => 0x102,
        "F_NS" => 0x202,
        "F_RETW" => 0x020,
        "W_D" => 0x012,
        "W_A" => 0x022,
        "W_B" => 0x042,
        "W_DA" => 0x032,
        "W_DB" => 0x052,
        "W_BC" => 0x0c2,
        "W_DBC" => 0x0d2,
        "W_DAB" => 0x072,
        _ => panic!("drapeau {name}"),
    }
}

fn fmt_num(name: &str) -> u8 {
    match name {
        "DF_NONE" => 0,
        "DF_10X" => 1,
        "DF_12X" => 2,
        "DF_11N" => 3,
        "DF_11X" => 4,
        "DF_10T" => 5,
        "DF_20T" => 6,
        "DF_22X" => 7,
        "DF_21T" => 8,
        "DF_21S" => 9,
        "DF_21H" => 10,
        "DF_21C" => 11,
        "DF_23X" => 12,
        "DF_22B" => 13,
        "DF_22T" => 14,
        "DF_22S" => 15,
        "DF_22C" => 16,
        "DF_30T" => 17,
        "DF_32X" => 18,
        "DF_31I" => 19,
        "DF_31T" => 20,
        "DF_31C" => 21,
        "DF_35C" => 22,
        "DF_3RC" => 23,
        "DF_45CC" => 24,
        "DF_4RCC" => 25,
        "DF_51L" => 26,
        _ => panic!("format {name}"),
    }
}

fn kind_num(name: &str) -> u8 {
    match name {
        "DK_NONE" => 0,
        "DK_STRING" => 1,
        "DK_TYPE" => 2,
        "DK_FIELD" => 3,
        "DK_METHOD" => 4,
        "DK_PROTO" => 5,
        "DK_CALLSITE" => 6,
        "DK_HANDLE" => 7,
        _ => panic!("genre {name}"),
    }
}

fn parse_u16(tok: &str) -> u16 {
    let tok = tok.trim().trim_end_matches(',');
    if let Some(hex) = tok.strip_prefix("0x") {
        u16::from_str_radix(hex, 16).expect("hex")
    } else {
        tok.parse().expect("décimal")
    }
}

fn refusal_rows() -> Vec<(u32, c_int, Vec<u8>)> {
    let text = include_str!("../../../tests/host/d04/test_refus.c");
    let start = text.find("g_rows[51][17] = {").expect("table");
    let body = &text[start..];
    let end = body.find("};").expect("fin");
    let mut rows = Vec::new();
    for chunk in body[..end].split('{').skip(1) {
        let inside = chunk.split('}').next().expect("accolade");
        let nums: Vec<u16> = inside
            .split(',')
            .filter_map(|tok| {
                let tok = tok.trim();
                if tok.is_empty() {
                    None
                } else {
                    Some(parse_u16(tok))
                }
            })
            .collect();
        if nums.len() < 3 {
            continue;
        }
        let n = usize::from(nums[0]);
        let regs = u32::from(nums[1]);
        let expect = match nums[2] {
            0 => 0,
            1 => E_INVAL,
            2 => E_RANGE,
            3 => E_NOTSUP,
            other => panic!("code {other}"),
        };
        let mut code: Vec<u16> = nums[3..].to_vec();
        if code.len() < n {
            code.resize(n, 0);
        } else {
            code.truncate(n);
        }
        rows.push((regs, expect, units(&code)));
    }
    rows
}

struct Rng(u32);

impl Rng {
    fn next(&mut self) -> u32 {
        self.0 ^= self.0 << 13;
        self.0 ^= self.0 >> 17;
        self.0 ^= self.0 << 5;
        self.0
    }
}

fn sample(i: u32) -> &'static [u16] {
    match i {
        0 => LOOP,
        1 => PACKED,
        2 => SPARSE,
        _ => CALLS,
    }
}

#[test]
fn layout_matches_gcc() {
    use core::mem::{align_of, offset_of, size_of};
    assert_eq!(size_of::<t_span>(), 16);
    assert_eq!(align_of::<t_span>(), 8);
    assert_eq!(offset_of!(t_span, len), 8);
    assert_eq!(size_of::<t_dinsn>(), 48);
    assert_eq!(align_of::<t_dinsn>(), 8);
    assert_eq!(offset_of!(t_dinsn, a), 4);
    assert_eq!(offset_of!(t_dinsn, lit), 16);
    assert_eq!(offset_of!(t_dinsn, idx), 24);
    assert_eq!(offset_of!(t_dinsn, idx2), 28);
    assert_eq!(offset_of!(t_dinsn, args), 32);
    assert_eq!(size_of::<t_dcodelimits>(), 32);
    assert_eq!(align_of::<t_dcodelimits>(), 4);
    assert_eq!(offset_of!(t_dcodelimits, protos), 28);
    assert_eq!(size_of::<t_dswitch>(), 32);
    assert_eq!(align_of::<t_dswitch>(), 8);
    assert_eq!(offset_of!(t_dswitch, count), 4);
    assert_eq!(offset_of!(t_dswitch, first_key), 8);
    assert_eq!(offset_of!(t_dswitch, keys), 16);
    assert_eq!(offset_of!(t_dswitch, targets), 24);
    assert_eq!(size_of::<t_darray>(), 16);
    assert_eq!(align_of::<t_darray>(), 8);
    assert_eq!(offset_of!(t_darray, data), 8);
}

#[test]
fn table_matches_c_source() {
    let text = include_str!("../../../lib/dexcode/dexcode_tab.c");
    let mut seen = 0u32;
    for line in text.lines() {
        let Some(rest) = line.trim().strip_prefix("{\"") else {
            continue;
        };
        let Some((name, rest)) = rest.split_once("\", ") else {
            continue;
        };
        let rest = rest.trim_end_matches(',').trim_end_matches('}');
        let mut parts = rest.split(", ");
        let fmt = parts.next().expect("fmt");
        let kind = parts.next().expect("kind");
        let flags = parts.next().expect("flags");
        let op = u8::try_from(seen).expect("opcode");
        assert_eq!(op_name(op), name, "nom {op:#04x}");
        assert_eq!(format_of(op), fmt_num(fmt), "format {name}");
        assert_eq!(kind_of(op), kind_num(kind), "genre {name}");
        assert_eq!(flags_of(op), flag_num(flags), "drapeaux {name}");
        seen += 1;
    }
    assert_eq!(seen, 256);
}

#[test]
fn table_lengths_names_and_ranges() {
    let mut unused = 0i32;
    let mut bad = 0i32;
    let mut names = std::collections::BTreeSet::new();
    for op in 0u16..256 {
        let op = op as u8;
        let fmt = format_of(op);
        let len = format_len_of(fmt);
        if fmt == 0 {
            unused += 1;
        } else if !(1..=5).contains(&len) {
            bad += 1;
        }
        assert!(names.insert(op_name(op)));
    }
    assert_eq!(unused, 32);
    assert_eq!(bad, 0);
    assert_eq!(format_len_of(200), 0);
    for (fmt, len) in LENS.iter().copied().enumerate() {
        assert_eq!(format_len_of(u8::try_from(fmt).expect("fmt")), len);
    }
    assert_eq!(op_name(0x00), "nop");
    assert_eq!(op_name(0x18), "const-wide");
    assert_eq!(op_name(0x90), "add-int");
    assert_eq!(op_name(0xaf), "rem-double");
    assert_eq!(op_name(0xb0), "add-int/2addr");
    assert_eq!(op_name(0xcf), "rem-double/2addr");
    assert_eq!(op_name(0xd1), "rsub-int");
    assert_eq!(op_name(0xe2), "ushr-int/lit8");
    assert_eq!(op_name(0xfb), "invoke-polymorphic/range");
    assert_eq!(flags_of(0x28), 0x001);
    assert_eq!(flags_of(0x0e), 0);
    assert_eq!(flags_of(0x9b), 0x0d2);
    assert_eq!(flags_of(0xa3), 0x052);
    let ranges = [
        (0x3e, 0x43, 0u8, 0u8),
        (0x73, 0x73, 0, 0),
        (0x79, 0x7a, 0, 0),
        (0xe3, 0xf9, 0, 0),
        (0x2d, 0x31, 12, 0),
        (0x32, 0x37, 14, 0),
        (0x38, 0x3d, 8, 0),
        (0x44, 0x51, 12, 0),
        (0x52, 0x5f, 16, 3),
        (0x60, 0x6d, 11, 3),
        (0x6e, 0x72, 22, 4),
        (0x74, 0x78, 23, 4),
        (0x7b, 0x8f, 2, 0),
        (0x90, 0xaf, 12, 0),
        (0xb0, 0xcf, 2, 0),
        (0xd0, 0xd7, 15, 0),
        (0xd8, 0xe2, 13, 0),
    ];
    for (lo, hi, fmt, kind) in ranges {
        for op in lo..=hi {
            assert_eq!(format_of(op), fmt, "{op:#04x}");
            assert_eq!(kind_of(op), kind, "{op:#04x}");
        }
    }
    assert_ne!(format_of(0x1a), format_of(0x1b));
    assert_eq!(kind_of(0x1a), 1);
    assert_eq!(kind_of(0x1b), 1);
}

#[test]
fn decode_formats_from_d04() {
    let (rc, insn) = decode_words(0x0000, 0);
    assert_eq!(rc, 0);
    assert_eq!(insn.len, 1);
    assert_eq!(insn.fmt, 1);
    let (_, insn) = decode_words(0xef01, 0);
    assert_eq!(insn.a, 15);
    assert_eq!(insn.b, 14);
    let (_, insn) = decode_words(0x8f12, 0);
    assert_eq!(insn.a, 15);
    assert_eq!(insn.lit, -8);
    let (_, insn) = decode_words(0x7012, 0);
    assert_eq!(insn.lit, 7);
    let (_, insn) = decode_words(0xff0f, 0);
    assert_eq!(insn.a, 255);
    let (_, insn) = decode_words(0x8028, 0);
    assert_eq!(insn.lit, -128);
    let (_, insn) = decode_words(0x7f28, 0);
    assert_eq!(insn.lit, 127);
    let (_, insn) = decode_words(0x8000_0029, 0);
    assert_eq!(insn.lit, -32768);
    assert_eq!(insn.len, 2);
    let (_, insn) = decode_words(0xffff_ff02, 0);
    assert_eq!(insn.a, 255);
    assert_eq!(insn.b, 65535);
    let (_, insn) = decode_words(0x8000_ff38, 0);
    assert_eq!(insn.a, 255);
    assert_eq!(insn.lit, -32768);
    let (_, insn) = decode_words(0x7fff_ff13, 0);
    assert_eq!(insn.lit, 32767);
    let (_, insn) = decode_words(0x8000_ff13, 0);
    assert_eq!(insn.lit, -32768);
    let (_, insn) = decode_words(0x8000_0115, 0);
    assert_eq!(insn.lit, i64::from(i32::MIN));
    let (_, insn) = decode_words(0x7fff_0115, 0);
    assert_eq!(insn.lit, 0x7fff_0000);
    let (_, insn) = decode_words(0x8000_0119, 0);
    assert_eq!(insn.lit, i64::MIN);
    let (_, insn) = decode_words(0x7fff_0119, 0);
    assert_eq!(insn.lit, 0x7fff_0000_0000_0000);
    let (_, insn) = decode_words(0xffff_ff1a, 0);
    assert_eq!(insn.idx, 65535);
    let (_, insn) = decode_words(0xfdfe_ff90, 0);
    assert_eq!((insn.a, insn.b, insn.c), (255, 254, 253));
    let (_, insn) = decode_words(0x80fe_ffd8, 0);
    assert_eq!(insn.b, 254);
    assert_eq!(insn.lit, -128);
    let (_, insn) = decode_words(0x7ffe_ffd8, 0);
    assert_eq!(insn.lit, 127);
    let (_, insn) = decode_words(0xffff_ef32, 0);
    assert_eq!((insn.a, insn.b, insn.lit), (15, 14, -1));
    let (_, insn) = decode_words(0x7fff_efd0, 0);
    assert_eq!(insn.lit, 32767);
    let (_, insn) = decode_words(0x8000_efd0, 0);
    assert_eq!(insn.lit, -32768);
    let (_, insn) = decode_words(0xffff_ef52, 0);
    assert_eq!((insn.a, insn.b, insn.idx), (15, 14, 65535));
    let (_, insn) = decode_words(0x8000_0000_002a, 0);
    assert_eq!(insn.lit, i64::from(i32::MIN));
    assert_eq!(insn.len, 3);
    let (_, insn) = decode_words(0x7fff_ffff_002a, 0);
    assert_eq!(insn.lit, i64::from(i32::MAX));
    let (_, insn) = decode_words(0xfffe_ffff_0003, 0);
    assert_eq!((insn.a, insn.b), (65535, 65534));
    let (_, insn) = decode_words(0x7fff_ffff_ff14, 0);
    assert_eq!(insn.lit, i64::from(i32::MAX));
    let (_, insn) = decode_words(0x8000_0000_ff17, 0);
    assert_eq!(insn.lit, i64::from(i32::MIN));
    let (_, insn) = decode_words(0xffff_ffff_ff2b, 0);
    assert_eq!(insn.a, 255);
    assert_eq!(insn.lit, -1);
    let (_, insn) = decode_words(0xffff_ffff_ff1b, 0);
    assert_eq!(insn.idx, 0xffff_ffff);
}

#[test]
fn decode_calls_wide_and_truncation() {
    let (_, insn) = decode_words(0x4321_ffff_556e, 0);
    assert_eq!(insn.argc, 5);
    assert_eq!(insn.idx, 65535);
    assert_eq!(insn.args, [1, 2, 3, 4, 5]);
    assert_eq!(decode_words(0x4321_ffff_656e, 0).0, E_INVAL);
    let (_, insn) = decode_words(0xffff_1234_ff77, 0);
    assert_eq!(insn.argc, 255);
    assert_eq!(insn.c, 65535);
    assert_eq!(insn.idx, 0x1234);
    let (_, insn) = decode_words(0xbeef_0010_0102_25fa, 0);
    assert_eq!(insn.len, 4);
    assert_eq!(insn.argc, 2);
    assert_eq!(insn.args[1], 1);
    assert_eq!(insn.idx, 0x0102);
    assert_eq!(insn.idx2, 0xbeef);
    let (_, insn) = decode_words(0xcafe_0009_0007_03fb, 0);
    assert_eq!(insn.argc, 3);
    assert_eq!(insn.c, 9);
    assert_eq!(insn.idx2, 0xcafe);
    let (_, insn) = decode_words(0xff18, 0x8000);
    assert_eq!(insn.len, 5);
    assert_eq!(insn.a, 255);
    assert_eq!(insn.lit, i64::MIN);
    let (_, insn) = decode_words(0xffff_ffff_ffff_ff18, 0x7fff);
    assert_eq!(insn.lit, i64::MAX);
    let (_, insn) = decode_words(0x0807_0605_0403_0218, 0x0a09);
    assert_eq!(insn.lit, 0x0a09_0807_0605_0403);
    let mut bad = 0i32;
    for op in 0u16..256 {
        let op = op as u8;
        let fmt = format_of(op);
        if fmt == 0 {
            continue;
        }
        let len = format_len_of(fmt);
        let mut raw = vec![0u8; 2 * usize::from(len)];
        let short = 2 * (usize::from(len) - 1);
        if short == 0 {
            raw[0] = op;
            let (rc, _) = decode_buf(&raw[..0], 0);
            let (rc2, _) = decode_buf(&raw, 0);
            if rc != E_INVAL || rc2 != 0 {
                bad += 1;
            }
            continue;
        }
        raw[0] = op;
        let (rc, _) = decode_buf(&raw[..short], 0);
        let (rc2, _) = decode_buf(&raw, 0);
        if rc != E_INVAL || rc2 != 0 {
            bad += 1;
        }
    }
    assert_eq!(bad, 0);
    assert_eq!(decode_words(0x003e, 0).0, E_INVAL);
    let buf = units(&[0, 0, 0, 0, 0]);
    assert_eq!(decode_buf(&buf, 5).0, E_INVAL);
    let mut insn = blank_insn();
    // SAFETY: le pointeur de sortie nul est refusé avant écriture.
    let rc = unsafe { dexcode_decode(span_of(&buf), 0, core::ptr::null_mut()) };
    assert_eq!(rc, E_INVAL);
    let empty = t_span {
        p: core::ptr::null(),
        len: 10,
    };
    // SAFETY: code nul, insn alignée. Aucun déréférencement du code.
    let rc = unsafe { dexcode_decode(empty, 0, &mut insn) };
    assert_eq!(rc, E_INVAL);
    let raw = [0u8; 16];
    // SAFETY: add(1) reste dans raw. Le pointeur est volontairement désaligné.
    let mis = unsafe { raw.as_ptr().add(1).cast::<t_dinsn>().cast_mut() };
    // SAFETY: mis est non nul mais désaligné. Le décodage refuse avant écriture.
    let rc = unsafe { dexcode_decode(span_of(&buf), 0, mis) };
    assert_eq!(rc, E_INVAL);
}

#[test]
fn verify_samples_payloads_and_prefixes() {
    let limits = lim(3);
    let loop_b = units(LOOP);
    let (rc, st) = verify_buf(&loop_b, &limits);
    assert_eq!(rc, 0);
    assert_eq!(st[0], 0x2b);
    assert!(property(&loop_b, &limits, &st));
    let packed = units(PACKED);
    let (rc, st) = verify_buf(&packed, &limits);
    assert_eq!(rc, 0);
    assert_eq!(st[0], 0x19);
    assert_eq!(st[1], 0);
    let mut sw = t_dswitch {
        sparse: 9,
        count: 9,
        first_key: 9,
        keys: core::ptr::null(),
        targets: core::ptr::null(),
    };
    // SAFETY: packed vit l'appel, sw est alignée.
    let rc = unsafe { dexcode_switch(span_of(&packed), 6, &mut sw) };
    assert_eq!(rc, 0);
    assert_eq!(sw.sparse, 0);
    assert_eq!(sw.count, 2);
    assert!(!sw.targets.is_null());
    // SAFETY: la cible 1 est dans packed, quatre octets après targets.
    let at = unsafe { sw.targets.add(4) };
    assert_eq!(rd32_at(at), 4);
    // SAFETY: pc impair, sw alignée. L'échec n'écrit pas sw.
    let rc = unsafe { dexcode_switch(span_of(&packed), 5, &mut sw) };
    assert_eq!(rc, E_INVAL);
    let mut ar = t_darray {
        width: 0,
        count: 0,
        data: core::ptr::null(),
    };
    // SAFETY: packed n'est pas une charge tableau. ar est alignée.
    let rc = unsafe { dexcode_array_data(span_of(&packed), 6, &mut ar) };
    assert_eq!(rc, E_INVAL);
    let sparse = units(SPARSE);
    let (rc, st) = verify_buf(&sparse, &limits);
    assert_eq!(rc, 0);
    assert_eq!(st[0], 0xc9);
    assert_eq!(st[1] | st[2], 0);
    // SAFETY: sparse vit l'appel, sw est alignée.
    let rc = unsafe { dexcode_switch(span_of(&sparse), 8, &mut sw) };
    assert_eq!(rc, 0);
    assert_eq!(sw.sparse, 1);
    assert_eq!(sw.first_key, -5);
    // SAFETY: la clé 1 est dans sparse, quatre octets après keys.
    let key = unsafe { sw.keys.add(4) };
    // SAFETY: la cible 1 est dans sparse, quatre octets après targets.
    let target = unsafe { sw.targets.add(4) };
    assert_eq!(rd32_at(key), 10);
    assert_eq!(rd32_at(target), 7);
    // SAFETY: sparse contient le tableau à l'unité 18. ar est alignée.
    let rc = unsafe { dexcode_array_data(span_of(&sparse), 18, &mut ar) };
    assert_eq!(rc, 0);
    assert_eq!(ar.width, 2);
    assert_eq!(ar.count, 3);
    // SAFETY: ar.data vise l'octet 8 de la charge, +4 reste dans sparse.
    let byte = unsafe { *ar.data.add(4) };
    assert_eq!(byte, 3);
    // SAFETY: l'unité 18 n'est pas un switch. sw est alignée.
    let rc = unsafe { dexcode_switch(span_of(&sparse), 18, &mut sw) };
    assert_eq!(rc, E_INVAL);
    // SAFETY: l'unité 26 est hors charge. ar est alignée.
    let rc = unsafe { dexcode_array_data(span_of(&sparse), 26, &mut ar) };
    assert_eq!(rc, E_INVAL);
    let calls = units(CALLS);
    let (rc, st) = verify_buf(&calls, &limits);
    assert_eq!(rc, 0);
    assert_eq!(st[0], 0x65);
    assert_eq!(st[1], 0x0a);
    assert_eq!(verify_buf(&calls, &lim(2)).0, E_RANGE);
    for words in [LOOP, PACKED, SPARSE, CALLS] {
        let bytes = units(words);
        for k in 0..words.len() {
            assert_ne!(verify_buf(&bytes[..k * 2], &limits).0, 0);
        }
    }
}

#[test]
fn refusal_table_and_bad_arguments() {
    let rows = refusal_rows();
    assert_eq!(rows.len(), 51);
    assert_eq!(rows[0].2, units(&[0x1012, 0x000f]));
    assert_eq!(rows[0].1, 0);
    assert_eq!(rows[50].2, units(&[0x0038, 0xffff]));
    assert_eq!(
        rows[46].2,
        units(&[0x002b, 4, 0, 0x000e, 0x0100, 2, 0xffff, 0x7fff, 3, 0, 3, 0])
    );
    for (regs, expect, bytes) in &rows {
        assert_eq!(verify_buf(bytes, &lim(*regs)).0, *expect);
    }
    let bytes = units(LOOP);
    let limits = lim(3);
    let mut starts = [0u8; 4];
    // SAFETY: bytes et limits vivent l'appel, starts est assez grand.
    let rc = unsafe { dexcode_verify(span_of(&bytes), &limits, starts.as_mut_ptr()) };
    assert_eq!(rc, 0);
    // SAFETY: lim nul est refusé avant lecture.
    let rc = unsafe { dexcode_verify(span_of(&bytes), core::ptr::null(), starts.as_mut_ptr()) };
    assert_eq!(rc, E_INVAL);
    // SAFETY: starts nul est refusé avant écriture.
    let rc = unsafe { dexcode_verify(span_of(&bytes), &limits, core::ptr::null_mut()) };
    assert_eq!(rc, E_INVAL);
    // SAFETY: longueur impaire, refusée avant lecture du code.
    let rc = unsafe {
        dexcode_verify(
            t_span {
                p: bytes.as_ptr(),
                len: 11,
            },
            &limits,
            starts.as_mut_ptr(),
        )
    };
    assert_eq!(rc, E_INVAL);
    // SAFETY: longueur nulle, refusée avant lecture du code.
    let rc = unsafe {
        dexcode_verify(
            t_span {
                p: bytes.as_ptr(),
                len: 0,
            },
            &limits,
            starts.as_mut_ptr(),
        )
    };
    assert_eq!(rc, E_INVAL);
    // SAFETY: code nul, refusé avant déréférencement.
    let rc = unsafe {
        dexcode_verify(
            t_span {
                p: core::ptr::null(),
                len: 12,
            },
            &limits,
            starts.as_mut_ptr(),
        )
    };
    assert_eq!(rc, E_INVAL);
    let mut over = limits;
    over.ins = 4;
    // SAFETY: bytes et starts vivent l'appel. ins > registers est refusé.
    let rc = unsafe { dexcode_verify(span_of(&bytes), &over, starts.as_mut_ptr()) };
    assert_eq!(rc, E_INVAL);
    assert_eq!(rd32_at(core::ptr::null()), 0);
}

#[test]
fn fixed_seed_mutations_stay_inside_buffer() {
    let limits = lim(3);
    let mut rng = Rng(0x2545_f491);
    let mut accepted = 0u32;
    for _ in 0..12_000 {
        let src = sample(rng.next() % 4);
        let mut bytes = units(src);
        let mut n = src.len() as u32;
        let mut k = 1 + rng.next() % 3;
        while k > 0 {
            let idx = (rng.next() % (2 * n)) as usize;
            bytes[idx] = (rng.next() & 0xff) as u8;
            k -= 1;
        }
        if rng.next().is_multiple_of(8) {
            n = rng.next() % (n + 1);
        }
        let slice = &bytes[..usize::try_from(n).expect("n") * 2];
        let (rc, starts) = verify_buf(slice, &limits);
        if rc == 0 {
            assert!(property(slice, &limits, &starts));
            accepted += 1;
        }
        let mut insn = blank_insn();
        let mut sw = t_dswitch {
            sparse: 0,
            count: 0,
            first_key: 0,
            keys: core::ptr::null(),
            targets: core::ptr::null(),
        };
        let mut ar = t_darray {
            width: 0,
            count: 0,
            data: core::ptr::null(),
        };
        let view = span_of(slice);
        let mut pc = 0u32;
        while pc <= n + 1 {
            // SAFETY: slice vit la boucle. Les sorties sont alignées. Un pc hors tampon est refusé.
            unsafe {
                dexcode_decode(view, pc, &mut insn);
                dexcode_switch(view, pc, &mut sw);
                dexcode_array_data(view, pc, &mut ar);
            }
            pc += 1;
        }
    }
    assert!(accepted > 100);
    assert!(accepted < 12_000);
}
