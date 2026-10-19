use crate::bn::{Mod, bn_cmp, bn_from_be, bn_modexp, bn_to_be};
use crate::spki::rsa_key_check;
use crate::{E_INVAL, RSA_DIGEST_LEN, RSA_MAX_BYTES, RsaPub, c_int};

const DI: [u8; 19] = [
    0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
    0x00, 0x04, 0x20,
];

fn emsa(k: usize, digest: &[u8]) -> Result<[u8; RSA_MAX_BYTES], c_int> {
    if digest.len() != RSA_DIGEST_LEN || k > RSA_MAX_BYTES {
        return Err(E_INVAL);
    }
    let Some(ps) = k.checked_sub(3 + DI.len() + RSA_DIGEST_LEN) else {
        return Err(E_INVAL);
    };
    let ff_end = 2 + ps;
    let di_at = ff_end + 1;
    let dig_at = di_at + DI.len();
    let Some(end) = dig_at.checked_add(RSA_DIGEST_LEN) else {
        return Err(E_INVAL);
    };
    if end != k {
        return Err(E_INVAL);
    }
    let mut em = [0u8; RSA_MAX_BYTES];
    em[0] = 0x00;
    em[1] = 0x01;
    em[2..ff_end].fill(0xff);
    em[ff_end] = 0x00;
    em[di_at..dig_at].copy_from_slice(&DI);
    em[dig_at..k].copy_from_slice(digest);
    Ok(em)
}

fn block_differs(a: &[u8], b: &[u8]) -> bool {
    let n = a.len();
    if b.len() != n {
        return true;
    }
    let mut diff = 0u32;
    for i in 0..n {
        diff |= u32::from(a[i] ^ b[i]);
    }
    diff != 0
}

fn rsa_public(key: &RsaPub, sig: &[u8]) -> Result<[u8; RSA_MAX_BYTES], c_int> {
    let nlen = key.nlen as usize;
    let m = Mod {
        n: bn_from_be(&key.n[..nlen]),
        nw: key.nlen.div_ceil(4),
    };
    let s = bn_from_be(sig);
    if bn_cmp(&s, &m.n, m.nw) >= 0 {
        return Err(E_INVAL);
    }
    let raised = bn_modexp(&s, key.e, &m);
    let mut out = [0u8; RSA_MAX_BYTES];
    bn_to_be(&raised, &mut out[..nlen]);
    Ok(out)
}

pub(crate) fn pkcs1_sha256(key: &RsaPub, sig: &[u8], digest: &[u8]) -> c_int {
    let rc = rsa_key_check(key);
    if rc < 0 {
        return rc;
    }
    if sig.len() != key.nlen as usize || digest.len() != RSA_DIGEST_LEN {
        return E_INVAL;
    }
    let got = match rsa_public(key, sig) {
        Ok(block) => block,
        Err(code) => return code,
    };
    let want = match emsa(key.nlen as usize, digest) {
        Ok(block) => block,
        Err(code) => return code,
    };
    let n = key.nlen as usize;
    if block_differs(&want[..n], &got[..n]) {
        return E_INVAL;
    }
    0
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn emsa_256_and_minimum() {
        let digest = [0x11u8; 32];
        let em = emsa(256, &digest).unwrap();
        assert_eq!(em[0], 0x00);
        assert_eq!(em[1], 0x01);
        assert!(em[2..204].iter().all(|b| *b == 0xff));
        assert_eq!(em[204], 0x00);
        assert_eq!(&em[205..224], &DI);
        assert_eq!(&em[224..256], &digest[..]);
        assert!(em[256..].iter().all(|b| *b == 0));
        let bare = emsa(54, &digest).unwrap();
        assert_eq!(&bare[..3], &[0x00, 0x01, 0x00]);
        assert_eq!(&bare[3..22], &DI);
        assert_eq!(&bare[22..54], &digest[..]);
        assert_eq!(emsa(53, &digest).unwrap_err(), E_INVAL);
    }

    #[test]
    fn differs_first_and_last_byte() {
        let a = [0u8, 1, 2, 3];
        let mut b = a;
        assert!(!block_differs(&a, &b));
        b[0] = 9;
        assert!(block_differs(&a, &b));
        b = a;
        b[3] = 9;
        assert!(block_differs(&a, &b));
        assert!(block_differs(&a, &a[..3]));
    }
}
