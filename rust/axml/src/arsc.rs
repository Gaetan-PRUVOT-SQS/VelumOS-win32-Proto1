use core::ffi::{c_char, c_int};

use crate::pool;
use crate::rd::{bytes, chunk, next, rd32, rd32_at, span_at};
use crate::{
    ARSC_MAX_REF, E_INVAL, E_NOENT, E_NOTSUP, E_OK, RES_ENTRY_COMPACT, RES_ENTRY_COMPLEX,
    RES_NO_ENTRY, RES_STRING_POOL, RES_T_REFERENCE, RES_T_STRING, RES_TABLE, RES_TABLE_PACKAGE,
    RES_TABLE_TYPE, t_arsc, t_reschunk, t_resquery, t_resvalue, t_span, t_text,
};

struct Pick {
    res_id: u32,
    lang: *const c_char,
    rank: i32,
    val: t_resvalue,
}

fn lang_bytes(lang: *const c_char) -> Option<(u8, u8)> {
    if lang.is_null() {
        return None;
    }
    // SAFETY: lang non nul. Une chaîne C a au moins l'octet de tête.
    let l0 = unsafe { *lang };
    if l0 == 0 {
        return None;
    }
    // SAFETY: lang[0] n'est pas nul, donc lang[1] appartient à la chaîne, NUL compris.
    let l1 = unsafe { *lang.add(1) };
    Some((l0 as u8, l1 as u8))
}

fn cfg_rank(cfg: &[u8], lang: *const c_char) -> i32 {
    if let Some((a, b)) = lang_bytes(lang)
        && cfg.len() > 9
        && cfg[8] == a
        && cfg[9] == b
    {
        return 2;
    }
    let mut i = 4usize;
    while i < cfg.len() {
        if cfg[i] != 0 {
            return 0;
        }
        i = i.saturating_add(1);
    }
    1
}

fn entry_value(t: &[u8], size: u32, pos: u64, v: &mut t_resvalue) -> c_int {
    let Some(mark) = pos.checked_add(8) else {
        return E_INVAL;
    };
    if mark > u64::from(size) {
        return E_INVAL;
    }
    let Some(esize) = crate::rd::rd16_at(t, pos) else {
        return E_INVAL;
    };
    let Some(flags) = crate::rd::rd16_at(t, pos.saturating_add(2)) else {
        return E_INVAL;
    };
    if flags & (RES_ENTRY_COMPLEX | RES_ENTRY_COMPACT) != 0 {
        return E_NOTSUP;
    }
    if esize < 8 {
        return E_INVAL;
    }
    let Some(end) = pos
        .checked_add(u64::from(esize))
        .and_then(|n| n.checked_add(8))
    else {
        return E_INVAL;
    };
    if end > u64::from(size) {
        return E_INVAL;
    }
    let base = pos.saturating_add(u64::from(esize));
    let Some(kind) = crate::rd::byte_at(t, base.saturating_add(3)) else {
        return E_INVAL;
    };
    let Some(data) = rd32_at(t, base.saturating_add(4)) else {
        return E_INVAL;
    };
    v.r#type = u32::from(kind);
    v.data = data;
    E_OK
}

fn type_pick(file: &[u8], c: &t_reschunk, k: &mut Pick) -> c_int {
    let Ok(off) = usize::try_from(c.off) else {
        return E_INVAL;
    };
    let Ok(len) = usize::try_from(c.size) else {
        return E_INVAL;
    };
    if off > file.len() || len > file.len() - off {
        return E_INVAL;
    }
    let t = &file[off..off + len];
    if c.hsize < 32 {
        return E_INVAL;
    }
    let Some(cfg_len) = rd32(t, 20) else {
        return E_INVAL;
    };
    if cfg_len < 12 || cfg_len > c.hsize - 20 {
        return E_INVAL;
    }
    let Some(n) = rd32(t, 12) else {
        return E_INVAL;
    };
    let e = k.res_id & 0xffff;
    let cover = u64::from(c.hsize).saturating_add(u64::from(n).saturating_mul(4));
    if cover > u64::from(c.size) {
        return E_INVAL;
    }
    let cfg_end = usize::try_from(20u64.saturating_add(u64::from(cfg_len))).unwrap_or(usize::MAX);
    if cfg_end > t.len() {
        return E_INVAL;
    }
    let rank = cfg_rank(&t[20..cfg_end], k.lang);
    let tid = t.get(8).copied().unwrap_or(0xff);
    let flags = t.get(9).copied().unwrap_or(0);
    let want = u8::try_from((k.res_id >> 16) & 0xff).unwrap_or(0xff);
    if tid != want || e >= n || rank <= k.rank {
        return E_OK;
    }
    if flags & 3 != 0 {
        return E_NOTSUP;
    }
    let slot = u64::from(c.hsize).saturating_add(u64::from(e).saturating_mul(4));
    let Some(ent) = rd32_at(t, slot) else {
        return E_INVAL;
    };
    if ent == RES_NO_ENTRY {
        return E_OK;
    }
    let Some(start) = rd32(t, 16) else {
        return E_INVAL;
    };
    k.rank = rank;
    let mut val = t_resvalue { r#type: 0, data: 0 };
    let code = entry_value(
        t,
        c.size,
        u64::from(start).saturating_add(u64::from(ent)),
        &mut val,
    );
    if code == E_OK {
        k.val = val;
    }
    code
}

fn pkg_scan(file: &[u8], pkg: &t_reschunk, k: &mut Pick) -> c_int {
    let end = u64::from(pkg.off).saturating_add(u64::from(pkg.size));
    let limit = usize::try_from(end).unwrap_or(file.len()).min(file.len());
    let mut pos = pkg.off.wrapping_add(pkg.hsize);
    let mut step = next(&file[..limit], &mut pos);
    while let Ok(Some(c)) = step {
        if c.r#type == RES_TABLE_TYPE {
            let code = type_pick(file, &c, k);
            if code != E_OK {
                return code;
            }
        }
        step = next(&file[..limit], &mut pos);
    }
    match step {
        Ok(_) => E_OK,
        Err(e) => e,
    }
}

