use crate::rd::{byte, cur, uleb};
use crate::{
    DEX_T_STRING, DEX_V_BOOLEAN, DEX_V_CHAR, DEX_V_DOUBLE, DEX_V_FLOAT, DEX_V_LONG, DEX_V_NULL,
    DEX_V_STRING, E_INVAL, E_NOENT, E_NOTSUP, E_OK, t_dex, t_dexclass, t_dexcur, t_dexvalue,
};
use core::ffi::c_int;

const G_MAX: [u8; 32] = [
    1, 0, 2, 2, 4, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 8, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0xfe, 0xfe,
    0xff, 0xff,
];
const DEFER: c_int = 1;

fn head(c: &mut t_dexcur, v: &mut t_dexvalue, n: &mut u32) -> c_int {
    let b = byte(c);
    v.kind = b & 0x1f;
    v.i = i64::from(b >> 5);
    let max = u32::from(G_MAX[v.kind as usize]);
    *n = (b >> 5) + 1;
    if c.err != 0 || max == 0 {
        return E_INVAL;
    }
    if max == 0xfe {
        return E_NOTSUP;
    }
    let limit = u32::from(v.kind == DEX_V_BOOLEAN);
    if max == 0xff && (b >> 5) > limit {
        return E_INVAL;
    }
    if max == 0xff {
        *n = 0;
    }
    if *n > max {
        return E_INVAL;
    }
    E_OK
}

fn payload(c: &mut t_dexcur, n: u32) -> u64 {
    let mut v = 0u64;
    let mut i = 0u32;
    while i < n {
        let b = u64::from(byte(c));
        let sh = 8 * i;
        if sh < 64 {
            v |= b << sh;
        }
        i += 1;
    }
    v
}

fn decode(d: &t_dex, v: &mut t_dexvalue, raw: u64, n: u32) -> c_int {
    if v.kind == DEX_V_NULL || v.kind == DEX_V_BOOLEAN {
        return E_OK;
    }
    if v.kind > DEX_V_STRING || (v.kind > DEX_V_DOUBLE && v.kind < DEX_V_STRING) {
        return DEFER;
    }
    v.i = raw as i64;
    if v.kind == DEX_V_FLOAT && (1..=4).contains(&n) {
        v.i = (raw << (8 * (4 - n))) as i64;
    }
    if v.kind == DEX_V_DOUBLE && (1..=8).contains(&n) {
        v.i = (raw << (8 * (8 - n))) as i64;
    }
    if v.kind <= DEX_V_LONG && v.kind != DEX_V_CHAR && (1..8).contains(&n) {
        let bit = 8 * n - 1;
        if (raw >> bit) & 1 == 1 {
            v.i = (raw | (u64::MAX << (8 * n))) as i64;
        }
    }
    if v.kind == DEX_V_STRING && raw >= u64::from(d.n[DEX_T_STRING as usize]) {
        return E_INVAL;
    }
    E_OK
}

fn element(d: &t_dex, c: &mut t_dexcur, v: &mut t_dexvalue) -> c_int {
    let mut n = 0u32;
    let r = head(c, v, &mut n);
    if r != E_OK {
        return r;
    }
    let raw = payload(c, n);
    if c.err != 0 {
        return E_INVAL;
    }
    decode(d, v, raw, n)
}

pub(crate) fn static_value(d: &t_dex, c: &t_dexclass, ordinal: u32, out: &mut t_dexvalue) -> c_int {
    if c.static_values_off == 0 {
        return E_NOENT;
    }
    let mut cur = cur(d, u64::from(c.static_values_off));
    let count = uleb(&mut cur);
    if cur.err != 0 {
        return E_INVAL;
    }
    if ordinal >= count {
        return E_NOENT;
    }
    let mut i = 0u32;
    let mut r = element(d, &mut cur, out);
    while r >= 0 && i < ordinal {
        r = element(d, &mut cur, out);
        i += 1;
    }
    if r > 0 {
        return E_NOTSUP;
    }
    r
}
