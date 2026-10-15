#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 8192

static void	init_reclaim_kept_aside(void)
{
	uint64_t	*list;
	uint64_t	n;
	t_pmm_stats	s;

	fake_reset();
	fake_range(1 * MIB, 16 * MIB, MEM_USABLE);
	fake_range(17 * MIB, 8 * MIB, MEM_BOOT_RECLAIM);
	h_eq_i64("boot", fake_boot(), 0);
	pmm_get_stats(&s);
	h_eq_u64("span jusqu'au reclaim", g_pmm.span, 25 * 256);
	h_eq_u64("meta 2 pages", g_pmm.meta_pages, 2);
	h_eq_u64("reserve = bas + reclaim + meta", s.reserved_pages,
		256 + 2048 + 2);
	h_eq_u64("total = libre", s.total_pages, s.free_pages);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("libres 4096 - 2", n, 4094);
	h_true(fake_list_max(list, n) < 17 * MIB, "rien dans le reclaim");
	fake_free_list(list, n, PMM_KERNEL);
	free(list);
}

static void	init_frame_zero_and_low(void)
{
	uint64_t	*list;
	uint64_t	n;

	fake_reset();
	fake_range(0, 4 * MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_true(fake_list_min(list, n) >= MIB, "sans max : rien sous 1 Mio");
	fake_free_list(list, n, PMM_KERNEL);
	n = fake_drain_to(PMM_DRIVER, list, CAP, MIB);
	h_eq_u64("zone basse : frames 1 a 255", n, 255);
	h_eq_u64("jamais la frame 0", fake_list_min(list, n), PAGE_SIZE);
	h_eq_u64("derniere frame basse", fake_list_max(list, n), MIB - PAGE_SIZE);
	fake_free_list(list, n, PMM_DRIVER);
	free(list);
}

static void	init_single_frame_range(void)
{
	uint64_t	*list;
	uint64_t	n;

	fake_reset();
	fake_range(2 * MIB, 2 * MIB, MEM_USABLE);
	fake_range(0x500000, PAGE_SIZE, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("plage d'une frame utilisable", fake_inside(list, n, 0x500000,
			0x501000), 1);
	h_eq_u64("total", n, 512 - g_pmm.meta_pages + 1);
	fake_free_list(list, n, PMM_KERNEL);
	free(list);
}

int	main(void)
{
	h_begin("a02/init_reclaim");
	h_run("init/partition : plages reclaim gardees de cote",
		init_reclaim_kept_aside);
	h_run("init/limite : frame 0 et zone sous 1 Mio", init_frame_zero_and_low);
	h_run("init/limite : plage d'une seule frame", init_single_frame_range);
	return (h_end());
}
