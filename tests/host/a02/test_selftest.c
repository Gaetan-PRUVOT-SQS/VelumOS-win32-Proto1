#include <string.h>
#include "velum/err.h"
#include "a02_fake.h"

static void	selftest_without_low_memory(void)
{
	t_pmm_stats	a;
	t_pmm_stats	b;
	int			o;

	fake_cmdline(NULL);
	h_eq_i64("boot", fake_simple(1, 64), 0);
	pmm_get_stats(&a);
	h_eq_i64("autotest", pmm_selftest(), 0);
	pmm_get_stats(&b);
	h_eq_u64("libres inchanges", b.free_pages, a.free_pages);
	o = PMM_KERNEL;
	while (o < PMM_OWNERS)
	{
		h_eq_u64("proprietaire inchange", b.owned[o], a.owned[o]);
		o++;
	}
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	selftest_with_low_memory(void)
{
	fake_cmdline(NULL);
	h_eq_i64("boot", fake_simple(0, 64), 0);
	h_eq_i64("autotest avec zone basse", pmm_selftest(), 0);
	h_eq_i64("invariants", pmm_check(), 0);
	h_eq_i64("aucune assertion", fake_assert_count(), 0);
}

static void	selftest_reports_failures(void)
{
	fake_cmdline(NULL);
	h_eq_i64("boot", fake_simple(1, 64), 0);
	pmm_fail_after(0);
	h_true(pmm_selftest() != 0, "echec d'allocation vu par l'autotest");
	h_true(strstr(fake_last_log(), "alloc et") != NULL, "journal de l'etape");
	pmm_fail_after(-1);
	g_pmm.stats.free_pages++;
	h_true(pmm_selftest() != 0, "compteurs faux vus par l'autotest");
	h_true(strstr(fake_last_log(), "invariants") != NULL, "journal");
}

int	main(void)
{
	h_begin("a02/selftest");
	h_run("autotest/nominal : RAM sans zone basse",
		selftest_without_low_memory);
	h_run("autotest/nominal : RAM avec zone basse", selftest_with_low_memory);
	h_run("autotest/supposition : echecs signales", selftest_reports_failures);
	return (h_end());
}
