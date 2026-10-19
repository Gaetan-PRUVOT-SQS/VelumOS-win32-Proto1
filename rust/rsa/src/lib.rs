#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod bn;
mod der;
mod spki;
mod verify;
mod x509;

pub(crate) use core::ffi::c_int;

pub const RSA_MIN_BITS: u32 = 2048;
pub const RSA_MAX_BITS: u32 = 4096;
pub const RSA_MIN_BYTES: usize = 256;
pub const RSA_MAX_BYTES: usize = 512;
pub const RSA_WORDS: usize = 128;
pub const RSA_MIN_EXP: u32 = 3;
pub const RSA_DIGEST_LEN: usize = 32;

pub const E_INVAL: c_int = -22;
pub const E_RANGE: c_int = -34;
pub const E_NOTSUP: c_int = -95;

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Span {
    pub p: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct RsaPub {
    pub n: [u8; RSA_MAX_BYTES],
    pub nlen: u32,
    pub e: u32,
}

const _: () = assert!(core::mem::size_of::<Span>() == 16);
const _: () = assert!(core::mem::align_of::<Span>() == 8);
const _: () = assert!(core::mem::offset_of!(Span, len) == 8);
const _: () = assert!(core::mem::size_of::<RsaPub>() == 520);
const _: () = assert!(core::mem::align_of::<RsaPub>() == 4);
const _: () = assert!(core::mem::offset_of!(RsaPub, nlen) == 512);
const _: () = assert!(core::mem::offset_of!(RsaPub, e) == 516);
const _: () = assert!(RSA_MAX_BYTES / 4 == RSA_WORDS);
const _: () = assert!(RSA_MIN_BITS / 8 == RSA_MIN_BYTES as u32);
const _: () = assert!(RSA_MAX_BITS / 8 == RSA_MAX_BYTES as u32);

/// # Safety
/// `p` est non nul, aligné pour `u8`, et valide pour `len` octets pendant `'a`.
unsafe fn bytes<'a>(p: *const u8, len: usize) -> &'a [u8] {
    // SAFETY: p non nul et len borné par l'appelant, qui garantit les octets.
    unsafe { core::slice::from_raw_parts(p, len) }
}

/// # Safety
/// `cert.p` est nul, ou valide pour `cert.len` octets le temps de l'appel.
/// `spki` est non nul, aligné, et désigne un `Span` inscriptible qui n'alias pas `cert`.
/// Succès : `spki.p` vise des octets de `cert`, qui doit leur survivre.
/// Échec ou pointeur nul : `E_INVAL`, et `spki` n'est pas écrit.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn x509_spki(cert: Span, spki: *mut Span) -> c_int {
    if spki.is_null() {
        return E_INVAL;
    }
    if cert.p.is_null() || cert.len > isize::MAX as usize {
        return E_INVAL;
    }
    let found = {
        // SAFETY: cert.p non nul, cert.len tient dans isize, octets valides pour l'appel.
        let input = unsafe { bytes(cert.p, cert.len) };
        match x509::extract(input) {
            Ok(part) => Span {
                p: part.as_ptr(),
                len: part.len(),
            },
            Err(code) => return code,
        }
    };
    // SAFETY: spki non nul et aligné ; write n'évalue pas l'ancien Span.
    unsafe { spki.write(found) };
    0
}

/// # Safety
/// `spki.p` est nul, ou valide pour `spki.len` octets le temps de l'appel.
/// `out` est non nul, aligné, et désigne un `RsaPub` inscriptible.
/// L'échec ne modifie pas `out`. La clé copiée ne dépend plus de `spki`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn der_spki_rsa(spki: Span, out: *mut RsaPub) -> c_int {
    if out.is_null() {
        return E_INVAL;
    }
    if spki.p.is_null() || spki.len > isize::MAX as usize {
        return E_INVAL;
    }
    let key = {
        // SAFETY: spki.p non nul, spki.len tient dans isize, octets valides pour l'appel.
        let input = unsafe { bytes(spki.p, spki.len) };
        match crate::spki::parse(input) {
            Ok(key) => key,
            Err(code) => return code,
        }
    };
    // SAFETY: out non nul et aligné ; write n'évalue pas l'ancien RsaPub.
    unsafe { out.write(key) };
    0
}

