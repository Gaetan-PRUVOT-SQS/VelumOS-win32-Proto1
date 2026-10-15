#include "velum/err.h"
#include "a02_fake.h"

static void	state_valid_cycle(void)
{
	uint64_t	first;
	uint64_t	again;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	first = pmm_alloc(PMM_KERNEL);
	h_eq_u64("libre -> alloue", g_pmm.owner[first >> PAGE_SHIFT], PMM_KERNEL);
	pmm_free(first, PMM_KERNEL);
	h_eq_u64("alloue -> libre", g_pmm.owner[first >> PAGE_SHIFT], PMM_FREE);
	g_pmm.hint = first >> PAGE_SHIFT;
	again = pmm_alloc(PMM_HEAP);
	h_eq_u64("libre -> alloue a un autre", again, first);
	h_eq_u64("nouveau proprietaire", g_pmm.owner[again >> PAGE_SHIFT],
		PMM_HEAP);
	pmm_free(again, PMM_HEAP);
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	state_invalid_transitions(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_USER);
	h_eq_i64("alloue -> libere par un autre", fake_free_try(phys, 1,
			PMM_DRIVER), 1);
	pmm_free(phys, PMM_USER);
	h_eq_i64("libre -> libre", fake_free_try(phys, 1, PMM_USER), 1);
	h_eq_i64("reserve -> libre", fake_free_try(0, 1, PMM_USER), 1);
	h_eq_i64("meta -> libre", fake_free_try(g_pmm.meta_phys, 1, PMM_USER), 1);
	h_eq_i64("quatre refus", fake_assert_count(), 4);
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	state_one_switch(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_USER);
	pmm_free(phys, PMM_USER);
	h_eq_i64("alloc, free, free : refuse", fake_free_try(phys, 1, PMM_USER),
		1);
	g_pmm.hint = phys >> PAGE_SHIFT;
	h_eq_u64("alloc, free, alloc : meme frame", pmm_alloc(PMM_USER), phys);
	h_eq_i64("alloc, free, alloc, free", fake_free_try(phys, 1, PMM_USER), 0);
	h_eq_i64("encore une fois : refuse", fake_free_try(phys, 1, PMM_USER), 1);
	h_eq_u64("tout libre", fake_owned(PMM_USER), 0);
}

int	main(void)
{
	h_begin("a02/states");
	h_run("etats/0-switch : transitions valides", state_valid_cycle);
	h_run("etats/invalides : transitions refusees", state_invalid_transitions);
	h_run("etats/1-switch : paires de transitions", state_one_switch);
	return (h_end());
}
