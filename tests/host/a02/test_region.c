#include <stdlib.h>
#include <string.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 9000

static void	region_add_ok(void)
{
	t_pmm_stats	a;
	t_pmm_stats	b;
	uint64_t	*list;
	uint64_t	n;

	h_eq_i64("boot", fake_region_machine(), 0);
	pmm_get_stats(&a);
	h_eq_i64("ajout du premier reclaim", pmm_add_region(17 * MIB, 2 * MIB),
		512);
	h_true(strstr(fake_last_log(), "+512") != NULL, "journal");
	h_eq_i64("ajout du second", pmm_add_region(20 * MIB, 2 * MIB), 512);
	pmm_get_stats(&b);
	h_eq_u64("total", b.total_pages, a.total_pages + 1024);
	h_eq_u64("libres", b.free_pages, a.free_pages + 1024);
	h_eq_u64("reserve", b.reserved_pages, a.reserved_pages - 1024);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_KERNEL, list, CAP);
	h_eq_u64("frames au-dela de 17 Mio utilisables", fake_inside(list, n,
			17 * MIB, 22 * MIB), 1024);
	h_eq_u64("pas dans le trou reserve", fake_inside(list, n, 19 * MIB,
			20 * MIB), 0);
	fake_free_list(list, n, PMM_KERNEL);
	h_eq_i64("invariants", pmm_check(), 0);
	free(list);
}

static void	region_errors(void)
{
	fake_reset();
	h_eq_i64("avant init", pmm_add_region(17 * MIB, 2 * MIB), E_INVAL);
	h_eq_i64("boot", fake_region_machine(), 0);
	h_eq_i64("longueur nulle", pmm_add_region(17 * MIB, 0), E_INVAL);
	h_eq_i64("debordement", pmm_add_region(UINT64_MAX - 10, 100), E_INVAL);
	h_eq_i64("plage reservee", pmm_add_region(19 * MIB, MIB), E_PERM);
	h_eq_i64("hors de toute plage", pmm_add_region(100 * MIB, MIB), E_PERM);
	h_eq_i64("plage utilisable", pmm_add_region(MIB, MIB), E_PERM);
	h_eq_i64("a cheval sur reclaim et reserve",
		pmm_add_region(18 * MIB, 2 * MIB), E_PERM);
	h_eq_i64("metadonnees", pmm_add_region(g_pmm.meta_phys,
			PAGE_SIZE), E_PERM);
	h_eq_i64("rien n'a change", pmm_check(), 0);
	h_eq_u64("total", g_pmm.stats.total_pages, 4096 - g_pmm.meta_pages);
}

static void	region_double_and_partial(void)
{
	h_eq_i64("boot", fake_region_machine(), 0);
	h_eq_i64("sous-plage non alignee", pmm_add_region(17 * MIB + 0x800,
			0x2800), 2);
	h_eq_i64("meme sous-plage", pmm_add_region(17 * MIB + 0x800, 0x2800),
		E_EXIST);
	h_eq_i64("plage entiere deja entamee", pmm_add_region(17 * MIB, 2 * MIB),
		E_EXIST);
	h_eq_u64("2 frames seulement", g_pmm.stats.total_pages,
		4096 - g_pmm.meta_pages + 2);
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	region_degenerate(void)
{
	h_eq_i64("boot", fake_region_machine(), 0);
	h_eq_i64("moins d'une frame", pmm_add_region(17 * MIB + 1, 100), 0);
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	fake_range(0, 0x10000, MEM_BOOT_RECLAIM);
	h_eq_i64("boot", fake_boot(), 0);
	h_eq_i64("reclaim contenant la frame 0", pmm_add_region(0, 0x10000), 15);
	h_eq_u64("frame 0 toujours interdite",
		pmm_alloc_pages(PMM_DRIVER, 1, 1, MIB), PAGE_SIZE);
	h_eq_i64("invariants", pmm_check(), 0);
}

int	main(void)
{
	h_begin("a02/region");
	h_run("region/etat : reclaim rendu au pool", region_add_ok);
	h_run("region/table : arguments refuses", region_errors);
	h_run("region/etat : sous-plage puis plage entiere",
		region_double_and_partial);
	h_run("region/limite : moins d'une frame, frame 0", region_degenerate);
	return (h_end());
}
