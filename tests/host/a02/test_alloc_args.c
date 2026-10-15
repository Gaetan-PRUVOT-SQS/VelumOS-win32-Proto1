#include "velum/err.h"
#include "a02_fake.h"

static const int		g_owner[] = {PMM_KERNEL, PMM_FREE, PMM_OWNERS, -1,
	PMM_KERNEL, PMM_KERNEL, PMM_KERNEL, PMM_KERNEL, PMM_KERNEL, PMM_KERNEL,
	PMM_KERNEL, PMM_KERNEL, PMM_KERNEL, PMM_KERNEL, PMM_KERNEL};
static const uint64_t	g_n[] = {1, 1, 1, 1, 0, 1281, 1024, 1, 1, 1,
	UINT64_MAX / 2, 1, 1, 1, 1};
static const uint64_t	g_al[] = {1, 1, 1, 1, 1, 1, 1, 3, 0, 1ull << 30,
	1, 1, 6, UINT64_MAX, 1};
static const uint64_t	g_max[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0xfff};
static const int		g_ok[] = {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};

static void	args_row(size_t i)
{
	t_pmm_stats	a;
	t_pmm_stats	b;
	uint64_t	phys;

	pmm_get_stats(&a);
	phys = pmm_alloc_pages((t_pmm_owner)g_owner[i], g_n[i], g_al[i], g_max[i]);
	pmm_get_stats(&b);
	h_eq_i64("succes attendu", phys != 0, g_ok[i]);
	h_eq_u64("alloc_calls", b.alloc_calls, a.alloc_calls + 1);
	h_eq_u64("fail_calls", b.fail_calls, a.fail_calls + !g_ok[i]);
	if (phys)
		pmm_free_pages(phys, g_n[i], (t_pmm_owner)g_owner[i]);
	h_eq_u64("pool inchange", fake_free_pages(), a.free_pages);
}

static void	args_decision_table(void)
{
	size_t	i;

	h_eq_i64("boot", fake_simple(1, 5), 0);
	h_eq_u64("span 1280", g_pmm.span, 1280);
	i = 0;
	while (i < sizeof(g_n) / sizeof(g_n[0]))
	{
		args_row(i);
		i++;
	}
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	args_counters(void)
{
	t_pmm_stats	s;
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 5), 0);
	phys = pmm_alloc(PMM_USER);
	pmm_free_pages(phys, 0, PMM_USER);
	pmm_free(phys, PMM_USER);
	pmm_alloc_pages(PMM_USER, 0, 1, 0);
	pmm_get_stats(&s);
	h_eq_u64("alloc_calls : valides et invalides", s.alloc_calls, 2);
	h_eq_u64("fail_calls", s.fail_calls, 1);
	h_eq_u64("free_calls : sans la liberation de 0 page", s.free_calls, 1);
	h_eq_u64("owned[PMM_FREE] = libres", s.owned[PMM_FREE], s.free_pages);
	h_eq_u64("total = libres + proprietaires", s.total_pages,
		s.free_pages + s.owned[PMM_USER]);
	h_eq_u64("span = total + reserve", g_pmm.span,
		s.total_pages + s.reserved_pages);
}

int	main(void)
{
	h_begin("a02/alloc_args");
	h_run("alloc/table : validite des arguments", args_decision_table);
	h_run("stats/etat : compteurs d'appels", args_counters);
	return (h_end());
}
