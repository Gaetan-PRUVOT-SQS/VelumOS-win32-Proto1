#include "velum/err.h"
#include "a02_fake.h"

static void	err_no_usable(void)
{
	fake_reset();
	h_eq_i64("aucune plage", fake_boot(), E_NOMEM);
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_RESERVED);
	h_eq_i64("que du reserve", fake_boot(), E_NOMEM);
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_BOOT_RECLAIM);
	h_eq_i64("que du reclaim", fake_boot(), E_NOMEM);
	fake_reset();
	fake_range(0, PAGE_SIZE, MEM_USABLE);
	fake_range(MIB, 0xfff, MEM_USABLE);
	h_eq_i64("frame 0 seule et moins d'une frame", fake_boot(), E_NOMEM);
	h_true(!g_pmm.bits, "etat inchange apres echec");
}

static void	err_overlap(void)
{
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	fake_range(8 * MIB, 2 * MIB, MEM_RESERVED);
	h_eq_i64("utilisable x reserve", fake_boot(), E_PROTO);
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	fake_range(8 * MIB, 2 * MIB, MEM_USABLE);
	h_eq_i64("utilisable x utilisable", fake_boot(), E_PROTO);
	fake_reset();
	fake_range(10 * MIB, 2 * MIB, MEM_BOOT_RECLAIM);
	fake_range(11 * MIB, 2 * MIB, MEM_KERNEL);
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	h_eq_i64("reclaim x noyau", fake_boot(), E_PROTO);
}

static void	err_overlap_tolerated(void)
{
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	fake_range(10 * MIB, 2 * MIB, MEM_RESERVED);
	fake_range(11 * MIB, 2 * MIB, MEM_KERNEL);
	fake_range(4 * MIB, 0, MEM_RESERVED);
	h_eq_i64("recouvrement hors pool et plage vide", fake_boot(), 0);
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	err_range_count(void)
{
	uint32_t	i;

	fake_reset();
	g_fake_info.nranges = BOOT_MAX_RANGES + 1;
	h_eq_i64("129 plages", fake_boot(), E_PROTO);
	fake_reset();
	i = 0;
	while (i < BOOT_MAX_RANGES - 1)
		fake_range((i++ + 64) * MIB, MIB / 2, MEM_RESERVED);
	fake_range(MIB, 16 * MIB, MEM_USABLE);
	h_eq_u64("128 plages", g_fake_info.nranges, BOOT_MAX_RANGES);
	h_eq_i64("128 plages acceptees", fake_boot(), 0);
}

int	main(void)
{
	h_begin("a02/init_errors");
	h_run("init/table : aucune plage utilisable", err_no_usable);
	h_run("init/table : recouvrements refuses", err_overlap);
	h_run("init/table : recouvrements toleres", err_overlap_tolerated);
	h_run("init/limite : 128 et 129 plages", err_range_count);
	return (h_end());
}
