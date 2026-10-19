use crate::{E_INVAL, c_int};

pub(crate) const DER_INT: u8 = 0x02;
pub(crate) const DER_BITS: u8 = 0x03;
pub(crate) const DER_NULL: u8 = 0x05;
pub(crate) const DER_OID: u8 = 0x06;
pub(crate) const DER_SEQ: u8 = 0x30;
pub(crate) const DER_CTX0: u8 = 0xa0;

#[derive(Debug)]
pub(crate) struct Node<'a> {
    pub all: &'a [u8],
    pub val: &'a [u8],
    pub rest: &'a [u8],
    pub tag: u8,
}

fn der_len(input: &[u8]) -> Result<(usize, usize), c_int> {
    let mut hdr = 2usize;
    let mut len = usize::from(input[1]);
    if input[1] < 0x80 {
        return Ok((hdr, len));
    }
    let n = usize::from(input[1] & 0x7f);
    if n == 0 || n > 4 || input.len() < 2 + n || input[2] == 0 {
        return Err(E_INVAL);
    }
    len = 0;
    for i in 0..n {
        len = (len << 8) | usize::from(input[2 + i]);
    }
    if len < 0x80 {
        return Err(E_INVAL);
    }
    hdr = 2 + n;
    Ok((hdr, len))
}

pub(crate) fn der_read(input: &[u8]) -> Result<Node<'_>, c_int> {
    if input.len() < 2 || input[0] & 0x1f == 0x1f {
        return Err(E_INVAL);
    }
    let (hdr, len) = der_len(input)?;
    let Some(room) = input.len().checked_sub(hdr) else {
        return Err(E_INVAL);
    };
    if len > room {
        return Err(E_INVAL);
    }
    let end = hdr + len;
    Ok(Node {
        all: &input[..end],
        val: &input[hdr..end],
        rest: &input[end..],
        tag: input[0],
    })
}

pub(crate) fn der_take(input: &[u8], tag: u8) -> Result<Node<'_>, c_int> {
    let node = der_read(input)?;
    if node.tag != tag {
        return Err(E_INVAL);
    }
    Ok(node)
}

pub(crate) fn der_only(input: &[u8], tag: u8) -> Result<&[u8], c_int> {
    let node = der_take(input, tag)?;
    if !node.rest.is_empty() {
        return Err(E_INVAL);
    }
    Ok(node.val)
}

pub(crate) fn der_uint(input: &[u8]) -> Result<(&[u8], &[u8]), c_int> {
    let node = der_take(input, DER_INT)?;
    if node.val.is_empty() || node.val[0] & 0x80 != 0 {
        return Err(E_INVAL);
    }
    if node.val.len() > 1 && node.val[0] == 0 && node.val[1] & 0x80 == 0 {
        return Err(E_INVAL);
    }
    let mut mag = node.val;
    if mag.len() > 1 && mag[0] == 0 {
        mag = &mag[1..];
    }
    Ok((mag, node.rest))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn empty_sequence() {
        let node = der_read(&[0x30, 0x00]).unwrap();
        assert_eq!(node.tag, DER_SEQ);
        assert!(node.val.is_empty());
        assert!(node.rest.is_empty());
    }

    #[test]
    fn truncated_and_bad_lengths() {
        assert_eq!(der_read(&[]).unwrap_err(), E_INVAL);
        assert_eq!(der_read(&[0x30]).unwrap_err(), E_INVAL);
        assert_eq!(der_read(&[0x30, 0x01]).unwrap_err(), E_INVAL);
        assert_eq!(der_read(&[0x30, 0x80, 0x00, 0x00]).unwrap_err(), E_INVAL);
        assert_eq!(der_read(&[0x1f, 0x00]).unwrap_err(), E_INVAL);
        assert_eq!(der_read(&[0x30, 0x82, 0x00, 0x80]).unwrap_err(), E_INVAL);
        let mut short = vec![0x30, 0x81, 0x7f];
        short.extend(std::iter::repeat_n(0x01, 0x7f));
        assert_eq!(der_read(&short).unwrap_err(), E_INVAL);
    }

    #[test]
    fn long_form_minimum() {
        let mut body = vec![0x30, 0x81, 0x80];
        body.extend(std::iter::repeat_n(0x01, 0x80));
        let node = der_read(&body).unwrap();
        assert_eq!(node.val.len(), 0x80);
        assert!(node.rest.is_empty());
    }

    #[test]
    fn only_rejects_tail() {
        assert_eq!(der_only(&[0x30, 0x00, 0x00], DER_SEQ).unwrap_err(), E_INVAL);
    }

    #[test]
    fn integers() {
        assert_eq!(der_uint(&[0x02, 0x00]).unwrap_err(), E_INVAL);
        assert_eq!(der_uint(&[0x02, 0x01, 0x80]).unwrap_err(), E_INVAL);
        assert_eq!(der_uint(&[0x02, 0x02, 0x00, 0x01]).unwrap_err(), E_INVAL);
        let (mag, rest) = der_uint(&[0x02, 0x02, 0x00, 0x80, 0xff]).unwrap();
        assert_eq!(mag, &[0x80]);
        assert_eq!(rest, &[0xff]);
        let (mag, rest) = der_uint(&[0x02, 0x01, 0x00]).unwrap();
        assert_eq!(mag, &[0x00]);
        assert!(rest.is_empty());
    }
}
