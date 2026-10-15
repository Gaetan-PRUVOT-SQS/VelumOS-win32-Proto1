#include <stdlib.h>
#include "harness.h"
#include "d01.h"

static void	flux_reference(void)
{
	t_blob	pack;
	t_rec	r;
	size_t	off;
	uint8_t	*in;
	uint8_t	*out;

	pack = d01_load("deflate.pack");
	off = 0;
	while (d01_next(&pack, &off, &r))
	{
		in = d01_dup(r.a, r.alen);
		out = malloc(r.blen);
		h_eq_i64("taille", inflate_raw(d01_span(in, r.alen), out, r.blen),
			(int64_t)r.blen);
		h_true(d01_same(out, r.b, r.blen), "contenu");
		free(in);
		free(out);
	}
	free(pack.p);
}

static void	tampon_trop_petit(void)
{
	t_blob	pack;
	t_rec	r;
	size_t	off;
	uint8_t	*in;
	uint8_t	*out;

	pack = d01_load("deflate.pack");
	off = 0;
	while (d01_next(&pack, &off, &r))
	{
		if (r.blen == 0)
			continue ;
		in = d01_dup(r.a, r.alen);
		out = malloc(r.blen - 1);
		h_eq_i64("cap - 1", inflate_raw(d01_span(in, r.alen), out,
				r.blen - 1), E_OVERFLOW);
		free(in);
		free(out);
	}
	free(pack.p);
}

static void	flux_refuses(void)
{
	t_blob	pack;
	t_rec	r;
	size_t	off;
	uint8_t	*in;
	uint8_t	out[64];

	pack = d01_load("mauvais.pack");
	off = 0;
	while (d01_next(&pack, &off, &r))
	{
		in = d01_dup(r.a, r.alen);
		h_eq_i64("flux refuse", inflate_raw(d01_span(in, r.alen), out, 64),
			r.code);
		free(in);
	}
	free(pack.p);
}

int	main(void)
{
	h_begin("d01 inflate");
	h_run("flux_reference_niveaux_et_blocs", flux_reference);
	h_run("tampon_trop_petit", tampon_trop_petit);
	h_run("flux_refuses", flux_refuses);
	return (h_end());
}
