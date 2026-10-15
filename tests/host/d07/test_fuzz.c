#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "d07.h"

static int	mutate_once(t_span f, uint32_t *seed, size_t *pos)
{
	uint8_t			*p;
	uint8_t			old;
	int				r;

	p = (uint8_t *)(uintptr_t)f.p;
	*pos = d07_rand(seed) % f.len;
	old = p[*pos];
	p[*pos] ^= (uint8_t)(1 + d07_rand(seed) % 255);
	r = d07_try(f);
	p[*pos] = old;
	return (r);
}

static void	fuzz_mutations(void)
{
	t_span		f;
	size_t		b[3];
	uint32_t	seed;
	uint32_t	n[3];

	f = d07_load("ok.apk");
	d07_block_bounds(f, &b[0], &b[1]);
	h_true(b[0] > 0 && b[0] < b[1], "bloc de signature trouve");
	seed = 0x2545f491;
	memset(n, 0, sizeof(n));
	while (b[1] && n[0] < D07_MUTATIONS)
	{
		if (mutate_once(f, &seed, &b[2]) == 0)
		{
			n[1] += (b[2] < b[0] || b[2] >= b[1]);
			n[2] += (b[2] >= b[0] && b[2] < b[1]);
		}
		n[0]++;
	}
	h_eq_u64("mutations jouees", n[0], D07_MUTATIONS);
	h_eq_u64("aucune mutation hors du bloc acceptee", n[1], 0);
	h_eq_u64("aucune mutation du bloc acceptee", n[2], 0);
	d07_free(f);
}

static void	fuzz_troncature(void)
{
	t_span		f;
	uint8_t		*cut;
	size_t		n;
	uint32_t	accepted;

	f = d07_load("ok.apk");
	accepted = 0;
	n = 0;
	while (f.p && n < f.len)
	{
		cut = malloc(n + 1);
		memcpy(cut, f.p, n);
		accepted += (d07_try((t_span){cut, n}) == 0);
		free(cut);
		n++;
	}
	h_true(f.len > 1000, "fichier charge");
	h_eq_u64("aucune troncature acceptee", accepted, 0);
	d07_free(f);
}

int	main(void)
{
	h_begin("d07/fuzz");
	h_run("fuzz_mutations", fuzz_mutations);
	h_run("fuzz_troncature", fuzz_troncature);
	return (h_end());
}
