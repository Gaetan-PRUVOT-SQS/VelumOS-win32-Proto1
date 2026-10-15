#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 16384

static void	layout_adjacent(void)
{
	uint64_t	run;

	fake_reset();
	fake_range(MIB, MIB, MEM_USABLE);
	fake_range(2 * MIB, MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	h_eq_u64("meta 1 page", g_pmm.meta_pages, 1);
	h_eq_u64("512 contigues refusees", pmm_alloc_pages(PMM_DMA, 512, 1, 0),
		0);
	run = pmm_alloc_pages(PMM_DMA, 511, 1, 0);
	h_eq_u64("511 contigues a cheval sur la jonction", run, MIB);
}

static void	layout_huge_hole(void)
{
	uint64_t	*list;
	uint64_t	n;

	fake_reset();
	fake_range(MIB, 32 * MIB, MEM_USABLE);
	fake_range(8 * GIB, 16 * MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	h_eq_u64("span", g_pmm.span, (8 * GIB + 16 * MIB) / PAGE_SIZE);
	h_eq_u64("meta 578 pages", g_pmm.meta_pages, 578);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("12288 - 578 frames", n, 12288 - 578);
	h_eq_u64("4096 frames au-dela de 8 Gio",
		fake_inside(list, n, 8 * GIB, UINT64_MAX), 4096);
	h_eq_u64("derniere frame 8 Gio + 16 Mio - 4 Kio", fake_list_max(list, n),
		8 * GIB + 16 * MIB - PAGE_SIZE);
	fake_free_list(list, n, PMM_KERNEL);
	free(list);
}

static void	layout_meta_placement(void)
{
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	fake_range(16 * MIB, 32 * MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	h_eq_u64("meta en fin de la plus grande plage", g_pmm.meta_phys
		+ g_pmm.meta_pages * PAGE_SIZE, 48 * MIB);
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	fake_range(16 * MIB, 8 * MIB, MEM_USABLE);
	h_eq_i64("boot egalite", fake_boot(), 0);
	h_eq_u64("a egalite : la derniere du tableau", g_pmm.meta_phys
		+ g_pmm.meta_pages * PAGE_SIZE, 24 * MIB);
}

static void	layout_meta_untouchable(void)
{
	uint64_t	*list;
	uint64_t	n;

	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("jamais dans la meta", fake_inside(list, n, g_pmm.meta_phys,
			g_pmm.meta_phys + g_pmm.meta_pages * PAGE_SIZE), 0);
	h_eq_u64("meta marquee", g_pmm.owner[g_pmm.meta_phys >> PAGE_SHIFT],
		PMM_MARK_META);
	fake_free_list(list, n, PMM_KERNEL);
	free(list);
}

int	main(void)
{
	h_begin("a02/init_layout");
	h_run("init/limite : plages adjacentes", layout_adjacent);
	h_run("init/limite : trou enorme et adresses 8 Gio+", layout_huge_hole);
	h_run("init/table : emplacement des metadonnees", layout_meta_placement);
	h_run("init/etat : metadonnees jamais allouees", layout_meta_untouchable);
	return (h_end());
}
