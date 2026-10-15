#include <stdio.h>
#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 100000

static void	cx_sequential(void)
{
	uint64_t	*list;
	uint64_t	n;

	h_eq_i64("boot 1 Gio", fake_simple(1, 1024), 0);
	list = malloc(CAP * sizeof(uint64_t));
	g_pmm.scanned = 0;
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("100000 allocations", n, CAP);
	fake_report("remplissage sequentiel", g_pmm.scanned, n);
	h_true(g_pmm.scanned <= n + n / 64 + 4, "un mot par allocation");
	fake_free_list(list, n, PMM_KERNEL);
	free(list);
}

static void	cx_nearly_full(void)
{
	uint64_t	*list;
	uint64_t	n;

	h_eq_i64("boot 256 Mio", fake_simple(1, 257), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("65536 - 19 frames", n, 65536 - 19);
	g_pmm.scanned = 0;
	h_eq_u64("pool plein : echec immediat", pmm_alloc(PMM_KERNEL), 0);
	h_eq_u64("zero mot lu (compteur de libres)", g_pmm.scanned, 0);
	pmm_free(list[300], PMM_KERNEL);
	g_pmm.hint = (list[301] >> PAGE_SHIFT);
	h_eq_u64("la seule frame libre", pmm_alloc(PMM_KERNEL), list[300]);
	fake_report("une frame libre, pire cas", g_pmm.scanned, 1);
	h_true(g_pmm.scanned <= g_pmm.words + 4, "au plus un tour de bitmap");
	fake_free_list(list, 300, PMM_KERNEL);
	free(list);
}

static void	cx_random_load(void)
{
	uint64_t	*live;
	uint64_t	nlive;
	uint64_t	allocs;

	h_eq_i64("boot 64 Mio", fake_simple(1, 65), 0);
	fake_seed(0x1234);
	printf("graine = %#llx\n", (unsigned long long)fake_seed_value());
	live = malloc(CAP * sizeof(uint64_t));
	nlive = 0;
	fake_churn(live, &nlive, 8000, 8000);
	g_pmm.scanned = 0;
	allocs = fake_churn(live, &nlive, 8000, 60000);
	fake_report("charge 50 %, alea", g_pmm.scanned, allocs);
	h_true(g_pmm.scanned <= 4 * allocs, "moins de 4 mots par allocation");
	fake_free_list(live, nlive, PMM_KERNEL);
	free(live);
}

static void	cx_fragmented(void)
{
	uint64_t	*list;
	uint64_t	n;
	uint64_t	i;

	h_eq_i64("boot 32 Mio", fake_simple(1, 33), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	i = 0;
	while (i < n)
	{
		pmm_free(list[i], PMM_KERNEL);
		i += 2;
	}
	g_pmm.scanned = 0;
	h_eq_u64("aucune paire dans un pool en damier",
		pmm_alloc_pages(PMM_DMA, 2, 1, 0), 0);
	fake_report("damier, pire cas", g_pmm.scanned, 1);
	h_true(g_pmm.scanned <= 2 * g_pmm.span, "borne lineaire en frames");
	free(list);
}

int	main(void)
{
	h_begin("a02/complexity");
	h_run("complexite/mesure : sequentiel", cx_sequential);
	h_run("complexite/limite : plein et presque plein", cx_nearly_full);
	h_run("complexite/aleatoire : charge 50 %", cx_random_load);
	h_run("complexite/limite : damier", cx_fragmented);
	return (h_end());
}
