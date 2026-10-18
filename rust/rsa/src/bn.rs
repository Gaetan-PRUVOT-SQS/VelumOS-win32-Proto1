use crate::RSA_WORDS;

#[derive(Clone)]
pub(crate) struct Bn {
    w: [u32; RSA_WORDS],
}

pub(crate) struct Mod {
    pub(crate) n: Bn,
    pub(crate) nw: u32,
}

impl Bn {
    pub(crate) fn zero() -> Self {
        Self { w: [0; RSA_WORDS] }
    }
}

fn limbs(nw: u32) -> usize {
    (nw as usize).min(RSA_WORDS)
}

pub(crate) fn bn_from_be(p: &[u8]) -> Bn {
    let mut r = Bn::zero();
    let full = p.len();
    let n = full.min(crate::RSA_MAX_BYTES);
    for i in 0..n {
        let shift = ((i % 4) * 8) as u32;
        r.w[i / 4] |= u32::from(p[full - 1 - i]) << shift;
    }
    r
}

pub(crate) fn bn_to_be(a: &Bn, out: &mut [u8]) {
    let full = out.len();
    let n = full.min(crate::RSA_MAX_BYTES);
    for i in 0..n {
        let shift = ((i % 4) * 8) as u32;
        out[full - 1 - i] = (a.w[i / 4] >> shift) as u8;
    }
}

pub(crate) fn bn_cmp(a: &Bn, b: &Bn, mut nw: u32) -> i32 {
    nw = limbs(nw) as u32;
    while nw > 0 {
        nw -= 1;
        let i = nw as usize;
        if a.w[i] > b.w[i] {
            return 1;
        }
        if a.w[i] < b.w[i] {
            return -1;
        }
    }
    0
}

pub(crate) fn bn_add(r: &mut Bn, b: &Bn, nw: u32) -> u32 {
    let mut c = 0u64;
    for i in 0..limbs(nw) {
        c += u64::from(r.w[i]) + u64::from(b.w[i]);
        r.w[i] = c as u32;
        c >>= 32;
    }
    c as u32
}

pub(crate) fn bn_sub(r: &mut Bn, b: &Bn, nw: u32) -> u32 {
    let mut borrow = 0u32;
    for i in 0..limbs(nw) {
        let d = u64::from(r.w[i])
            .wrapping_sub(u64::from(b.w[i]))
            .wrapping_sub(u64::from(borrow));
        r.w[i] = d as u32;
        borrow = (d >> 63) as u32;
    }
    borrow
}

pub(crate) fn bn_shl1(r: &mut Bn, nw: u32) -> u32 {
    let mut c = 0u32;
    for i in 0..limbs(nw) {
        let t = r.w[i] >> 31;
        r.w[i] = (r.w[i] << 1) | c;
        c = t;
    }
    c
}

fn mod_fix(r: &mut Bn, m: &Mod, carry: u32) {
    if carry != 0 || bn_cmp(r, &m.n, m.nw) >= 0 {
        bn_sub(r, &m.n, m.nw);
    }
}

pub(crate) fn bn_modmul(a: &Bn, b: &Bn, m: &Mod) -> Bn {
    let mut acc = Bn::zero();
    if m.nw > RSA_WORDS as u32 {
        return acc;
    }
    let mut i = m.nw * 32;
    while i > 0 {
        i -= 1;
        let carry = bn_shl1(&mut acc, m.nw);
        mod_fix(&mut acc, m, carry);
        let limb = (i / 32) as usize;
        if (a.w[limb] >> (i % 32)) & 1 == 1 {
            let carry = bn_add(&mut acc, b, m.nw);
            mod_fix(&mut acc, m, carry);
        }
    }
    acc
}

pub(crate) fn bn_modexp(base: &Bn, e: u32, m: &Mod) -> Bn {
    let mut bit = 0x8000_0000u32;
    while bit != 0 && e & bit == 0 {
        bit >>= 1;
    }
    let mut acc = base.clone();
    bit >>= 1;
    while bit != 0 {
        acc = bn_modmul(&acc, &acc, m);
        if e & bit != 0 {
            acc = bn_modmul(&acc, base, m);
        }
        bit >>= 1;
    }
    acc
}

#[cfg(test)]
mod tests {
    use super::*;

    fn limb(words: &[u32]) -> Bn {
        let mut b = Bn::zero();
        for (i, w) in words.iter().enumerate() {
            b.w[i] = *w;
        }
        b
    }

    fn modulo(words: &[u32]) -> Mod {
        Mod {
            n: limb(words),
            nw: words.len() as u32,
        }
    }

    #[test]
    fn from_be_five_bytes() {
        let b = bn_from_be(&[0x12, 0x34, 0x56, 0x78, 0x9a]);
        assert_eq!(b.w[0], 0x3456_789a);
        assert_eq!(b.w[1], 0x12);
        assert_eq!(b.w[2], 0);
    }

    #[test]
    fn from_be_keeps_low_bytes_past_max() {
        let mut p = vec![0xAA];
        p.extend(std::iter::repeat_n(0, 511));
        p.push(0x01);
        let b = bn_from_be(&p);
        assert_eq!(b.w[0] & 0xff, 0x01);
        assert_eq!(b.w[127] >> 24, 0);
    }

    #[test]
    fn to_be_roundtrip() {
        let src = [0x12u8, 0x34, 0x56, 0x78, 0x9a];
        let b = bn_from_be(&src);
        let mut out = [0u8; 5];
        bn_to_be(&b, &mut out);
        assert_eq!(out, src);
    }

    #[test]
    fn cmp_signs() {
        let a = limb(&[1, 2]);
        let b = limb(&[1, 3]);
        assert_eq!(bn_cmp(&a, &a, 2), 0);
        assert_eq!(bn_cmp(&b, &a, 2), 1);
        assert_eq!(bn_cmp(&a, &b, 2), -1);
        assert_eq!(bn_cmp(&a, &b, 0), 0);
    }

    #[test]
    fn add_carry_and_sub_borrow() {
        let mut r = limb(&[0xffff_ffff, 0]);
        assert_eq!(bn_add(&mut r, &limb(&[1, 0]), 2), 0);
        assert_eq!(r.w[0], 0);
        assert_eq!(r.w[1], 1);
        let mut z = limb(&[0]);
        assert_eq!(bn_sub(&mut z, &limb(&[1]), 1), 1);
        assert_eq!(z.w[0], 0xffff_ffff);
        let mut top = limb(&[0x8000_0000]);
        assert_eq!(bn_shl1(&mut top, 1), 1);
        assert_eq!(top.w[0], 0);
    }

    #[test]
    fn modmul_small() {
        let m = modulo(&[13]);
        assert_eq!(bn_modmul(&limb(&[6]), &limb(&[7]), &m).w[0], 3);
        assert_eq!(bn_modmul(&limb(&[8]), &limb(&[8]), &m).w[0], 12);
    }

    #[test]
    fn modexp_reduces() {
        let m = modulo(&[13]);
        assert_eq!(bn_modexp(&limb(&[5]), 3, &m).w[0], 8);
        assert_eq!(bn_modexp(&limb(&[2]), 10, &m).w[0], 10);
        let big = modulo(&[1, 1]);
        let raised = bn_modexp(&limb(&[2]), 33, &big);
        assert_eq!(raised.w[0], 0xffff_ffff);
        assert_eq!(raised.w[1], 0);
    }
}
