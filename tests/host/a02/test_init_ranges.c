#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 8192

static void	machine_all_types(void)
{
	fake_reset();
	fake_range(1 * MIB, 8 * MIB, MEM_USABLE);
	fake_range(9 * MIB, MIB, MEM_RESERVED);
	fake_range(10 * MIB, MIB, MEM_ACPI_RECLAIM);
	fake_range(11 * MIB, MIB, MEM_ACPI_NVS);
	fake_range(12 * MIB, MIB, MEM_BAD);
	fake_range(13 * MIB, MIB, MEM_KERNEL);
	fake_range(14 * MIB, MIB, MEM_FRAMEBUFFER);
	fake_range(15 * MIB, 16 * MIB, MEM_USABLE);
}

static uint64_t	count_forbidden(const uint64_t *l, uint64_t n)
{
	uint64_t	bad;
	uint64_t	meta_end;

	meta_end = g_pmm.meta_phys + g_pmm.meta_pages * PAGE_SIZE;
	bad = fake_inside(l, n, 9 * MIB, 15 * MIB);
	bad += fake_inside(l, n, 0, MIB);
	bad += fake_inside(l, n, 31 * MIB, UINT64_MAX);
	bad += fake_inside(l, n, g_pmm.meta_phys, meta_end);
	return (bad);
}

static void	init_types_never_allocated(void)
{
	uint64_t	*list;
	uint64_t	n;

	machine_all_types();
	h_eq_i64("boot", fake_boot(), 0);
	h_eq_u64("meta 3 pages", g_pmm.meta_pages, 3);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("frames libres 6144 - 3", n, 6141);
	h_eq_u64("aucune frame interdite", count_forbidden(list, n), 0);
	h_eq_u64("aucun doublon", fake_list_dups(list, n), 0);
	h_eq_u64("pool vide", fake_free_pages(), 0);
	fake_free_list(list, n, PMM_KERNEL);
	h_eq_i64("invariants", pmm_check(), 0);
	free(list);
}

static void	init_unaligned_inward(void)
{
	uint64_t	*list;
	uint64_t	n;
	uint64_t	low;

	fake_reset();
	fake_range(0x100800, 0x5000, MEM_USABLE);
	fake_range(2 * MIB, 2 * MIB + 0x7ff, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	low = fake_inside(list, n, 0, 2 * MIB);
	h_eq_u64("4 frames dans la plage non alignee", low, 4);
	h_eq_u64("premiere frame 0x101000", list[0], 0x101000);
	h_eq_u64("derniere frame basse 0x104000", list[3], 0x104000);
	h_eq_u64("fin tronquee a 4 Mio", n - low,
		(4 * MIB - 2 * MIB) / PAGE_SIZE - g_pmm.meta_pages);
	fake_free_list(list, n, PMM_KERNEL);
	free(list);
}

int	main(void)
{
	h_begin("a02/init_ranges");
	h_run("init/partition : types non utilisables jamais donnes",
		init_types_never_allocated);
	h_run("init/limite : alignement vers l'interieur", init_unaligned_inward);
	return (h_end());
}
