use core::ffi::c_char;

use crate::crc::crc32;
use crate::inflate::inflate_slice;
use crate::{
    E_INVAL, E_NOENT, E_NOTSUP, E_OVERFLOW, E_RANGE, ZIP_MAX_ENTRIES, ZIP_NAME_MAX, bytes_of,
    t_span, t_zip, t_zipent,
};

const SIG_EOCD: u32 = 0x0605_4b50;
const SIG_ZIP64_LOC: u32 = 0x0706_4b50;
const SIG_CFH: u32 = 0x0201_4b50;
const SIG_LFH: u32 = 0x0403_4b50;

#[derive(Clone, Copy)]
struct Ent {
    name_off: usize,
    name_len: u32,
    method: u32,
    crc: u32,
    csize: u32,
    ulen: u32,
    data_off: u32,
    lfh_off: u32,
}

pub(crate) fn open_mut(z: &mut t_zip, file: t_span) -> i32 {
    z.file = file;
    z.cd_off = 0;
    z.cd_size = 0;
    z.count = 0;
    match open_checked(z) {
        Ok(()) => 0,
        Err(e) => {
            z.count = 0;
            e
        }
    }
}

fn open_checked(z: &mut t_zip) -> Result<(), i32> {
    let f = bytes_of(z.file.p, z.file.len)?;
    let pos = eocd_find(f)?;
    eocd_parse(z, f, pos)?;
    let mut off = z.cd_off;
    let mut i = 0u32;
    while i < z.count {
        let e = cd_read(f, z.cd_off, z.cd_size, &mut off)?;
        reject_dup(f, z.cd_off, z.cd_size, &e, i)?;
        i = i.checked_add(1).ok_or(E_INVAL)?;
    }
    if u64::from(off) != u64::try_from(pos).map_err(|_| E_INVAL)? {
        return Err(E_INVAL);
    }
    Ok(())
}

pub(crate) fn entry_at(z: &t_zip, index: u32, out: &mut t_zipent) -> Result<(), i32> {
    if z.count > ZIP_MAX_ENTRIES {
        return Err(E_INVAL);
    }
    let f = bytes_of(z.file.p, z.file.len)?;
    let mut off = z.cd_off;
    let mut e = cd_read(f, z.cd_off, z.cd_size, &mut off)?;
    let mut i = 0u32;
    while i < index {
        e = cd_read(f, z.cd_off, z.cd_size, &mut off)?;
        i += 1;
    }
    fill_ent(z.file.p, &e, out)
}

pub(crate) fn find_name(z: &t_zip, name: *const c_char, out: &mut t_zipent) -> Result<(), i32> {
    if z.count > ZIP_MAX_ENTRIES {
        return Err(E_INVAL);
    }
    let f = bytes_of(z.file.p, z.file.len)?;
    let mut off = z.cd_off;
    let mut i = 0u32;
    while i < z.count {
        let e = match cd_read(f, z.cd_off, z.cd_size, &mut off) {
            Ok(e) => e,
            Err(_) => return Err(E_INVAL),
        };
        fill_ent(z.file.p, &e, out)?;
        let bytes = name_at(f, e.name_off, e.name_len)?;
        if cstr_eq(name, bytes) {
            return Ok(());
        }
        i += 1;
    }
    Err(E_NOENT)
}

pub(crate) fn extract_ref(z: &t_zip, e: &t_zipent, out: *mut u8, cap: usize) -> Result<i64, i32> {
    let file = bytes_of(z.file.p, z.file.len)?;
    let end = u64::from(e.data_off)
        .checked_add(u64::from(e.csize))
        .ok_or(E_INVAL)?;
    if end > u64::try_from(file.len()).map_err(|_| E_INVAL)? {
        return Err(E_INVAL);
    }
    if u64::from(e.r#usize) > u64::try_from(cap).map_err(|_| E_OVERFLOW)? {
        return Err(E_OVERFLOW);
    }
    let n = match e.method {
        0 => copy_stored(file, e, out)?,
        8 => inflate_member(file, e, out)?,
        _ => return Err(E_NOTSUP),
    };
    if n != e.r#usize as usize {
        return Err(E_INVAL);
    }
    if crc_buf(out, n)? != e.crc {
        return Err(E_INVAL);
    }
    i64::try_from(n).map_err(|_| E_OVERFLOW)
}

