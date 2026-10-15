#include "velum/err.h"
#include "a02_fake.h"

static void	limit_meta_too_small(void)
{
	fake_reset();
	fake_range(MIB, 64 * 1024, MEM_USABLE);
	fake_range(4 * GIB - 64 * 1024, 64 * 1024, MEM_USABLE);
	h_eq_i64("meta plus grande que la plus grande plage", fake_boot(),
		E_NOMEM);
	h_true(!g_pmm.bits, "etat inchange apres echec");
}

static void	limit_only_metadata(void)
{
	fake_reset();
	fake_range(MIB, PAGE_SIZE, MEM_USABLE);
	h_eq_i64("une frame = la meta, rien de libre", fake_boot(), E_NOMEM);
	h_true(!g_pmm.bits, "etat remis a zero");
	fake_reset();
	fake_range(MIB, 2 * PAGE_SIZE, MEM_USABLE);
	h_eq_i64("deux frames : meta + une libre", fake_boot(), 0);
	h_eq_u64("une frame libre", fake_free_pages(), 1);
	h_eq_u64("c'est la premiere", pmm_alloc(PMM_KERNEL), MIB);
	h_eq_u64("puis plus rien", pmm_alloc(PMM_KERNEL), 0);
}

static void	limit_physical_width(void)
{
	fake_reset();
	fake_range((1ull << 52) - MIB, MIB, MEM_USABLE);
	h_eq_i64("2^52 : meta introuvable", fake_boot(), E_NOMEM);
	fake_reset();
	fake_range((1ull << 52) - MIB, MIB + PAGE_SIZE, MEM_USABLE);
	h_eq_i64("au-dela de 2^52 : refuse", fake_boot(), E_PROTO);
	fake_reset();
	fake_range(1ull << 60, MIB, MEM_USABLE);
	h_eq_i64("2^60 : refuse", fake_boot(), E_PROTO);
}

static void	limit_wraparound(void)
{
	fake_reset();
	fake_range(0xfffffffffffff000ull, 0x2000, MEM_USABLE);
	h_eq_i64("fin au-dela de 2^64", fake_boot(), E_NOMEM);
	fake_reset();
	fake_range(UINT64_MAX - 100, 200, MEM_USABLE);
	h_eq_i64("base dans la derniere page", fake_boot(), E_NOMEM);
	fake_reset();
	fake_range(MIB, 8 * MIB, MEM_USABLE);
	fake_range(UINT64_MAX - 10, 100, MEM_RESERVED);
	h_eq_i64("reserve qui deborde ignore", fake_boot(), 0);
}

int	main(void)
{
	h_begin("a02/init_limits");
	h_run("init/limite : metadonnees plus grandes que la plage",
		limit_meta_too_small);
	h_run("init/limite : RAM = metadonnees", limit_only_metadata);
	h_run("init/limite : largeur d'adresse physique", limit_physical_width);
	h_run("init/limite : debordement 64 bits", limit_wraparound);
	return (h_end());
}
