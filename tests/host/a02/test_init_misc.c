#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 2048

static void	misc_tiny_ranges(void)
{
	uint64_t	*list;
	uint64_t	n;

	fake_reset();
	fake_range(MIB, 4 * MIB, MEM_USABLE);
	fake_range(8 * MIB + 0x10, 0x500, MEM_USABLE);
	fake_range(9 * MIB, 0, MEM_USABLE);
	fake_range(10 * MIB + 1, 0x100, MEM_BOOT_RECLAIM);
	fake_range(12 * MIB, MIB, MEM_BOOT_RECLAIM);
	h_eq_i64("boot", fake_boot(), 0);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("seules les frames entieres", n, 1024 - g_pmm.meta_pages);
	h_eq_u64("rien dans les plages minuscules", fake_inside(list, n,
			8 * MIB, 11 * MIB), 0);
	h_eq_i64("reclaim minuscule : rien a ajouter",
		pmm_add_region(MIB * 10 + 1, 0x100), 0);
	h_eq_i64("reclaim valide apres un minuscule",
		pmm_add_region(12 * MIB, MIB), 256);
	fake_free_list(list, n, PMM_KERNEL);
	free(list);
}

static void	misc_double_init(void)
{
	t_snap	snap;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	fake_snap_take(&snap);
	h_eq_i64("deuxieme init refuse", pmm_boot_init(), E_BUSY);
	h_true(fake_snap_same(&snap), "etat intact");
	h_true(pmm_alloc(PMM_KERNEL) != 0, "l'allocateur sert toujours");
	fake_snap_drop(&snap);
}

static void	misc_stats_null(void)
{
	h_eq_i64("boot", fake_simple(1, 8), 0);
	pmm_get_stats(NULL);
	h_eq_i64("pas de verrou pris", fake_irq_depth(), 0);
	h_eq_u64("ticket intact", g_pmm.ticket, 0);
}

static void	misc_region_beyond_span(void)
{
	h_eq_i64("boot", fake_region_machine(), 0);
	fake_range(40 * MIB, MIB, MEM_BOOT_RECLAIM);
	h_eq_i64("reclaim hors de la memoire geree",
		pmm_add_region(40 * MIB, MIB), E_RANGE);
	h_eq_i64("rien n'a change", pmm_check(), 0);
	g_fake_info.nranges = 100000;
	h_eq_i64("nranges corrompu : lecture bornee",
		pmm_add_region(100 * MIB, MIB), E_PERM);
}

int	main(void)
{
	h_begin("a02/init_misc");
	h_run("init/limite : plages plus petites qu'une frame", misc_tiny_ranges);
	h_run("init/etat : deuxieme appel refuse", misc_double_init);
	h_run("stats/supposition : pointeur nul", misc_stats_null);
	h_run("region/supposition : infos de boot alterees",
		misc_region_beyond_span);
	return (h_end());
}
