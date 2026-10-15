#include "velum/err.h"
#include "a02_fake.h"

static void	fail_zero(void)
{
	t_pmm_stats	a;
	t_pmm_stats	b;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	pmm_get_stats(&a);
	pmm_fail_after(0);
	h_eq_u64("pmm_alloc", pmm_alloc(PMM_KERNEL), 0);
	h_eq_u64("pmm_alloc_zero", pmm_alloc_zero(PMM_KERNEL), 0);
	h_eq_u64("pmm_alloc_pages", pmm_alloc_pages(PMM_KERNEL, 4, 4, 0), 0);
	pmm_get_stats(&b);
	h_eq_u64("fail_calls +3", b.fail_calls, a.fail_calls + 3);
	h_eq_u64("pool inchange", b.free_pages, a.free_pages);
	pmm_fail_after(-1);
	h_true(pmm_alloc(PMM_KERNEL) != 0, "reactive par -1");
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	fail_every_rank(void)
{
	uint64_t	list[10];
	uint64_t	ok;
	uint64_t	k;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	k = 0;
	while (k < 10)
	{
		pmm_fail_after((int64_t)k);
		ok = fake_burst(list);
		h_eq_u64("exactement k succes", ok, k);
		pmm_fail_after(-1);
		fake_free_list(list, ok, PMM_KERNEL);
		h_eq_u64("aucune fuite", fake_owned(PMM_KERNEL), 0);
		k++;
	}
}

static void	fail_stays_failing(void)
{
	uint64_t	first;
	uint64_t	i;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	pmm_fail_after(2);
	h_true(fake_mixed_alloc(0) != 0, "1");
	first = fake_mixed_alloc(1);
	h_true(first != 0, "2");
	i = 0;
	while (i < 5)
		h_eq_u64("echec persistant", fake_mixed_alloc(i++), 0);
	pmm_free(first, PMM_KERNEL);
	h_eq_i64("la liberation reste possible", fake_assert_count(), 0);
	pmm_fail_after(-5);
	h_true(fake_mixed_alloc(0) != 0, "toute valeur negative desarme");
}

int	main(void)
{
	h_begin("a02/fail_after");
	h_run("fail_after/limite : n = 0", fail_zero);
	h_run("fail_after/table : chaque rang 0 a 9", fail_every_rank);
	h_run("fail_after/etat : l'echec persiste", fail_stays_failing);
	return (h_end());
}
