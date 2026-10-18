use crate::{E_INVAL, E_OVERFLOW};

const LBASE: [u16; 29] = [
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131,
    163, 195, 227, 258,
];

const DBASE: [u16; 30] = [
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537,
    2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577,
];

const ORDER: [usize; 19] = [
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15,
];

struct Huff {
    count: [u16; 16],
    sym: [u16; 288],
    max: u32,
}

struct Bits<'a> {
    input: &'a [u8],
    pos: usize,
    acc: u32,
    nb: u32,
}

struct Sink {
    ptr: *mut u8,
    cap: usize,
    n: usize,
}

pub(crate) fn inflate_slice(input: &[u8], out: *mut u8, cap: usize) -> Result<usize, i32> {
    let mut bits = Bits {
        input,
        pos: 0,
        acc: 0,
        nb: 0,
    };
    let mut sink = Sink {
        ptr: out,
        cap,
        n: 0,
    };
    loop {
        let last = bits.pull(1)?;
        match bits.pull(2)? {
            0 => stored(&mut bits, &mut sink)?,
            1 => {
                let (lit, dist) = fixed_pair()?;
                codes(&mut bits, &mut sink, &lit, &dist)?;
            }
            2 => {
                let (lit, dist) = dynamic_pair(&mut bits)?;
                codes(&mut bits, &mut sink, &lit, &dist)?;
            }
            _ => return Err(E_INVAL),
        }
        if last == 1 {
            return Ok(sink.n);
        }
    }
}

impl Bits<'_> {
    fn pull(&mut self, n: u32) -> Result<u32, i32> {
        if n > 16 {
            return Err(E_INVAL);
        }
        while self.nb < n {
            let b = *self.input.get(self.pos).ok_or(E_INVAL)?;
            let shifted = u32::from(b).checked_shl(self.nb).ok_or(E_INVAL)?;
            self.acc |= shifted;
            self.pos += 1;
            self.nb += 8;
        }
        let mask = if n == 0 { 0 } else { u32::MAX >> (32 - n) };
        let v = self.acc & mask;
        self.acc = self.acc.checked_shr(n).ok_or(E_INVAL)?;
        self.nb -= n;
        Ok(v)
    }
}

impl Sink {
    fn write(&mut self, b: u8) -> Result<(), i32> {
        if self.n >= self.cap {
            return Err(E_OVERFLOW);
        }
        write_byte(self.ptr, self.n, b)?;
        self.n += 1;
        Ok(())
    }

    fn copy_from(&mut self, src: &[u8]) -> Result<(), i32> {
        if src.len() > self.cap.saturating_sub(self.n) {
            return Err(E_OVERFLOW);
        }
        let mut i = 0usize;
        while i < src.len() {
            write_byte(self.ptr, self.n + i, src[i])?;
            i += 1;
        }
        self.n += src.len();
        Ok(())
    }

    fn repeat_dist(&mut self, len: usize, dist: usize) -> Result<(), i32> {
        if dist == 0 || dist > self.n {
            return Err(E_INVAL);
        }
        if len > self.cap.saturating_sub(self.n) {
            return Err(E_OVERFLOW);
        }
        let mut left = len;
        while left > 0 {
            let b = read_byte(self.ptr, self.n - dist)?;
            write_byte(self.ptr, self.n, b)?;
            self.n += 1;
            left -= 1;
        }
        Ok(())
    }
}

fn write_byte(out: *mut u8, i: usize, b: u8) -> Result<(), i32> {
    if out.is_null() || i > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: i < cap, out non nul. zip_extract et inflate_raw interdisent le chevauchement.
    unsafe { out.add(i).write(b) };
    Ok(())
}

fn read_byte(out: *mut u8, i: usize) -> Result<u8, i32> {
    if out.is_null() || i > isize::MAX as usize {
        return Err(E_INVAL);
    }
    // SAFETY: i < n, l'octet a déjà été écrit par write_byte.
    Ok(unsafe { out.add(i).read() })
}

fn rd16(f: &[u8], off: usize) -> Result<u32, i32> {
    let b0 = *f.get(off).ok_or(E_INVAL)?;
    let b1 = *f.get(off.checked_add(1).ok_or(E_INVAL)?).ok_or(E_INVAL)?;
    Ok(u32::from(b0) | (u32::from(b1) << 8))
}

