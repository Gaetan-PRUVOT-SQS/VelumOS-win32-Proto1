#include <stdlib.h>
#include <string.h>
#include "velum/err.h"
#include "d05_vec.h"

const t_vec	*vec_find(const char *name)
{
	size_t	i;

	i = 0;
	while (i < g_vec_count)
	{
		if (!strcmp(g_vec[i].name, name))
			return (&g_vec[i]);
		i++;
	}
	return (NULL);
}

int	key_from(t_span src, int cert, t_rsapub *k)
{
	uint8_t	*copy;
	t_span	in;
	int		r;

	copy = malloc(src.len + (src.len == 0));
	if (!copy)
		return (E_NOMEM);
	memcpy(copy, src.p, src.len);
	in.p = copy;
	in.len = src.len;
	r = 0;
	if (cert)
		r = x509_spki(in, &in);
	if (r == 0)
		r = der_spki_rsa(in, k);
	free(copy);
	return (r);
}

int	sig_run(const t_rsapub *k, t_span sig, const uint8_t *digest)
{
	uint8_t	*copy;
	t_span	in;
	int		r;

	copy = malloc(sig.len + (sig.len == 0));
	if (!copy)
		return (E_NOMEM);
	memcpy(copy, sig.p, sig.len);
	in.p = copy;
	in.len = sig.len;
	r = rsa_pkcs1_sha256_verify(k, in, digest);
	free(copy);
	return (r);
}

int	vec_check(const t_vec *v)
{
	t_rsapub	k;
	t_span		s;
	int			r;

	s.p = v->key;
	s.len = v->key_len;
	r = key_from(s, v->flags & VEC_CERT, &k);
	if (!(v->flags & VEC_PARSE))
		return (r < 0);
	if (r != 0)
		return (0);
	s.p = v->sig;
	s.len = v->sig_len;
	r = sig_run(&k, s, v->digest);
	if (v->flags & VEC_OK)
		return (r == 0);
	return (r < 0);
}
