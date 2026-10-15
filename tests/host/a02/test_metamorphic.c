#include <stdio.h>
#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 4000

static void	meta_roundtrip(uint64_t seed)
{
	t_farg		*blk;
	t_snap		snap;
	uint64_t	n;

	h_eq_i64("boot", fake_simple(1, 33), 0);
	fake_seed(seed);
	printf("metamorphique : graine = %#llx\n",
		(unsigned long long)fake_seed_value());
	fake_snap_take(&snap);
	blk = malloc(CAP * sizeof(t_farg));
	n = fake_random_fill(blk, CAP);
	h_true(n > 100, "au moins 100 blocs alloues");
	h_true(!fake_snap_same(&snap), "l'etat a bien change");
	fake_shuffle_free(blk, n);
	h_true(fake_snap_same(&snap), "alloc puis liberation : etat initial");
	h_eq_i64("invariants", pmm_check(), 0);
	free(blk);
	fake_snap_drop(&snap);
}

static void	meta_default_seed(void)
{
	meta_roundtrip(FAKE_DEFAULT_SEED);
}

static void	meta_many_seeds(void)
{
	uint64_t	i;

	i = 1;
	while (i <= 6)
	{
		meta_roundtrip(i * 0x9e3779b97f4a7c15ull);
		i++;
	}
}

static void	meta_bulk_equals_singles(void)
{
	t_pmm_stats	bulk;
	t_pmm_stats	singles;
	uint64_t	phys;
	uint64_t	list[64];

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc_pages(PMM_DMA, 64, 1, 0);
	pmm_get_stats(&bulk);
	pmm_free_pages(phys, 64, PMM_DMA);
	h_eq_u64("64 pages", fake_drain(PMM_DMA, list, 64), 64);
	pmm_get_stats(&singles);
	h_eq_u64("meme nombre de libres", bulk.free_pages, singles.free_pages);
	h_eq_u64("meme compteur DMA", bulk.owned[PMM_DMA], singles.owned[PMM_DMA]);
	fake_free_list(list, 64, PMM_DMA);
	h_eq_i64("invariants", pmm_check(), 0);
}

int	main(void)
{
	h_begin("a02/metamorphic");
	h_run("metamorphique : alloc puis free melange (graine par defaut)",
		meta_default_seed);
	h_run("metamorphique : six autres graines", meta_many_seeds);
	h_run("metamorphique : n pages = n x 1 page", meta_bulk_equals_singles);
	return (h_end());
}