fn stored(bits: &mut Bits<'_>, sink: &mut Sink) -> Result<(), i32> {
    let rewind = (bits.nb / 8) as usize;
    if bits.pos < rewind {
        return Err(E_INVAL);
    }
    bits.pos -= rewind;
    bits.acc = 0;
    bits.nb = 0;
    if bits.input.len().saturating_sub(bits.pos) < 4 {
        return Err(E_INVAL);
    }
    let len = rd16(bits.input, bits.pos)?;
    let nlen = rd16(bits.input, bits.pos + 2)?;
    if len ^ nlen != 0xffff {
        return Err(E_INVAL);
    }
    let start = bits.pos + 4;
    let end = start.checked_add(len as usize).ok_or(E_INVAL)?;
    if end > bits.input.len() {
        return Err(E_INVAL);
    }
    sink.copy_from(&bits.input[start..end])?;
    bits.pos = end;
    Ok(())
}

fn codes(bits: &mut Bits<'_>, sink: &mut Sink, lit: &Huff, dist: &Huff) -> Result<(), i32> {
    loop {
        let sym = huff_decode(bits, lit)?;
        if sym < 256 {
            sink.write(u8::try_from(sym).map_err(|_| E_INVAL)?)?;
            continue;
        }
        if sym == 256 {
            return Ok(());
        }
        let len = length_of(bits, sym)?;
        let d = distance_of(bits, dist)?;
        sink.repeat_dist(len, d)?;
    }
}

fn length_of(bits: &mut Bits<'_>, sym: u32) -> Result<usize, i32> {
    if !(257..=285).contains(&sym) {
        return Err(E_INVAL);
    }
    let idx = sym - 257;
    let mut extra = 0u32;
    if (8..28).contains(&idx) {
        extra = (idx >> 2) - 1;
    }
    let add = bits.pull(extra)?;
    let base = u32::from(*LBASE.get(idx as usize).ok_or(E_INVAL)?);
    Ok(base.checked_add(add).ok_or(E_INVAL)? as usize)
}

fn distance_of(bits: &mut Bits<'_>, dist: &Huff) -> Result<usize, i32> {
    let sym = huff_decode(bits, dist)?;
    if sym > 29 {
        return Err(E_INVAL);
    }
    let mut extra = 0u32;
    if sym >= 4 {
        extra = (sym >> 1) - 1;
    }
    let add = bits.pull(extra)?;
    let base = u32::from(*DBASE.get(sym as usize).ok_or(E_INVAL)?);
    Ok(base.checked_add(add).ok_or(E_INVAL)? as usize)
}

fn fixed_pair() -> Result<(Huff, Huff), i32> {
    let mut lens = [8u8; 288];
    for (i, slot) in lens.iter_mut().enumerate() {
        if (144..256).contains(&i) {
            *slot = 9;
        } else if (256..280).contains(&i) {
            *slot = 7;
        }
    }
    let mut lit = blank_huff();
    huff_build(&mut lit, &lens)?;
    let mut dist = blank_huff();
    huff_build(&mut dist, &[5u8; 32])?;
    Ok((lit, dist))
}

fn dynamic_pair(bits: &mut Bits<'_>) -> Result<(Huff, Huff), i32> {
    let hlit = bits.pull(5)? + 257;
    let hdist = bits.pull(5)? + 1;
    let hclen = bits.pull(4)? + 4;
    if hlit > 286 || hdist > 30 {
        return Err(E_INVAL);
    }
    let clen = dyn_clen(bits, hclen)?;
    let total = (hlit + hdist) as usize;
    let mut lens = [0u8; 316];
    dyn_lens(bits, &clen, &mut lens, total)?;
    if lens[256] == 0 {
        return Err(E_INVAL);
    }
    let mut lit = blank_huff();
    let mut dist = blank_huff();
    huff_build(&mut lit, &lens[..hlit as usize])?;
    huff_build(&mut dist, &lens[hlit as usize..total])?;
    Ok((lit, dist))
}