fn eocd_find(f: &[u8]) -> Result<usize, i32> {
    if f.len() < 22 || f.len() > u32::MAX as usize {
        return Err(E_INVAL);
    }
    let mut low = 0usize;
    if f.len() > 22 + 65535 {
        low = f.len() - 22 - 65535;
    }
    let mut i = f.len() - 22;
    loop {
        let sig = rd32(f, i)?;
        let comment = rd16(f, i + 20)?;
        if sig == SIG_EOCD && comment as usize == f.len() - i - 22 {
            return Ok(i);
        }
        if i == low {
            return Err(E_INVAL);
        }
        i -= 1;
    }
}

fn eocd_parse(z: &mut t_zip, f: &[u8], pos: usize) -> Result<(), i32> {
    if pos >= 20 && rd32(f, pos - 20)? == SIG_ZIP64_LOC {
        return Err(E_NOTSUP);
    }
    let disk = rd16(f, pos + 4)?;
    let cd_disk = rd16(f, pos + 6)?;
    let disk_entries = rd16(f, pos + 8)?;
    let total = rd16(f, pos + 10)?;
    if disk != 0 || cd_disk != 0 || disk_entries != total {
        return Err(E_NOTSUP);
    }
    z.count = total;
    z.cd_size = rd32(f, pos + 12)?;
    z.cd_off = rd32(f, pos + 16)?;
    if z.count == 0xffff || z.cd_size == 0xffff_ffff || z.cd_off == 0xffff_ffff {
        return Err(E_NOTSUP);
    }
    let sum = u64::from(z.cd_off) + u64::from(z.cd_size);
    if sum != u64::try_from(pos).map_err(|_| E_INVAL)? {
        return Err(E_INVAL);
    }
    if z.count > ZIP_MAX_ENTRIES {
        return Err(E_RANGE);
    }
    Ok(())
}

fn cd_read(f: &[u8], cd_off: u32, cd_size: u32, off: &mut u32) -> Result<Ent, i32> {
    let end = u64::from(cd_off) + u64::from(cd_size);
    let at = u64::from(*off);
    if at < u64::from(cd_off) || at > end || end - at < 46 {
        return Err(E_INVAL);
    }
    let at_us = *off as usize;
    let name_len = rd16(f, at_us + 28)?;
    let extra_len = rd16(f, at_us + 30)?;
    let comment_len = rd16(f, at_us + 32)?;
    let next = at + 46 + u64::from(name_len) + u64::from(extra_len) + u64::from(comment_len);
    if next > end || next > u64::from(u32::MAX) {
        return Err(E_INVAL);
    }
    let ent = cd_fields(f, at_us)?;
    let ent = cd_local(f, cd_off, ent)?;
    *off = u32::try_from(next).map_err(|_| E_INVAL)?;
    Ok(ent)
}

fn cd_fields(f: &[u8], at: usize) -> Result<Ent, i32> {
    if rd32(f, at)? != SIG_CFH {
        return Err(E_INVAL);
    }
    if rd16(f, at + 8)? & 0x41 != 0 || rd16(f, at + 34)? != 0 {
        return Err(E_NOTSUP);
    }
    let method = rd16(f, at + 10)?;
    let crc = rd32(f, at + 16)?;
    let csize = rd32(f, at + 20)?;
    let ulen = rd32(f, at + 24)?;
    let name_len = rd16(f, at + 28)?;
    let lfh_off = rd32(f, at + 42)?;
    if method != 0 && method != 8 {
        return Err(E_NOTSUP);
    }
    if csize == 0xffff_ffff || ulen == 0xffff_ffff || lfh_off == 0xffff_ffff {
        return Err(E_NOTSUP);
    }
    if name_len > ZIP_NAME_MAX {
        return Err(E_RANGE);
    }
    if name_len == 0 || (method == 0 && csize != ulen) {
        return Err(E_INVAL);
    }
    let name_off = at.checked_add(46).ok_or(E_INVAL)?;
    let _name = name_at(f, name_off, name_len)?;
    Ok(Ent {
        name_off,
        name_len,
        method,
        crc,
        csize,
        ulen,
        data_off: 0,
        lfh_off,
    })
}

fn cd_local(f: &[u8], cd_off: u32, mut e: Ent) -> Result<Ent, i32> {
    let need = u64::from(e.lfh_off) + 30 + u64::from(e.name_len);
    if need > u64::from(cd_off) {
        return Err(E_INVAL);
    }
    let lfh = e.lfh_off as usize;
    if rd32(f, lfh)? != SIG_LFH {
        return Err(E_INVAL);
    }
    if rd16(f, lfh + 6)? & 0x41 != 0 {
        return Err(E_NOTSUP);
    }
    if rd16(f, lfh + 8)? != e.method || rd16(f, lfh + 26)? != e.name_len {
        return Err(E_INVAL);
    }
    let local = name_at(f, lfh + 30, e.name_len)?;
    let central = name_at(f, e.name_off, e.name_len)?;
    if local != central {
        return Err(E_INVAL);
    }
    let extra = rd16(f, lfh + 28)?;
    let end = need + u64::from(extra);
    if end + u64::from(e.csize) > u64::from(cd_off) {
        return Err(E_INVAL);
    }
    e.data_off = u32::try_from(end).map_err(|_| E_INVAL)?;
    Ok(e)
}

