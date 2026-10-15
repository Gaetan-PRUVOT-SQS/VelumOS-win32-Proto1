#include "rsa_int.h"

static const uint8_t	g_di[19] = {0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60,
	0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};

static void	emsa_build(uint8_t *em, size_t k, const uint8_t *digest)
{
	size_t	i;
	size_t	ps;

	ps = k - 3 - sizeof(g_di) - RSA_DIGEST_LEN;
	em[0] = 0x00;
	em[1] = 0x01;
	i = 0;
	while (i < ps)
		em[2 + i++] = 0xff;
	em[2 + ps] = 0x00;
	i = 0;
	while (i < sizeof(g_di))
	{
		em[3 + ps + i] = g_di[i];
		i++;
	}
	i = 0;
	while (i < RSA_DIGEST_LEN)
	{
		em[3 + ps + sizeof(g_di) + i] = digest[i];
		i++;
	}
}

static int	rsa_public(const t_rsapub *k, t_span sig, uint8_t *out)
{
	t_mod	m;
	t_bn	s;

	bn_from_be(&m.n, k->n, k->nlen);
	m.nw = (k->nlen + 3) / 4;
	bn_from_be(&s, sig.p, sig.len);
	if (bn_cmp(&s, &m.n, m.nw) >= 0)
		return (E_INVAL);
	bn_modexp(&s, &s, k->e, &m);
	bn_to_be(&s, out, k->nlen);
	return (0);
}

static int	block_differs(const uint8_t *a, const uint8_t *b, size_t n)
{
	uint32_t	diff;
	size_t		i;

	diff = 0;
	i = 0;
	while (i < n)
	{
		diff |= (uint32_t)(a[i] ^ b[i]);
		i++;
	}
	return (diff != 0);
}

int	rsa_pkcs1_sha256_verify(const t_rsapub *k, t_span sig,
		const uint8_t digest[32])
{
	uint8_t	want[RSA_MAX_BYTES];
	uint8_t	got[RSA_MAX_BYTES];
	int		r;

	if (!k || !sig.p || !digest)
		return (E_INVAL);
	r = rsa_key_check(k);
	if (r < 0)
		return (r);
	if (sig.len != k->nlen)
		return (E_INVAL);
	if (rsa_public(k, sig, got) < 0)
		return (E_INVAL);
	emsa_build(want, k->nlen, digest);
	if (block_differs(want, got, k->nlen))
		return (E_INVAL);
	return (0);
}
