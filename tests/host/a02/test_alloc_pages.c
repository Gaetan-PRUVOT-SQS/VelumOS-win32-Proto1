#include "velum/err.h"
#include "a02_fake.h"

static const uint64_t	g_sizes[] = {2, 3, 7, 64, 513};

static int	contiguous_owned(uint64_t phys, uint64_t n, int owner)
{
	uint64_t	i;

	i = 0;
	while (i < n)
	{
		if (g_pmm.owner[(phys >> PAGE_SHIFT) + i] != owner)
			return (0);
		i++;
	}
	return (1);
}

static void	pages_sizes(void)
{
	uint64_t	phys;
	size_t		i;

	h_eq_i64("boot", fake_simple(1, 16), 0);
	i = 0;
	while (i < sizeof(g_sizes) / sizeof(g_sizes[0]))
	{
		phys = pmm_alloc_pages(PMM_DMA, g_sizes[i], 1, 0);
		h_true(phys && is_aligned(phys, PAGE_SIZE), "adresse valide");
		h_true(contiguous_owned(phys, g_sizes[i], PMM_DMA), "contigues");
		h_eq_u64("compteur DMA", fake_owned(PMM_DMA), g_sizes[i]);
		pmm_free_pages(phys, g_sizes[i], PMM_DMA);
		h_eq_u64("compteur DMA remis", fake_owned(PMM_DMA), 0);
		i++;
	}
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	pages_alignment(void)
{
	static const uint64_t	al[] = {1, 2, 512};
	static const uint64_t	n[] = {1, 3};
	uint64_t				phys;
	size_t					i;
	size_t					j;

	h_eq_i64("boot", fake_simple(1, 16), 0);
	i = 0;
	while (i < 3 * 2)
	{
		j = 0;
		while (j < 3)
		{
			pmm_alloc(PMM_KERNEL);
			phys = pmm_alloc_pages(PMM_DMA, n[i % 2], al[i / 2], 0);
			h_true(phys && (phys / PAGE_SIZE) % al[i / 2] == 0, "aligne");
			j++;
		}
		i++;
	}
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	pages_exact_fit(void)
{
	h_eq_i64("boot", fake_simple(1, 2), 0);
	h_eq_u64("255 libres", fake_free_pages(), 255);
	h_eq_u64("255 contigues", pmm_alloc_pages(PMM_DMA, 255, 1, 0), MIB);
	h_eq_u64("plus rien", pmm_alloc(PMM_DMA), 0);
	pmm_free_pages(MIB, 255, PMM_DMA);
	h_eq_u64("256 refusees", pmm_alloc_pages(PMM_DMA, 256, 1, 0), 0);
	h_eq_u64("une frame", pmm_alloc(PMM_KERNEL), MIB);
	h_eq_u64("254 contre la meta", pmm_alloc_pages(PMM_DMA, 254, 1, 0),
		MIB + PAGE_SIZE);
	h_eq_u64("254 puis plus rien", fake_free_pages(), 0);
}

int	main(void)
{
	h_begin("a02/alloc_pages");
	h_run("alloc/partition : n frames contigues", pages_sizes);
	h_run("alloc/partition : alignement 1, 2, 512 pages", pages_alignment);
	h_run("alloc/limite : ajustement exact contre la fin", pages_exact_fit);
	return (h_end());
}
