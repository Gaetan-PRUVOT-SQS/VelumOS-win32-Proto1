#include <stdint.h>
#include "a04_fake.h"

static uint64_t	g_calls;
static uint64_t	g_maps;

static void	fail_baseline(void)
{
	t_script		s;
	t_heap_stats	st;

	a04_fresh();
	a04_script_run(&s);
	heap_get_stats(&st);
	g_calls = st.alloc_calls;
	g_maps = g_fake.maps;
	h_eq_u64("E8 reference : aucun echec", st.fail_calls, 0);
	h_true(g_calls > 30 && g_maps > 5, "E8 reference : script significatif");
	a04_script_end(&s);
	a04_drain("E8 reference");
}

static void	fail_at_each_allocation_rank(void)
{
	t_script		s;
	t_heap_stats	st;
	uint64_t		k;

	k = 0;
	while (k <= g_calls + 1)
	{
		a04_fresh();
		heap_fail_after((int64_t)k);
		a04_script_run(&s);
		heap_fail_after(-1);
		heap_get_stats(&st);
		h_true(k >= g_calls || st.fail_calls > 0, "E8 l'echec a eu lieu");
		a04_script_end(&s);
		a04_drain("E8 rang d'allocation");
		k++;
	}
}

static void	fail_at_each_backend_rank(void)
{
	t_script	s;
	uint64_t	k;

	k = 0;
	while (k <= g_maps + 1)
	{
		a04_fresh();
		fake_pages_fail_after((int64_t)k);
		a04_script_run(&s);
		fake_pages_fail_after(-1);
		a04_script_end(&s);
		a04_drain("E8 rang du vmm");
		k++;
	}
}

static void	fail_both_at_once(void)
{
	t_script	s;
	uint64_t	k;

	k = 0;
	while (k < 12)
	{
		a04_fresh();
		heap_fail_after((int64_t)(k * 5));
		fake_pages_fail_after((int64_t)(11 - k));
		a04_script_run(&s);
		heap_fail_after(-1);
		fake_pages_fail_after(-1);
		a04_script_end(&s);
		a04_drain("E8 deux injections");
		k++;
	}
}

int	main(void)
{
	h_begin("a04/echecs-rang");
	h_run("E8 reference sans echec", fail_baseline);
	h_run("E8 echec a chaque rang du tas", fail_at_each_allocation_rank);
	h_run("E8 refus a chaque rang du vmm", fail_at_each_backend_rank);
	h_run("E8 deux injections", fail_both_at_once);
	return (h_end());
}
