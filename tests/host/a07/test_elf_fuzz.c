#include <stdio.h>
#include <stdlib.h>
#include "a07_fake.h"
#include "harness.h"
#include "velum/vmm.h"

static void	fz_one(t_gen *g)
{
	t_elfinfo	in;
	int			rc;

	rc = gen_check(g, &in);
	if (rc < 0)
	{
		g_fz.rejected++;
		return ;
	}
	g_fz.accepted++;
	h_true(fz_invariants(&in, g->size), "invariants d'un ELF accepte");
}

static void	fz_mutate(void)
{
	t_gen		g;
	uint64_t	r;
	int			i;
	int			k;

	i = 0;
	while (i < 6000)
	{
		gen_valid(&g, (uint16_t)(ELF_ET_EXEC + (i & 1)));
		k = 1 + (int)(fz_next() % 8);
		while (k-- > 0)
		{
			r = fz_next();
			if (r & 1)
				g.buf[(r >> 8) % 0x180] ^= (uint8_t)(1u << ((r >> 4) & 7));
			else if (r & 2)
				gen_patch(&g, (uint32_t)((r >> 8) % 0x180) & ~7u, 8,
					fz_next() >> (fz_next() % 64));
			else
				g.buf[0x2000 + (r >> 8) % 0x140] ^= (uint8_t)(r >> 32);
		}
		fz_one(&g);
		i++;
	}
}

static void	fz_truncate(void)
{
	t_gen		g;
	t_elfinfo	in;
	uint64_t	n;
	int			bad;

	bad = 0;
	n = 0;
	while (n < GEN_SIZE)
	{
		gen_valid(&g, ELF_ET_DYN);
		g.size = n;
		if (gen_check(&g, &in) == 0)
			bad++;
		n++;
	}
	h_eq_i64("toute troncature refusee", bad, 0);
}

int	main(void)
{
	const char	*env;

	env = getenv("A07_SEED");
	if (env)
		g_fz.rng = strtoull(env, NULL, 0) | 1;
	printf("a07/elf_fuzz : graine %#llx\n", (unsigned long long)g_fz.rng);
	h_begin("a07/elf_fuzz");
	h_run("mutations", fz_mutate);
	h_run("troncatures", fz_truncate);
	printf("a07/elf_fuzz : %d refusees, %d acceptees\n", g_fz.rejected,
		g_fz.accepted);
	h_true(g_fz.rejected >= 200, "au moins 200 variantes refusees");
	return (h_end());
}
