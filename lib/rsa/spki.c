#include "rsa_int.h"

static const uint8_t	g_rsa_oid[9] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
	0x01, 0x01, 0x01};

int	rsa_key_check(const t_rsapub *k)
{
	if (!k)
		return (E_INVAL);
	if (k->nlen < RSA_MIN_BYTES || k->nlen > RSA_MAX_BYTES)
		return (E_RANGE);
	if (k->n[0] == 0 || (k->nlen == RSA_MIN_BYTES && !(k->n[0] & 0x80)))
		return (E_RANGE);
	if (!(k->n[k->nlen - 1] & 1) || !(k->e & 1) || k->e < RSA_MIN_EXP)
		return (E_INVAL);
	return (0);
}

static int	alg_check(t_span alg)
{
	t_der	oid;
	t_der	par;
	size_t	i;

	if (der_take(alg, DER_OID, &oid) < 0)
		return (E_INVAL);
	if (oid.val.len != sizeof(g_rsa_oid))
		return (E_NOTSUP);
	i = 0;
	while (i < sizeof(g_rsa_oid))
	{
		if (oid.val.p[i] != g_rsa_oid[i])
			return (E_NOTSUP);
		i++;
	}
	if (der_take(oid.rest, DER_NULL, &par) < 0)
		return (E_INVAL);
	if (par.val.len != 0 || par.rest.len != 0)
		return (E_INVAL);
	return (0);
}

static int	key_fill(t_span n, t_span e, t_rsapub *out)
{
	t_rsapub	k;
	size_t		i;

	if (n.len < RSA_MIN_BYTES || n.len > RSA_MAX_BYTES || e.len > 4)
		return (E_RANGE);
	i = 0;
	while (i < RSA_MAX_BYTES)
		k.n[i++] = 0;
	i = 0;
	while (i < n.len)
	{
		k.n[i] = n.p[i];
		i++;
	}
	k.nlen = (uint32_t)n.len;
	k.e = 0;
	i = 0;
	while (i < e.len)
		k.e = (k.e << 8) | e.p[i++];
	i = (size_t)rsa_key_check(&k);
	if (i == 0)
		*out = k;
	return ((int)i);
}

static int	key_parse(t_span bits, t_rsapub *out)
{
	t_span	seq;
	t_span	n;
	t_span	e;

	if (bits.len < 1 || bits.p[0] != 0)
		return (E_INVAL);
	bits.p++;
	bits.len--;
	if (der_only(bits, DER_SEQ, &seq) < 0)
		return (E_INVAL);
	if (der_uint(seq, &n, &seq) < 0 || der_uint(seq, &e, &seq) < 0)
		return (E_INVAL);
	if (seq.len != 0)
		return (E_INVAL);
	return (key_fill(n, e, out));
}

int	der_spki_rsa(t_span spki, t_rsapub *out)
{
	t_span	body;
	t_der	alg;
	t_der	bits;
	int		r;

	if (!out || der_only(spki, DER_SEQ, &body) < 0)
		return (E_INVAL);
	if (der_take(body, DER_SEQ, &alg) < 0)
		return (E_INVAL);
	r = alg_check(alg.val);
	if (r < 0)
		return (r);
	if (der_take(alg.rest, DER_BITS, &bits) < 0 || bits.rest.len != 0)
		return (E_INVAL);
	return (key_parse(bits.val, out));
}
