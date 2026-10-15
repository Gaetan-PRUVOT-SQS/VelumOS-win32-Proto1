#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 8192

static const uint64_t	g_max[] = {0xfff, 0x1000, 0x1fff, 0x2000, 0x100000,
	0x100001, 0x100fff, 0x101000, 0x101fff, 0x200000, UINT64_MAX};
static const uint64_t	g_first[] = {0, 0, 0, 0x1000, 0x1000, 0, 0, 0x100000,
	0x100000, 0x100000, 0x100000};

static void	zone_max_table(void)
{
	uint64_t	phys;
	size_t		i;

	h_eq_i64("boot", fake_simple(0, 4), 0);
	i = 0;
	while (i < sizeof(g_max) / sizeof(g_max[0]))
	{
		phys = pmm_alloc_pages(PMM_DRIVER, 1, 1, g_max[i]);
		h_eq_u64("premiere frame sous max", phys, g_first[i]);
		if (phys)
			pmm_free(phys, PMM_DRIVER);
		i++;
	}
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	zone_dma16(void)
{
	uint64_t	*list;
	uint64_t	n;

	h_eq_i64("boot", fake_simple(1, 64), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain_to(PMM_DMA, list, CAP, 16 * MIB);
	h_eq_u64("3840 frames sous 16 Mio", n, 3840);
	h_eq_u64("la plus haute", fake_list_max(list, n), 16 * MIB - PAGE_SIZE);
	h_eq_u64("la plus basse", fake_list_min(list, n), MIB);
	h_eq_u64("sans max : apres la zone", pmm_alloc(PMM_KERNEL), 16 * MIB);
	fake_free_list(list, n, PMM_DMA);
	free(list);
}

static void	zone_dma32_with_high_ram(void)
{
	uint64_t	*list;
	uint64_t	n;
	uint64_t	low;

	fake_reset();
	fake_range(MIB, 4 * MIB, MEM_USABLE);
	fake_range(4 * GIB, 4 * MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	list = malloc(CAP * sizeof(uint64_t));
	low = fake_drain_to(PMM_DMA, list, CAP, 4 * GIB);
	h_eq_u64("tout le bas, rien au-dessus", low, 1024);
	h_true(fake_list_max(list, low) < 4 * GIB, "sous 4 Gio");
	n = fake_drain(PMM_KERNEL, list + low, CAP - low);
	h_eq_u64("meta 289 pages en haut", g_pmm.meta_pages, 289);
	h_eq_u64("le haut ensuite : 1024 - 289", n, 1024 - 289);
	h_eq_u64("premiere au-dessus de 32 bits", fake_list_min(list + low, n),
		4 * GIB);
	fake_free_list(list, low, PMM_DMA);
	fake_free_list(list + low, n, PMM_KERNEL);
	free(list);
}

static void	zone_high_only(void)
{
	fake_reset();
	fake_range(4 * GIB, 8 * MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	h_eq_u64("rien sous 4 Gio", pmm_alloc_pages(PMM_DMA, 1, 1, 4 * GIB), 0);
	h_eq_u64("max 4 Gio + 1 Mio", pmm_alloc_pages(PMM_DMA, 1, 1,
			4 * GIB + MIB), 4 * GIB);
	h_eq_u64("sans max : saute le trou", pmm_alloc(PMM_KERNEL),
		4 * GIB + PAGE_SIZE);
	h_eq_u64("adresse sur 64 bits", pmm_alloc(PMM_KERNEL) >> 32, 1);
}

int	main(void)
{
	h_begin("a02/alloc_zone");
	h_run("alloc/limite : max autour de 1 Mio et frame 0", zone_max_table);
	h_run("alloc/partition : DMA ISA 16 Mio", zone_dma16);
	h_run("alloc/partition : DMA 32 bits, RAM au-dessus",
		zone_dma32_with_high_ram);
	h_run("alloc/limite : RAM seulement au-dessus de 4 Gio", zone_high_only);
	return (h_end());
}
