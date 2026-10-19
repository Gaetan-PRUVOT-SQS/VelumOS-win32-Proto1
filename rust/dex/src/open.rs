use crate::adler::adler32;
use crate::classes::{verify_classes, verify_ids};
use crate::rd::{at_u32, file, fits, item_size, ptr_at};
use crate::{DEX_T_PROTO, DEX_T_TYPE, E_INVAL, E_NOTSUP, E_OK, t_dex, t_span};
use core::ffi::c_int;

fn wipe(d: &mut t_dex) {
    d.p = core::ptr::null();
    d.len = 0;
    d.n = [0; 6];
    d.off = [0; 6];
}

fn check_header(d: &t_dex) -> Result<(), c_int> {
    if d.len < 0x70 || d.len > 0xffff_ffff {
        return Err(E_INVAL);
    }
    let Some(b) = file(d) else {
        return Err(E_INVAL);
    };
    if b[0] != b'd' || b[1] != b'e' || b[2] != b'x' || b[3] != b'\n' {
        return Err(E_INVAL);
    }
    if b[4] != b'0' || b[5] != b'3' || !(b'5'..=b'9').contains(&b[6]) || b[7] != 0 {
        return Err(E_INVAL);
    }
    let Some(file_size) = at_u32(d, 32) else {
        return Err(E_INVAL);
    };
    if u64::from(file_size) != d.len as u64 {
        return Err(E_INVAL);
    }
    let Some(header_size) = at_u32(d, 36) else {
        return Err(E_INVAL);
    };
    if header_size != 0x70 {
        return Err(E_INVAL);
    }
    let Some(endian) = at_u32(d, 40) else {
        return Err(E_INVAL);
    };
    if endian == 0x7856_3412 {
        return Err(E_NOTSUP);
    }
    if endian != 0x1234_5678 {
        return Err(E_INVAL);
    }
    let Some(sum) = at_u32(d, 8) else {
        return Err(E_INVAL);
    };
    let Some(p) = ptr_at(d.p, d.len, 12) else {
        return Err(E_INVAL);
    };
    if sum != adler32(p, d.len - 12) {
        return Err(E_INVAL);
    }
    Ok(())
}

fn load_tables(d: &mut t_dex) -> Result<(), c_int> {
    let mut i = 0u32;
    while i < 6 {
        let at = 56 + 8 * (i as usize);
        let Some(n) = at_u32(d, at) else {
            return Err(E_INVAL);
        };
        let Some(off) = at_u32(d, at + 4) else {
            return Err(E_INVAL);
        };
        d.n[i as usize] = n;
        d.off[i as usize] = off;
        let Some(size) = item_size(i) else {
            return Err(E_INVAL);
        };
        if n != 0
            && (off < 0x70 || off & 3 != 0 || !fits(d.len, u64::from(off), u64::from(n), size))
        {
            return Err(E_INVAL);
        }
        i += 1;
    }
    if d.n[DEX_T_TYPE as usize] > 0xffff || d.n[DEX_T_PROTO as usize] > 0xffff {
        return Err(E_INVAL);
    }
    Ok(())
}

fn check_areas(d: &t_dex) -> Result<(), c_int> {
    let Some(data_size) = at_u32(d, 104) else {
        return Err(E_INVAL);
    };
    let Some(data_off) = at_u32(d, 108) else {
        return Err(E_INVAL);
    };
    if !fits(d.len, u64::from(data_off), u64::from(data_size), 1) {
        return Err(E_INVAL);
    }
    let Some(link_size) = at_u32(d, 44) else {
        return Err(E_INVAL);
    };
    if link_size != 0 {
        let Some(link_off) = at_u32(d, 48) else {
            return Err(E_INVAL);
        };
        if !fits(d.len, u64::from(link_off), u64::from(link_size), 1) {
            return Err(E_INVAL);
        }
    }
    let Some(map) = at_u32(d, 52) else {
        return Err(E_INVAL);
    };
    if map < 0x70 || map & 3 != 0 || !fits(d.len, u64::from(map), 4, 1) {
        return Err(E_INVAL);
    }
    let Ok(at) = usize::try_from(map) else {
        return Err(E_INVAL);
    };
    let Some(count) = at_u32(d, at) else {
        return Err(E_INVAL);
    };
    if !fits(d.len, u64::from(map) + 4, u64::from(count), 12) {
        return Err(E_INVAL);
    }
    Ok(())
}

pub(crate) fn open_at(d: &mut t_dex, file: t_span) -> c_int {
    wipe(d);
    if file.p.is_null() {
        return E_INVAL;
    }
    d.p = file.p;
    d.len = file.len;
    let r = check_header(d)
        .and_then(|()| load_tables(d))
        .and_then(|()| check_areas(d))
        .and_then(|()| verify_ids(d))
        .and_then(|()| verify_classes(d));
    match r {
        Ok(()) => E_OK,
        Err(e) => {
            wipe(d);
            e
        }
    }
}