fn dyn_clen(bits: &mut Bits<'_>, hclen: u32) -> Result<Huff, i32> {
    let n = hclen as usize;
    if n > ORDER.len() {
        return Err(E_INVAL);
    }
    let mut lens = [0u8; 19];
    for i in 0..n {
        lens[ORDER[i]] = u8::try_from(bits.pull(3)?).map_err(|_| E_INVAL)?;
    }
    let mut h = blank_huff();
    if huff_build(&mut h, &lens)? {
        return Err(E_INVAL);
    }
    Ok(h)
}

fn dyn_repeat(bits: &mut Bits<'_>, sym: u32) -> Result<usize, i32> {
    let (nbits, base) = match sym {
        17 => (3, 3u32),
        18 => (7, 11),
        _ => (2, 3),
    };
    let v = bits.pull(nbits)?;
    Ok(base.checked_add(v).ok_or(E_INVAL)? as usize)
}

fn dyn_lens(bits: &mut Bits<'_>, code: &Huff, lens: &mut [u8], total: usize) -> Result<(), i32> {
    let mut i = 0usize;
    while i < total {
        let sym = huff_decode(bits, code)?;
        let (rep, val) = if sym < 16 {
            (1usize, u8::try_from(sym).map_err(|_| E_INVAL)?)
        } else {
            let rep = dyn_repeat(bits, sym)?;
            if sym == 16 {
                if i == 0 {
                    return Err(E_INVAL);
                }
                (rep, lens[i - 1])
            } else {
                (rep, 0)
            }
        };
        if rep > total - i {
            return Err(E_INVAL);
        }
        let end = i + rep;
        while i < end {
            lens[i] = val;
            i += 1;
        }
    }
    Ok(())
}

fn blank_huff() -> Huff {
    Huff {
        count: [0; 16],
        sym: [0; 288],
        max: 0,
    }
}

fn huff_build(h: &mut Huff, lens: &[u8]) -> Result<bool, i32> {
    h.count = [0; 16];
    h.sym = [0; 288];
    h.max = 0;
    for &len in lens {
        if len > 15 {
            return Err(E_INVAL);
        }
        if u32::from(len) > h.max {
            h.max = u32::from(len);
        }
        h.count[len as usize] = h.count[len as usize].checked_add(1).ok_or(E_INVAL)?;
    }
    h.count[0] = 0;
    let mut left: i32 = 1;
    let mut len = 1usize;
    while len <= 15 {
        let doubled = left.checked_mul(2).ok_or(E_INVAL)?;
        left = doubled
            .checked_sub(i32::from(h.count[len]))
            .ok_or(E_INVAL)?;
        if left < 0 {
            return Err(E_INVAL);
        }
        len += 1;
    }
    if left > 0 && h.max > 1 {
        return Err(E_INVAL);
    }
    let mut offs = [0u16; 16];
    let mut i = 1usize;
    while i < 15 {
        offs[i + 1] = offs[i].checked_add(h.count[i]).ok_or(E_INVAL)?;
        i += 1;
    }
    for (sym, &len) in lens.iter().enumerate() {
        if len == 0 {
            continue;
        }
        let slot = usize::from(offs[len as usize]);
        let dest = h.sym.get_mut(slot).ok_or(E_INVAL)?;
        *dest = u16::try_from(sym).map_err(|_| E_INVAL)?;
        offs[len as usize] = offs[len as usize].checked_add(1).ok_or(E_INVAL)?;
    }
    Ok(left > 0)
}

fn huff_decode(bits: &mut Bits<'_>, h: &Huff) -> Result<u32, i32> {
    let mut code = 0u32;
    let mut first = 0u32;
    let mut index = 0u32;
    let mut len = 0u32;
    while len < h.max {
        len += 1;
        if len as usize >= h.count.len() {
            return Err(E_INVAL);
        }
        code |= bits.pull(1)?;
        let count = u32::from(h.count[len as usize]);
        let delta = code.wrapping_sub(first);
        if delta < count {
            let at = index.checked_add(delta).ok_or(E_INVAL)? as usize;
            let sym = h.sym.get(at).copied().ok_or(E_INVAL)?;
            return Ok(u32::from(sym));
        }
        index = index.checked_add(count).ok_or(E_INVAL)?;
        let sum = first.checked_add(count).ok_or(E_INVAL)?;
        first = sum.checked_shl(1).ok_or(E_INVAL)?;
        code = code.checked_shl(1).ok_or(E_INVAL)?;
    }
    Err(E_INVAL)
}