/// # Safety
/// `k` est nul, ou aligné et initialisé le temps de l'appel.
/// `sig.p` est nul, ou valide pour `sig.len` octets si la longueur égale `k.nlen`.
/// `digest` est nul, ou valide pour 32 octets.
/// Aucune écriture. Un pointeur nul renvoie `E_INVAL`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn rsa_pkcs1_sha256_verify(
    k: *const RsaPub,
    sig: Span,
    digest: *const u8,
) -> c_int {
    if k.is_null() || sig.p.is_null() || digest.is_null() {
        return E_INVAL;
    }
    // SAFETY: k non nul, aligné, initialisé pour toute la durée de l'appel.
    let key = unsafe { &*k };
    let rc = spki::rsa_key_check(key);
    if rc < 0 {
        return rc;
    }
    if sig.len != key.nlen as usize || sig.len > isize::MAX as usize {
        return E_INVAL;
    }
    // SAFETY: sig.p non nul et sig.len égal à nlen, donc au plus RSA_MAX_BYTES.
    let sig_bytes = unsafe { bytes(sig.p, sig.len) };
    // SAFETY: digest non nul et valide pour RSA_DIGEST_LEN octets.
    let digest_bytes = unsafe { bytes(digest, RSA_DIGEST_LEN) };
    verify::pkcs1_sha256(key, sig_bytes, digest_bytes)
}

#[cfg(test)]
#[allow(clippy::undocumented_unsafe_blocks)]
mod tests {
    use super::*;

    const OID: &[u8] = &[0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01];
    const SPKI_N3_E3: &[u8] = &[
        0x30, 0x1a, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
        0x05, 0x00, 0x03, 0x09, 0x00, 0x30, 0x06, 0x02, 0x01, 0x03, 0x02, 0x01, 0x03,
    ];
    const CERT_N3: &[u8] = &[
        0x30, 0x2e, 0x30, 0x27, 0x02, 0x01, 0x01, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
        0x30, 0x1a, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
        0x05, 0x00, 0x03, 0x09, 0x00, 0x30, 0x06, 0x02, 0x01, 0x03, 0x02, 0x01, 0x03, 0x30, 0x00,
        0x03, 0x01, 0x00,
    ];
    const CERT_N3_VERSION: &[u8] = &[
        0x30, 0x33, 0x30, 0x2c, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x00, 0x30,
        0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x1a, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
        0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x09, 0x00, 0x30, 0x06, 0x02, 0x01, 0x03,
        0x02, 0x01, 0x03, 0x30, 0x00, 0x03, 0x01, 0x00,
    ];
    const CERT_N3_EXT: &[u8] = &[
        0x30, 0x30, 0x30, 0x29, 0x02, 0x01, 0x01, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
        0x30, 0x1a, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
        0x05, 0x00, 0x03, 0x09, 0x00, 0x30, 0x06, 0x02, 0x01, 0x03, 0x02, 0x01, 0x03, 0x30, 0x00,
        0x30, 0x00, 0x03, 0x01, 0x00,
    ];
    const CERT_N3_SHORT: &[u8] = &[
        0x30, 0x2c, 0x30, 0x25, 0x02, 0x01, 0x01, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x1a,
        0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00,
        0x03, 0x09, 0x00, 0x30, 0x06, 0x02, 0x01, 0x03, 0x02, 0x01, 0x03, 0x30, 0x00, 0x03, 0x01,
        0x00,
    ];

    fn tlv(tag: u8, body: &[u8]) -> Vec<u8> {
        let mut out = Vec::with_capacity(body.len() + 6);
        out.push(tag);
        let size = body.len();
        if size < 0x80 {
            out.push(size as u8);
        } else {
            let raw = size.to_be_bytes();
            let start = raw.iter().position(|b| *b != 0).unwrap();
            let raw = &raw[start..];
            out.push(0x80 | raw.len() as u8);
            out.extend_from_slice(raw);
        }
        out.extend_from_slice(body);
        out
    }

    fn der_int(mag: &[u8]) -> Vec<u8> {
        if mag.is_empty() || mag[0] & 0x80 != 0 {
            let mut body = vec![0x00];
            body.extend_from_slice(mag);
            tlv(0x02, &body)
        } else {
            tlv(0x02, mag)
        }
    }

    fn spki_of(n: &[u8], e: &[u8], oid: &[u8], unused: u8, params: &[u8]) -> Vec<u8> {
        let mut alg_body = tlv(0x06, oid);
        alg_body.extend_from_slice(params);
        let alg = tlv(0x30, &alg_body);
        let mut key_body = der_int(n);
        key_body.extend_from_slice(&der_int(e));
        let key = tlv(0x30, &key_body);
        let mut bits = vec![unused];
        bits.extend_from_slice(&key);
        let mut body = alg;
        body.extend_from_slice(&tlv(0x03, &bits));
        tlv(0x30, &body)
    }

