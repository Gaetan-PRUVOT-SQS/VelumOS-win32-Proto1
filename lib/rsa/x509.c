#include "rsa_int.h"

static int	skip(t_span *in, uint8_t tag)
{
	t_der	d;

	if (der_take(*in, tag, &d) < 0)
		return (E_INVAL);
	*in = d.rest;
	return (0);
}

static int	tbs_spki(t_span tbs, t_span *spki)
{
	t_der	d;

	if (der_read(tbs, &d) < 0)
		return (E_INVAL);
	if (d.tag == DER_CTX0)
		tbs = d.rest;
	if (skip(&tbs, DER_INT) < 0 || skip(&tbs, DER_SEQ) < 0)
		return (E_INVAL);
	if (skip(&tbs, DER_SEQ) < 0 || skip(&tbs, DER_SEQ) < 0)
		return (E_INVAL);
	if (skip(&tbs, DER_SEQ) < 0 || der_take(tbs, DER_SEQ, &d) < 0)
		return (E_INVAL);
	*spki = d.all;
	return (0);
}

int	x509_spki(t_span cert, t_span *spki)
{
	t_span	body;
	t_der	tbs;

	if (!spki || der_only(cert, DER_SEQ, &body) < 0)
		return (E_INVAL);
	if (der_take(body, DER_SEQ, &tbs) < 0)
		return (E_INVAL);
	body = tbs.rest;
	if (skip(&body, DER_SEQ) < 0 || skip(&body, DER_BITS) < 0)
		return (E_INVAL);
	if (body.len != 0)
		return (E_INVAL);
	return (tbs_spki(tbs.val, spki));
}
