use core::ffi::c_int;

use crate::pool;
use crate::rd::chunk;
use crate::rd::{byte_at, bytes, next, rd16_at, rd32_at};
use crate::{
    AXML_DONE, AXML_END, AXML_MAX_DEPTH, AXML_NO_STRING, AXML_SKIP, AXML_START, AXML_TEXT, E_INVAL,
    E_NOENT, E_OK, E_RANGE, RES_STRING_POOL, RES_T_STRING, RES_XML, RES_XML_CDATA, RES_XML_END,
    RES_XML_END_NS, RES_XML_RESMAP, RES_XML_START, RES_XML_START_NS, t_axml, t_axmlattr,
    t_reschunk, t_span, t_text,
};

fn reset(x: &mut t_axml) {
    x.file.p = core::ptr::null();
    x.file.len = 0;
    x.map_off = 0;
    x.map_count = 0;
    x.pos = 0;
    x.depth = 0;
    x.ev = u32::try_from(AXML_DONE).unwrap_or(0);
    x.ev_off = 0;
    x.attr_off = 0;
    x.attr_size = 0;
    x.attr_count = 0;
}

pub(crate) fn open(x: &mut t_axml, file: t_span) -> c_int {
    reset(x);
    let b = match bytes(file) {
        Ok(b) => b,
        Err(e) => return e,
    };
    let c = match chunk(b, 0) {
        Ok(c) => c,
        Err(e) => return e,
    };
    if c.r#type != RES_XML {
        return E_INVAL;
    }
    let limited = t_span {
        p: file.p,
        len: c.size as usize,
    };
    let b = match bytes(limited) {
        Ok(b) => b,
        Err(e) => return e,
    };
    let mut pos = c.hsize;
    let pool_c = match next(b, &mut pos) {
        Ok(Some(c)) => c,
        _ => return E_INVAL,
    };
    if pool_c.r#type != RES_STRING_POOL {
        return E_INVAL;
    }
    let sub = t_span {
        p: file.p.wrapping_add(pool_c.off as usize),
        len: pool_c.size as usize,
    };
    if pool::open(&mut x.pool, sub) < 0 {
        return E_INVAL;
    }
    x.file = limited;
    x.pos = pos;
    E_OK
}

fn set_map(x: &mut t_axml, c: &t_reschunk) -> c_int {
    if x.map_off != 0 {
        return E_INVAL;
    }
    let Some(delta) = c.size.checked_sub(c.hsize) else {
        return E_INVAL;
    };
    let Some(off) = c.off.checked_add(c.hsize) else {
        return E_INVAL;
    };
    x.map_off = off;
    x.map_count = delta / 4;
    AXML_SKIP
}

fn body_ok(c: &t_reschunk, need: u32) -> bool {
    if c.hsize < 16 || c.hsize > c.size {
        return false;
    }
    c.size - c.hsize >= need
}

fn ev_ns(c: &t_reschunk) -> c_int {
    if !body_ok(c, 8) {
        return E_INVAL;
    }
    AXML_SKIP
}

fn ev_start(x: &mut t_axml, b: &[u8], c: &t_reschunk) -> c_int {
    if !body_ok(c, 20) {
        return E_INVAL;
    }
    let base = u64::from(c.off).saturating_add(u64::from(c.hsize));
    let Some(name) = rd32_at(b, base.saturating_add(4)) else {
        return E_INVAL;
    };
    if name >= x.pool.count {
        return E_INVAL;
    }
    let Some(start) = rd16_at(b, base.saturating_add(8)) else {
        return E_INVAL;
    };
    let Some(asz) = rd16_at(b, base.saturating_add(10)) else {
        return E_INVAL;
    };
    let Some(acount) = rd16_at(b, base.saturating_add(12)) else {
        return E_INVAL;
    };
    let span = u64::from(acount)
        .saturating_mul(u64::from(asz))
        .saturating_add(u64::from(start));
    let body = u64::from(c.size) - u64::from(c.hsize);
    if start < 20 || asz < 20 || span > body {
        return E_INVAL;
    }
    if x.depth >= AXML_MAX_DEPTH {
        return E_RANGE;
    }
    let Some(ev_off) = u32::try_from(base).ok() else {
        return E_INVAL;
    };
    x.depth = x.depth.wrapping_add(1);
    x.ev = u32::try_from(AXML_START).unwrap_or(1);
    x.ev_off = ev_off;
    x.attr_off = ev_off.wrapping_add(start);
    x.attr_size = asz;
    x.attr_count = acount;
    AXML_START
}

fn ev_end(x: &mut t_axml, b: &[u8], c: &t_reschunk) -> c_int {
    if !body_ok(c, 8) || x.depth == 0 {
        return E_INVAL;
    }
    let base = u64::from(c.off).saturating_add(u64::from(c.hsize));
    let Some(name) = rd32_at(b, base.saturating_add(4)) else {
        return E_INVAL;
    };
    if name >= x.pool.count {
        return E_INVAL;
    }
    let Some(ev_off) = u32::try_from(base).ok() else {
        return E_INVAL;
    };
    x.depth = x.depth.wrapping_sub(1);
    x.ev = u32::try_from(AXML_END).unwrap_or(2);
    x.ev_off = ev_off;
    x.attr_count = 0;
    AXML_END
}

fn ev_text(x: &mut t_axml, b: &[u8], c: &t_reschunk) -> c_int {
    if !body_ok(c, 12) {
        return E_INVAL;
    }
    let base = u64::from(c.off).saturating_add(u64::from(c.hsize));
    let Some(idx) = rd32_at(b, base) else {
        return E_INVAL;
    };
    if idx >= x.pool.count {
        return E_INVAL;
    }
    let Some(ev_off) = u32::try_from(base).ok() else {
        return E_INVAL;
    };
    x.ev = u32::try_from(AXML_TEXT).unwrap_or(3);
    x.ev_off = ev_off;
    x.attr_count = 0;
    AXML_TEXT
}

