#include <string.h>
#include "harness.h"
#include "velum/crypto.h"
#include "d05_vec.h"

static uint32_t	g_seed = 0x0d05c0de;

static uint32_t	rnd(void)
{
	g_seed ^= g_seed << 13;
	g_seed ^= g_seed >> 17;
	g_seed ^= g_seed << 5;
	return (g_seed);
}

static void	truncation(const t_vec *v)
{
	t_rsapub	k;
	t_span		s;

	s.p = v->key;
	s.len = 0;
	while (s.len < v->key_len)
	{
		h_true(key_from(s, v->flags & VEC_CERT, &k) < 0, "troncature refusee");
		s.len++;
	}
	h_eq_i64("entier accepte", key_from(s, v->flags & VEC_CERT, &k), 0);
}

static int	mutate_once(const t_vec *v, const t_rsapub *ref, uint8_t *buf)
{
	t_rsapub	k;
	t_span		s;
	uint32_t	n;
	uint32_t	pos;

	memcpy(buf, v->key, v->key_len);
	n = 1 + rnd() % 3;
	while (n--)
	{
		pos = rnd() % v->key_len;
		buf[pos] ^= (uint8_t)(1 + rnd() % 255);
	}
	s.p = buf;
	s.len = v->key_len;
	if (key_from(s, v->flags & VEC_CERT, &k) < 0)
		return (1);
	s.p = v->sig;
	s.len = v->sig_len;
	if (sig_run(&k, s, v->digest) < 0)
		return (2);
	return (3 * (k.nlen == ref->nlen && k.e == ref->e
			&& ct_equal(k.n, ref->n, RSA_MAX_BYTES)));
}

static void	mutations(const t_vec *v, int count)
{
	uint8_t		buf[4096];
	t_rsapub	ref;
	t_span		s;
	int			seen[4];
	int			r;

	s.p = v->key;
	s.len = v->key_len;
	memset(seen, 0, sizeof(seen));
	if (v->key_len > sizeof(buf) || key_from(s, v->flags & VEC_CERT, &ref))
		count = 0;
	while (count-- > 0)
	{
		r = mutate_once(v, &ref, buf);
		seen[r]++;
		h_true(r != 0, "aucune signature fausse acceptee");
	}
	h_true(seen[1] > 0, "mutations refusees a l'analyse");
	h_true(seen[2] > 0, "mutations refusees a la verification");
}

int	main(void)
{
	const t_vec	*cert;
	const t_vec	*spki;

	h_begin("d05 troncature et mutations");
	cert = vec_find(VEC_BASE_CERT);
	spki = vec_find(VEC_BASE_SPKI);
	h_true(cert && spki, "vecteurs de base");
	if (!cert || !spki)
		return (h_end());
	truncation(cert);
	truncation(spki);
	mutations(cert, 5000);
	mutations(spki, 5000);
	return (h_end());
}
