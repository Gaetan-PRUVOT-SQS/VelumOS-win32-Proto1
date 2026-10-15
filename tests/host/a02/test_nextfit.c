#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 4096

static void	nextfit_sequential(void)
{
	uint64_t	p[20];
	uint64_t	n;
	uint64_t	next;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	n = fake_drain(PMM_KERNEL, p, 20);
	h_eq_u64("20 frames", n, 20);
	h_eq_u64("la premiere", p[0], MIB);
	h_eq_u64("croissantes et jointives", p[19], MIB + 19 * PAGE_SIZE);
	pmm_free(p[3], PMM_KERNEL);
	next = pmm_alloc(PMM_KERNEL);
	h_eq_u64("pas de reutilisation immediate (next-fit)", next,
		MIB + 20 * PAGE_SIZE);
	h_eq_u64("l'indice avance", g_pmm.hint, 256 + 21);
}

static void	nextfit_wraps(void)
{
	uint64_t	*list;
	uint64_t	n;

	h_eq_i64("boot", fake_simple(1, 2), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("pool vide", n, 255);
	pmm_free(list[200], PMM_KERNEL);
	pmm_free(list[10], PMM_KERNEL);
	h_eq_u64("repart du bas apres le tour", pmm_alloc(PMM_KERNEL), list[10]);
	h_eq_u64("puis la suivante", pmm_alloc(PMM_KERNEL), list[200]);
	h_eq_u64("plus rien", pmm_alloc(PMM_KERNEL), 0);
	free(list);
}

static void	nextfit_straddle(void)
{
	uint64_t	*list;
	uint64_t	n;
	uint64_t	i;

	h_eq_i64("boot", fake_simple(1, 2), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("255 frames", n, 255);
	i = 4;
	while (i < 12)
		pmm_free(list[i++], PMM_KERNEL);
	g_pmm.hint = (list[8] >> PAGE_SHIFT);
	h_eq_u64("8 frames dont le debut est avant l'indice",
		pmm_alloc_pages(PMM_DMA, 8, 1, 0), list[4]);
	h_eq_u64("pool vide", fake_free_pages(), 0);
	fake_free_list(list, 4, PMM_KERNEL);
	free(list);
}

static void	nextfit_fragmentation(void)
{
	uint64_t	*list;
	uint64_t	n;
	uint64_t	i;

	h_eq_i64("boot", fake_simple(1, 9), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	i = 0;
	while (i < n)
	{
		pmm_free(list[i], PMM_KERNEL);
		i += 2;
	}
	h_eq_u64("moitie libre", fake_free_pages(), (n + 1) / 2);
	h_eq_u64("aucune paire contigue", pmm_alloc_pages(PMM_DMA, 2, 1, 0), 0);
	i = 1;
	while (i < n)
	{
		pmm_free(list[i], PMM_KERNEL);
		i += 2;
	}
	h_true(pmm_alloc_pages(PMM_DMA, 512, 512, 0) != 0, "512 alignees");
	h_true(pmm_alloc_pages(PMM_DMA, 64, 1, 0) != 0, "puis 64 contigues");
	free(list);
}

int	main(void)
{
	h_begin("a02/nextfit");
	h_run("nextfit/etat : sequentiel, pas de reutilisation immediate",
		nextfit_sequential);
	h_run("nextfit/etat : retour au debut apres le haut", nextfit_wraps);
	h_run("nextfit/limite : plage a cheval sur l'indice", nextfit_straddle);
	h_run("nextfit/etat : fragmentation puis defragmentation",
		nextfit_fragmentation);
	return (h_end());
}