    fn cert_around(spki: &[u8], version: bool) -> Vec<u8> {
        let mut tbs = Vec::new();
        if version {
            tbs.extend_from_slice(&[0xa0, 0x03, 0x02, 0x01, 0x02]);
        }
        tbs.extend_from_slice(&[
            0x02, 0x01, 0x01, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
        ]);
        tbs.extend_from_slice(spki);
        let tbs = tlv(0x30, &tbs);
        let mut body = tbs;
        body.extend_from_slice(&[0x30, 0x00, 0x03, 0x01, 0x00]);
        tlv(0x30, &body)
    }

    fn span_of(buf: &[u8]) -> Span {
        Span {
            p: buf.as_ptr(),
            len: buf.len(),
        }
    }

    fn key_ok() -> RsaPub {
        let mut k = RsaPub {
            n: [0; RSA_MAX_BYTES],
            nlen: 256,
            e: 3,
        };
        k.n[0] = 0x80;
        k.n[255] = 0x01;
        k
    }

    fn verify(k: *const RsaPub, sig: &[u8], digest: *const u8) -> c_int {
        unsafe { rsa_pkcs1_sha256_verify(k, span_of(sig), digest) }
    }

    #[test]
    fn layout_matches_c_abi() {
        assert_eq!(core::mem::size_of::<Span>(), 16);
        assert_eq!(core::mem::size_of::<RsaPub>(), 520);
        assert_eq!(E_INVAL, -22);
        assert_eq!(E_RANGE, -34);
        assert_eq!(E_NOTSUP, -95);
    }

    #[test]
    fn hand_der_matches_builder() {
        let built = spki_of(&[0x03], &[0x03], OID, 0, &[0x05, 0x00]);
        assert_eq!(built, SPKI_N3_E3);
        assert_eq!(cert_around(SPKI_N3_E3, false), CERT_N3);
        assert_eq!(cert_around(SPKI_N3_E3, true), CERT_N3_VERSION);
    }

    #[test]
    fn null_pointers_are_einval() {
        let key = key_ok();
        let digest = [0u8; 32];
        let sig = [0u8; 1];
        assert_eq!(verify(core::ptr::null(), &sig, digest.as_ptr()), E_INVAL);
        assert_eq!(
            unsafe {
                rsa_pkcs1_sha256_verify(
                    &key,
                    Span {
                        p: core::ptr::null(),
                        len: 0,
                    },
                    digest.as_ptr(),
                )
            },
            E_INVAL
        );
        assert_eq!(verify(&key, &sig, core::ptr::null()), E_INVAL);
        assert_eq!(
            unsafe { x509_spki(span_of(CERT_N3), core::ptr::null_mut()) },
            E_INVAL
        );
        assert_eq!(
            unsafe { der_spki_rsa(span_of(SPKI_N3_E3), core::ptr::null_mut()) },
            E_INVAL
        );
        assert_eq!(
            unsafe {
                der_spki_rsa(
                    Span {
                        p: core::ptr::null(),
                        len: 4,
                    },
                    &mut key_ok(),
                )
            },
            E_INVAL
        );
    }

    #[test]
    fn short_signature_and_bad_exponent_and_short_modulus() {
        let key = key_ok();
        let digest = [0u8; 32];
        let short = [0u8; 255];
        assert_eq!(verify(&key, &short, digest.as_ptr()), E_INVAL);
        assert_eq!(verify(&key, &[], digest.as_ptr()), E_INVAL);
        let mut bad = key.clone();
        bad.e = 1;
        let sig = [0u8; 256];
        assert_eq!(verify(&bad, &sig, digest.as_ptr()), E_INVAL);
        bad = key.clone();
        bad.nlen = 255;
        assert_eq!(verify(&bad, &sig, digest.as_ptr()), E_RANGE);
        bad.nlen = 0;
        assert_eq!(verify(&bad, &short, digest.as_ptr()), E_RANGE);
        bad = key;
        bad.nlen = 513;
        assert_eq!(verify(&bad, &short, digest.as_ptr()), E_RANGE);
    }

