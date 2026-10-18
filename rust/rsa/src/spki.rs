use crate::der::{DER_BITS, DER_NULL, DER_OID, DER_SEQ, der_only, der_take, der_uint};
use crate::{E_INVAL, E_NOTSUP, E_RANGE, RSA_MAX_BYTES, RSA_MIN_BYTES, RSA_MIN_EXP, RsaPub, c_int};

const RSA_OID: [u8; 9] = [0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01];

pub(crate) fn rsa_key_check(k: &RsaPub) -> c_int {
    if k.nlen < RSA_MIN_BYTES as u32 || k.nlen > RSA_MAX_BYTES as u32 {
        return E_RANGE;
    }
    let nlen = k.nlen as usize;
    if k.n[0] == 0 || (k.nlen == RSA_MIN_BYTES as u32 && k.n[0] & 0x80 == 0) {
        return E_RANGE;
    }
    if k.n[nlen - 1] & 1 == 0 || k.e & 1 == 0 || k.e < RSA_MIN_EXP {
        return E_INVAL;
    }
    0
}

fn alg_check(alg: &[u8]) -> Result<(), c_int> {
    let oid = der_take(alg, DER_OID)?;
    if oid.val.len() != RSA_OID.len() || oid.val != RSA_OID {
        return Err(E_NOTSUP);
    }
    let par = der_take(oid.rest, DER_NULL)?;
    if !par.val.is_empty() || !par.rest.is_empty() {
        return Err(E_INVAL);
    }
    Ok(())
}

fn key_fill(n: &[u8], e: &[u8]) -> Result<RsaPub, c_int> {
    if n.len() < RSA_MIN_BYTES || n.len() > RSA_MAX_BYTES || e.len() > 4 {
        return Err(E_RANGE);
    }
    let mut k = RsaPub {
        n: [0; RSA_MAX_BYTES],
        nlen: n.len() as u32,
        e: 0,
    };
    k.n[..n.len()].copy_from_slice(n);
    let mut exp = 0u32;
    for &byte in e {
        exp = (exp << 8) | u32::from(byte);
    }
    k.e = exp;
    let rc = rsa_key_check(&k);
    if rc != 0 {
        return Err(rc);
    }
    Ok(k)
}

fn key_parse(bits: &[u8]) -> Result<RsaPub, c_int> {
    if bits.is_empty() || bits[0] != 0 {
        return Err(E_INVAL);
    }
    let seq = der_only(&bits[1..], DER_SEQ)?;
    let (n, seq) = der_uint(seq)?;
    let (e, seq) = der_uint(seq)?;
    if !seq.is_empty() {
        return Err(E_INVAL);
    }
    key_fill(n, e)
}

pub(crate) fn parse(bytes: &[u8]) -> Result<RsaPub, c_int> {
    let body = der_only(bytes, DER_SEQ)?;
    let alg = der_take(body, DER_SEQ)?;
    alg_check(alg.val)?;
    let bits = der_take(alg.rest, DER_BITS)?;
    if !bits.rest.is_empty() {
        return Err(E_INVAL);
    }
    key_parse(bits.val)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn key_at(nlen: u32, lead: u8, last: u8, e: u32) -> RsaPub {
        let mut k = RsaPub {
            n: [0; RSA_MAX_BYTES],
            nlen,
            e,
        };
        if nlen >= 1 && (nlen as usize) <= RSA_MAX_BYTES {
            k.n[0] = lead;
            k.n[nlen as usize - 1] = last;
        }
        k
    }

    #[test]
    fn key_check_table() {
        assert_eq!(rsa_key_check(&key_at(255, 0x80, 0x01, 3)), E_RANGE);
        assert_eq!(rsa_key_check(&key_at(0, 0, 0, 3)), E_RANGE);
        assert_eq!(rsa_key_check(&key_at(513, 0x80, 0x01, 3)), E_RANGE);
        assert_eq!(rsa_key_check(&key_at(256, 0x00, 0x01, 3)), E_RANGE);
        assert_eq!(rsa_key_check(&key_at(256, 0x7f, 0x01, 3)), E_RANGE);
        assert_eq!(rsa_key_check(&key_at(256, 0x80, 0x02, 3)), E_INVAL);
        assert_eq!(rsa_key_check(&key_at(256, 0x80, 0x01, 1)), E_INVAL);
        assert_eq!(rsa_key_check(&key_at(256, 0x80, 0x01, 2)), E_INVAL);
        assert_eq!(rsa_key_check(&key_at(256, 0x80, 0x01, 0)), E_INVAL);
        assert_eq!(rsa_key_check(&key_at(256, 0x80, 0x01, 3)), 0);
        assert_eq!(rsa_key_check(&key_at(512, 0x01, 0x01, 65537)), 0);
        assert_eq!(rsa_key_check(&key_at(512, 0x00, 0x01, 3)), E_RANGE);
    }
}