fn node(x: &mut t_axml, b: &[u8], c: &t_reschunk) -> c_int {
    match c.r#type {
        RES_XML_RESMAP => set_map(x, c),
        RES_XML_START_NS | RES_XML_END_NS => ev_ns(c),
        RES_XML_START => ev_start(x, b, c),
        RES_XML_END => ev_end(x, b, c),
        RES_XML_CDATA => ev_text(x, b, c),
        RES_STRING_POOL => E_INVAL,
        _ => AXML_SKIP,
    }
}

fn finish(x: &mut t_axml, err: bool) -> c_int {
    x.ev = u32::try_from(AXML_DONE).unwrap_or(0);
    x.attr_count = 0;
    if err || x.depth != 0 {
        E_INVAL
    } else {
        AXML_DONE
    }
}

pub(crate) fn next_ev(x: &mut t_axml) -> c_int {
    if x.file.p.is_null() {
        return E_INVAL;
    }
    let b = match bytes(x.file) {
        Ok(b) => b,
        Err(_) => return E_INVAL,
    };
    let mut step = next(b, &mut x.pos);
    loop {
        let c = match step {
            Ok(Some(c)) => c,
            Ok(None) => return finish(x, false),
            Err(_) => return finish(x, true),
        };
        let code = node(x, b, &c);
        if code < 0 {
            x.pos = c.off;
        }
        if code != AXML_SKIP {
            return code;
        }
        step = next(b, &mut x.pos);
    }
}

pub(crate) fn attr_count(x: &t_axml) -> u32 {
    if x.file.p.is_null() || x.ev != u32::try_from(AXML_START).unwrap_or(1) {
        return 0;
    }
    x.attr_count
}

pub(crate) fn attr(x: &t_axml, i: u32, out: &mut t_axmlattr) -> c_int {
    if x.file.p.is_null() || x.ev != u32::try_from(AXML_START).unwrap_or(1) {
        return E_INVAL;
    }
    if i >= x.attr_count {
        return E_RANGE;
    }
    let b = match bytes(x.file) {
        Ok(b) => b,
        Err(_) => return E_INVAL,
    };
    let at =
        u64::from(x.attr_off).saturating_add(u64::from(i).saturating_mul(u64::from(x.attr_size)));
    if at
        .checked_add(20)
        .map(|end| end > u64::try_from(b.len()).unwrap_or(0))
        .unwrap_or(true)
    {
        return E_INVAL;
    }
    let Some(ns) = rd32_at(b, at) else {
        return E_INVAL;
    };
    let Some(name) = rd32_at(b, at.saturating_add(4)) else {
        return E_INVAL;
    };
    let Some(raw) = rd32_at(b, at.saturating_add(8)) else {
        return E_INVAL;
    };
    let Some(kind) = byte_at(b, at.saturating_add(15)) else {
        return E_INVAL;
    };
    let Some(data) = rd32_at(b, at.saturating_add(16)) else {
        return E_INVAL;
    };
    out.ns = ns;
    out.name = name;
    out.raw = raw;
    out.r#type = u32::from(kind);
    out.data = data;
    out.res_id = 0;
    if name < x.map_count {
        let m = u64::from(x.map_off).saturating_add(u64::from(name).saturating_mul(4));
        let Some(id) = rd32_at(b, m) else {
            return E_INVAL;
        };
        out.res_id = id;
    }
    if name >= x.pool.count
        || (raw != AXML_NO_STRING && raw >= x.pool.count)
        || (out.r#type == RES_T_STRING && data >= x.pool.count)
    {
        return E_INVAL;
    }
    E_OK
}

pub(crate) fn attr_find(x: &t_axml, res_id: u32, out: &mut t_axmlattr) -> c_int {
    let mut i = 0u32;
    let mut r = attr(x, 0, out);
    while r == E_OK && out.res_id != res_id {
        i = match i.checked_add(1) {
            Some(v) => v,
            None => return E_NOENT,
        };
        r = attr(x, i, out);
    }
    if r == E_RANGE { E_NOENT } else { r }
}

fn pool_index(x: &t_axml, at: u64) -> Result<u32, c_int> {
    let b = bytes(x.file).map_err(|_| E_INVAL)?;
    rd32_at(b, at).ok_or(E_INVAL)
}

pub(crate) fn name(x: &t_axml, out: t_text) -> c_int {
    if x.file.p.is_null() || (x.ev != 1 && x.ev != 2) {
        return E_INVAL;
    }
    let idx = match pool_index(x, u64::from(x.ev_off).saturating_add(4)) {
        Ok(v) => v,
        Err(e) => return e,
    };
    let pool = x.pool;
    pool::get_copied(pool, idx, out)
}

pub(crate) fn string(x: &t_axml, index: u32, out: t_text) -> c_int {
    if x.file.p.is_null() {
        return E_INVAL;
    }
    let pool = x.pool;
    pool::get_copied(pool, index, out)
}

pub(crate) fn text(x: &t_axml, out: t_text) -> c_int {
    if x.file.p.is_null() || x.ev != u32::try_from(AXML_TEXT).unwrap_or(3) {
        return E_INVAL;
    }
    let idx = match pool_index(x, u64::from(x.ev_off)) {
        Ok(v) => v,
        Err(e) => return e,
    };
    let pool = x.pool;
    pool::get_copied(pool, idx, out)
}