    #[test]
    fn parse_refuses_and_keeps_output() {
        let mut out = RsaPub {
            n: [0xA5; RSA_MAX_BYTES],
            nlen: 0xA5A5_A5A5,
            e: 0x5A5A_5A5A,
        };
        let before = out.clone();
        let rc = unsafe { der_spki_rsa(span_of(&[0x00]), &mut out) };
        assert_eq!(rc, E_INVAL);
        assert_eq!(out, before);
        assert_eq!(
            unsafe { der_spki_rsa(span_of(SPKI_N3_E3), &mut out) },
            E_RANGE
        );
        assert_eq!(out, before);
        let mut oid = OID.to_vec();
        oid[8] = 0x0a;
        let pss = spki_of(&[0x03], &[0x03], &oid, 0, &[0x05, 0x00]);
        assert_eq!(unsafe { der_spki_rsa(span_of(&pss), &mut out) }, E_NOTSUP);
        assert_eq!(out, before);
        let unused = spki_of(&[0x03], &[0x03], OID, 1, &[0x05, 0x00]);
        assert_eq!(unsafe { der_spki_rsa(span_of(&unused), &mut out) }, E_INVAL);
        let even_e = spki_of(&modulus_256(), &[0x04], OID, 0, &[0x05, 0x00]);
        assert_eq!(unsafe { der_spki_rsa(span_of(&even_e), &mut out) }, E_INVAL);
        let wide_e = spki_of(
            &modulus_256(),
            &[0x01, 0x00, 0x00, 0x00, 0x01],
            OID,
            0,
            &[0x05, 0x00],
        );
        assert_eq!(unsafe { der_spki_rsa(span_of(&wide_e), &mut out) }, E_RANGE);
    }

    fn modulus_256() -> Vec<u8> {
        let mut n = vec![0x80];
        n.extend(std::iter::repeat_n(0, 254));
        n.push(0x01);
        n
    }

    #[test]
    fn parses_minimum_modulus() {
        let n = modulus_256();
        let spki = spki_of(&n, &[0x01, 0x00, 0x01], OID, 0, &[0x05, 0x00]);
        let mut out = RsaPub {
            n: [0; RSA_MAX_BYTES],
            nlen: 0,
            e: 0,
        };
        assert_eq!(unsafe { der_spki_rsa(span_of(&spki), &mut out) }, 0);
        assert_eq!(out.nlen, 256);
        assert_eq!(out.e, 65537);
        assert_eq!(out.n[0], 0x80);
        assert_eq!(out.n[1], 0x00);
        assert_eq!(out.n[255], 0x01);
        assert!(out.n[256..].iter().all(|b| *b == 0));
    }

    #[test]
    fn x509_extracts_spki_and_refuses_tail() {
        let mut found = Span {
            p: core::ptr::null(),
            len: usize::MAX,
        };
        assert_eq!(unsafe { x509_spki(span_of(CERT_N3), &mut found) }, 0);
        let got = unsafe { core::slice::from_raw_parts(found.p, found.len) };
        assert_eq!(got, SPKI_N3_E3);
        assert_eq!(
            unsafe { x509_spki(span_of(CERT_N3_VERSION), &mut found) },
            0
        );
        let got = unsafe { core::slice::from_raw_parts(found.p, found.len) };
        assert_eq!(got, SPKI_N3_E3);
        assert_eq!(unsafe { x509_spki(span_of(CERT_N3_EXT), &mut found) }, 0);
        let got = unsafe { core::slice::from_raw_parts(found.p, found.len) };
        assert_eq!(got, SPKI_N3_E3);
        let mut trailed = CERT_N3.to_vec();
        trailed.push(0x00);
        found.len = 7;
        assert_eq!(unsafe { x509_spki(span_of(&trailed), &mut found) }, E_INVAL);
        assert_eq!(found.len, 7);
        assert_eq!(
            unsafe { x509_spki(span_of(SPKI_N3_E3), &mut found) },
            E_INVAL
        );
        assert_eq!(found.len, 7);
        assert_eq!(
            unsafe { x509_spki(span_of(CERT_N3_SHORT), &mut found) },
            E_INVAL
        );
        assert_eq!(found.len, 7);
        assert_eq!(
            unsafe {
                x509_spki(
                    Span {
                        p: core::ptr::null(),
                        len: 8,
                    },
                    &mut found,
                )
            },
            E_INVAL
        );
        assert_eq!(found.len, 7);
    }

    #[test]
    fn wrong_signature_and_modulus_are_rejected() {
        let key = key_ok();
        let digest = [0x22u8; 32];
        let mut sig = [0u8; 256];
        sig.copy_from_slice(&key.n[..256]);
        assert_eq!(verify(&key, &sig, digest.as_ptr()), E_INVAL);
        sig = [0u8; 256];
        sig[255] = 1;
        assert_eq!(verify(&key, &sig, digest.as_ptr()), E_INVAL);
    }
}
