#include "velum/err.h"
#include "a02_fake.h"

static void	hint_outside_zone(void)
{
	h_eq_i64("boot", fake_simple(1, 8), 0);
	g_pmm.hint = 5;
	h_eq_u64("indice sous la zone : on part du bas", pmm_alloc(PMM_KERNEL),
		MIB);
	h_eq_u64("l'indice est recale", g_pmm.hint, 257);
	g_pmm.hint = g_pmm.span + 100;
	h_eq_u64("indice au-dela du span", pmm_alloc(PMM_KERNEL),
		MIB + PAGE_SIZE);
	g_pmm.hint = g_pmm.span;
	h_eq_u64("indice egal au span", pmm_alloc(PMM_KERNEL),
		MIB + 2 * PAGE_SIZE);
}

static void	hint_ignored_when_constrained(void)
{
	h_eq_i64("boot", fake_simple(1, 8), 0);
	g_pmm.hint = 300;
	h_eq_u64("avec max : premier ajustement depuis le bas",
		pmm_alloc_pages(PMM_DMA, 1, 1, 4 * MIB), MIB);
	h_eq_u64("l'indice n'a pas bouge", g_pmm.hint, 300);
	h_eq_u64("sans max : part de l'indice", pmm_alloc(PMM_KERNEL),
		300 * PAGE_SIZE);
}

static void	hint_frame_zero_guard(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(0, 4), 0);
	g_pmm.owner[0] = PMM_FREE;
	g_pmm.bits[0] &= ~1ull;
	phys = pmm_alloc_pages(PMM_DRIVER, 1, 1, 0x2000);
	h_eq_u64("frame 0 libre par corruption : jamais donnee", phys, PAGE_SIZE);
	phys = pmm_alloc_pages(PMM_DRIVER, 1, 1, 0x1000);
	h_eq_u64("et max d'une page : refus", phys, 0);
}

int	main(void)
{
	h_begin("a02/hint");
	h_run("nextfit/limite : indice hors de la zone", hint_outside_zone);
	h_run("nextfit/table : indice et demande bornee",
		hint_ignored_when_constrained);
	h_run("zone/supposition : frame 0 corrompue", hint_frame_zero_guard);
	return (h_end());
}
