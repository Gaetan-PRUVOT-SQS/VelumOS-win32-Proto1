#include <stdlib.h>
#include "harness.h"
#include "d01.h"

static void	troncature(void)
{
	t_blob	pack;
	t_rec	r;
	size_t	off;
	size_t	k;
	uint8_t	*in;

	pack = d01_load("deflate.pack");
	off = 0;
	while (d01_next(&pack, &off, &r))
	{
		k = 0;
		while (r.alen <= 2048 && k < r.alen)
		{
			in = d01_dup(r.a, k);
			h_true(inflate_raw(d01_span(in, k), pack.p, 0) < 0, "tronque");
			free(in);
			k++;
		}
	}
	free(pack.p);
}

static int	mutation(const t_rec *r, uint32_t *seed)
{
	uint8_t	*in;
	uint8_t	*out;
	int64_t	n;

	in = d01_dup(r->a, r->alen);
	out = malloc(r->blen);
	d01_mutate(in, r->alen, seed);
	n = inflate_raw(d01_span(in, r->alen), out, r->blen);
	free(in);
	free(out);
	return (n <= (int64_t)r->blen);
}

static void	mutations(void)
{
	t_blob		pack;
	t_rec		r;
	size_t		off;
	uint32_t	seed;
	int			i;

	pack = d01_load("deflate.pack");
	off = 0;
	seed = 0x1d01;
	i = 0;
	while (i < 12000)
	{
		if (!d01_next(&pack, &off, &r))
			off = 0;
		else if (r.alen > 0 && r.alen <= 4096)
		{
			h_true(mutation(&r, &seed), "mutation DEFLATE");
			i++;
		}
	}
	free(pack.p);
}

int	main(void)
{
	h_begin("d01 inflate fuzz");
	h_run("troncature_chaque_octet", troncature);
	h_run("mutations_graine_fixe", mutations);
	return (h_end());
}
