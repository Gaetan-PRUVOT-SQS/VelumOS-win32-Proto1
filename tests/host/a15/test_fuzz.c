#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	fuzz_round(t_fx *dst, t_fx *src)
{
	fx_clip(dst);
	fuzz_op(dst, src, (int)(rng_next() % 15));
	fx_verify(dst, "fuzz: aucune ecriture hors du clip ni hors du tampon");
	if (src != dst)
		fx_verify(src, "fuzz: la surface source n'est pas modifiee");
}

static void	fuzz_fixture(int kind_a, int kind_b)
{
	t_fx	a;
	t_fx	b;
	int		i;

	fx_open(&a, kind_a);
	fx_open(&b, kind_b);
	i = 0;
	while (i < 100)
	{
		if (rng_next() % 4 == 0)
			fuzz_round(&a, &a);
		else
			fuzz_round(&a, &b);
		i++;
	}
	fx_close(&b);
	fx_close(&a);
}

int	main(void)
{
	uint64_t	seed;
	uint64_t	total;
	uint64_t	i;

	seed = 20261005;
	total = 1000000;
	if (getenv("A15_SEED") != NULL)
		seed = strtoull(getenv("A15_SEED"), NULL, 10);
	if (getenv("A15_FUZZ_N") != NULL)
		total = strtoull(getenv("A15_FUZZ_N"), NULL, 10);
	h_begin("a15/fuzz");
	rng_seed(seed);
	printf("a15/fuzz : graine %llu, %llu appels\n", (unsigned long long)seed,
		(unsigned long long)total);
	i = 0;
	while (i < total / 100)
	{
		fuzz_fixture((int)(i % FX_KINDS), (int)(rng_next() % FX_KINDS));
		i++;
	}
	return (h_end());
}