fn top_scan(a: &t_arsc, file: &[u8], k: &mut Pick) -> c_int {
    let mut pos = a.body;
    let mut step = next(file, &mut pos);
    while let Ok(Some(c)) = step {
        if c.r#type == RES_TABLE_PACKAGE {
            let Some(id) = rd32_at(file, u64::from(c.off).saturating_add(8)) else {
                return E_INVAL;
            };
            if id == k.res_id >> 24 {
                let code = pkg_scan(file, &c, k);
                if code != E_OK {
                    return code;
                }
            }
        }
        step = next(file, &mut pos);
    }
    match step {
        Ok(_) => E_OK,
        Err(e) => e,
    }
}

pub(crate) fn lookup(a: &t_arsc, res_id: u32, lang: *const c_char) -> Result<t_resvalue, c_int> {
    if a.file.p.is_null() {
        return Err(E_INVAL);
    }
    let file = bytes(a.file).map_err(|_| E_INVAL)?;
    let mut k = Pick {
        res_id,
        lang,
        rank: 0,
        val: t_resvalue { r#type: 0, data: 0 },
    };
    let r = top_scan(a, file, &mut k);
    if r < 0 {
        return Err(r);
    }
    if k.rank == 0 {
        return Err(E_NOENT);
    }
    Ok(k.val)
}

pub(crate) fn string(a: &t_arsc, q: &t_resquery, out: t_text) -> c_int {
    let mut id = q.res_id;
    let mut depth = 0i32;
    while depth <= ARSC_MAX_REF {
        depth = depth.wrapping_add(1);
        let v = match lookup(a, id, q.lang) {
            Ok(v) => v,
            Err(e) => return e,
        };
        if v.r#type == RES_T_STRING {
            return pool::get_copied(a.strings, v.data, out);
        }
        if v.r#type != RES_T_REFERENCE {
            return E_INVAL;
        }
        id = v.data;
    }
    crate::E_RANGE
}

fn pkg_pool(base: *const u8, file: &[u8], c: &t_reschunk, field: u32) -> c_int {
    let Some(off) = rd32_at(file, u64::from(c.off).saturating_add(u64::from(field))) else {
        return E_INVAL;
    };
    if off < c.hsize || off >= c.size {
        return E_INVAL;
    }
    let sub = span_at(base, c.off.wrapping_add(off), c.size.wrapping_sub(off));
    let mut tmp = t_respool_empty();
    pool::open(&mut tmp, sub)
}

fn t_respool_empty() -> crate::t_respool {
    crate::t_respool {
        p: core::ptr::null(),
        size: 0,
        hsize: 0,
        count: 0,
        strings: 0,
        utf8: 0,
    }
}

fn top(a: &mut t_arsc, file: &[u8], c: &t_reschunk) -> c_int {
    if c.r#type == RES_STRING_POOL {
        if a.has_pool != 0 {
            return E_INVAL;
        }
        a.has_pool = 1;
        let sub = span_at(a.file.p, c.off, c.size);
        return pool::open(&mut a.strings, sub);
    }
    if c.r#type != RES_TABLE_PACKAGE {
        return E_OK;
    }
    if c.hsize < 284 || pkg_pool(a.file.p, file, c, 268) < 0 || pkg_pool(a.file.p, file, c, 276) < 0
    {
        return E_INVAL;
    }
    a.packages = a.packages.wrapping_add(1);
    E_OK
}

pub(crate) fn open(a: &mut t_arsc, file: t_span) -> c_int {
    let b = match bytes(file) {
        Ok(b) => b,
        Err(e) => return e,
    };
    let c = match chunk(b, 0) {
        Ok(c) => c,
        Err(e) => return e,
    };
    a.file.p = file.p;
    a.file.len = c.size as usize;
    a.body = c.hsize;
    a.has_pool = 0;
    a.packages = 0;
    let limited = t_span {
        p: file.p,
        len: c.size as usize,
    };
    let b = match bytes(limited) {
        Ok(b) => b,
        Err(_) => {
            a.file.p = core::ptr::null();
            return E_INVAL;
        }
    };
    let mut pos = c.hsize;
    let mut step = if c.r#type == RES_TABLE && c.hsize >= 12 {
        next(b, &mut pos)
    } else {
        Err(E_INVAL)
    };
    while let Ok(Some(ch)) = step {
        let code = top(a, b, &ch);
        if code != E_OK {
            step = Err(code);
            break;
        }
        step = next(b, &mut pos);
    }
    if step.is_ok() && a.has_pool != 0 {
        return E_OK;
    }
    a.file.p = core::ptr::null();
    E_INVAL
}
