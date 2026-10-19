use crate::c_int;
use crate::der::{DER_BITS, DER_CTX0, DER_INT, DER_SEQ, der_only, der_read, der_take};

fn skip(input: &mut &[u8], tag: u8) -> Result<(), c_int> {
    let node = der_take(input, tag)?;
    *input = node.rest;
    Ok(())
}

fn tbs_spki(mut tbs: &[u8]) -> Result<&[u8], c_int> {
    let first = der_read(tbs)?;
    if first.tag == DER_CTX0 {
        tbs = first.rest;
    }
    skip(&mut tbs, DER_INT)?;
    skip(&mut tbs, DER_SEQ)?;
    skip(&mut tbs, DER_SEQ)?;
    skip(&mut tbs, DER_SEQ)?;
    skip(&mut tbs, DER_SEQ)?;
    let spki = der_take(tbs, DER_SEQ)?;
    Ok(spki.all)
}

pub(crate) fn extract(cert: &[u8]) -> Result<&[u8], c_int> {
    let body = der_only(cert, DER_SEQ)?;
    let tbs = der_take(body, DER_SEQ)?;
    let mut rest = tbs.rest;
    skip(&mut rest, DER_SEQ)?;
    skip(&mut rest, DER_BITS)?;
    if !rest.is_empty() {
        return Err(crate::E_INVAL);
    }
    tbs_spki(tbs.val)
}