fn reject_dup(f: &[u8], cd_off: u32, cd_size: u32, e: &Ent, index: u32) -> Result<(), i32> {
    let want = name_at(f, e.name_off, e.name_len)?;
    let mut off = cd_off;
    let mut j = 0u32;
    while j < index {
        let prev = cd_read(f, cd_off, cd_size, &mut off).map_err(|_| E_INVAL)?;
        if prev.name_len == e.name_len {
            let got = name_at(f, prev.name_off, prev.name_len)?;
            if got == want {
                return Err(E_INVAL);
            }
        }
        j += 1;
    }
    Ok(())
}

fn fill_ent(base: *const u8, e: &Ent, out: &mut t_zipent) -> Result<(), i32> {
    if base.is_null() || e.name_off > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: name_off est dans le fichier validé à la lecture du catalogue, base non nul.
    let name = unsafe { base.add(e.name_off).cast::<c_char>() };
    *out = t_zipent {
        name,
        name_len: e.name_len,
        method: e.method,
        crc: e.crc,
        csize: e.csize,
        r#usize: e.ulen,
        data_off: e.data_off,
        lfh_off: e.lfh_off,
    };
    Ok(())
}

fn copy_stored(file: &[u8], e: &t_zipent, out: *mut u8) -> Result<usize, i32> {
    if e.csize != e.r#usize {
        return Err(E_INVAL);
    }
    let n = e.r#usize as usize;
    if n == 0 {
        return Ok(0);
    }
    let start = e.data_off as usize;
    let end = start.checked_add(n).ok_or(E_INVAL)?;
    let src = file.get(start..end).ok_or(E_INVAL)?;
    if out.is_null() {
        return Err(E_INVAL);
    }
    let mut i = 0usize;
    while i < n {
        // SAFETY: i < n <= cap, out non nul, pas de chevauchement avec file.
        unsafe { out.add(i).write(src[i]) };
        i += 1;
    }
    Ok(n)
}

fn inflate_member(file: &[u8], e: &t_zipent, out: *mut u8) -> Result<usize, i32> {
    let start = e.data_off as usize;
    let end = start.checked_add(e.csize as usize).ok_or(E_INVAL)?;
    let src = file.get(start..end).ok_or(E_INVAL)?;
    match inflate_slice(src, out, e.r#usize as usize) {
        Ok(n) => Ok(n),
        Err(E_OVERFLOW) => Err(E_INVAL),
        Err(err) => Err(err),
    }
}

fn crc_buf(out: *const u8, n: usize) -> Result<u32, i32> {
    if n == 0 {
        return Ok(crc32(0, &[]));
    }
    if out.is_null() || n > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: n octets déjà écrits dans out, pointeur non nul.
    let bytes = unsafe { core::slice::from_raw_parts(out, n) };
    Ok(crc32(0, bytes))
}

fn cstr_eq(name: *const c_char, bytes: &[u8]) -> bool {
    if name.is_null() || bytes.len() > isize::MAX as usize {
        return false;
    }
    let mut k = 0usize;
    while k < bytes.len() {
        // SAFETY: name est une chaîne C, k <= 255.
        let c = unsafe { name.add(k).read() };
        if c == 0 || c as u8 != bytes[k] {
            return false;
        }
        k += 1;
    }
    // SAFETY: octet juste après le nom comparé, pour le NUL final.
    unsafe { name.add(bytes.len()).read() == 0 }
}

fn name_at(f: &[u8], off: usize, len: u32) -> Result<&[u8], i32> {
    let end = off.checked_add(len as usize).ok_or(E_INVAL)?;
    f.get(off..end).ok_or(E_INVAL)
}

fn rd16(f: &[u8], off: usize) -> Result<u32, i32> {
    let b0 = *f.get(off).ok_or(E_INVAL)?;
    let b1 = *f.get(off.checked_add(1).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    Ok(u32::from(b0) | (u32::from(b1) << 8))
}

fn rd32(f: &[u8], off: usize) -> Result<u32, i32> {
    let lo = rd16(f, off)?;
    let hi = rd16(f, off.checked_add(2).ok_or(E_INVAL)?)?;
    Ok(lo | (hi << 16))
}
