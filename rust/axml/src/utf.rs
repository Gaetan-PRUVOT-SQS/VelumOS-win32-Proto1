fn lead_need(b: u8) -> u32 {
    if (0xc2..=0xdf).contains(&b) {
        return 1;
    }
    if (0xe0..=0xef).contains(&b) {
        return 2;
    }
    if (0xf0..=0xf4).contains(&b) {
        return 3;
    }
    0
}

fn second_ok(lead: u8, b: u8) -> bool {
    match lead {
        0xe0 => (0xa0..=0xbf).contains(&b),
        0xed => (0x80..=0x9f).contains(&b),
        0xf0 => (0x90..=0xbf).contains(&b),
        0xf4 => (0x80..=0x8f).contains(&b),
        _ => b & 0xc0 == 0x80,
    }
}

pub(crate) fn seq(s: &[u8]) -> u32 {
    let Some(&lead) = s.first() else {
        return 0;
    };
    if lead == 0 {
        return 0;
    }
    if lead < 0x80 {
        return 1;
    }
    let need = lead_need(lead);
    let avail = u32::try_from(s.len()).unwrap_or(u32::MAX);
    if need == 0 || need >= avail {
        return 0;
    }
    let Some(&second) = s.get(1) else {
        return 0;
    };
    if !second_ok(lead, second) {
        return 0;
    }
    let mut i = 2u32;
    while i <= need {
        let Some(&b) = s.get(usize::try_from(i).unwrap_or(usize::MAX)) else {
            return 0;
        };
        if b & 0xc0 != 0x80 {
            return 0;
        }
        i = i.wrapping_add(1);
    }
    need.wrapping_add(1)
}

fn bits(cp: u32, shift: u32) -> u8 {
    u8::try_from((cp >> shift) & 0x3f).unwrap_or(0)
}

fn put_wide(cp: u32, d: &mut [u8]) {
    if cp < 0x1_0000 {
        if d.len() < 3 {
            return;
        }
        d[0] = 0xe0 | u8::try_from(cp >> 12).unwrap_or(0);
        d[1] = 0x80 | bits(cp, 6);
        d[2] = 0x80 | bits(cp, 0);
        return;
    }
    if d.len() < 4 {
        return;
    }
    d[0] = 0xf0 | u8::try_from(cp >> 18).unwrap_or(0);
    d[1] = 0x80 | bits(cp, 12);
    d[2] = 0x80 | bits(cp, 6);
    d[3] = 0x80 | bits(cp, 0);
}

pub(crate) fn put(cp: u32, out: &mut [u8], w: usize) -> u32 {
    let n =
        1usize + usize::from(cp >= 0x80) + usize::from(cp >= 0x800) + usize::from(cp >= 0x1_0000);
    if w >= out.len() {
        return 0;
    }
    let room = out.len() - w;
    if room <= n {
        return 0;
    }
    let d = &mut out[w..];
    if n == 1 {
        d[0] = u8::try_from(cp).unwrap_or(0);
    }
    if n == 2 {
        d[0] = 0xc0 | bits(cp, 6);
        d[1] = 0x80 | bits(cp, 0);
    }
    if n > 2 {
        put_wide(cp, d);
    }
    u32::try_from(n).unwrap_or(0)
}
