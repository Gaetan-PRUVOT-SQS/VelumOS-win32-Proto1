#include <stdint.h>
#include <stdio.h>
#include "alloc_int.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

static void	churn_random(void)
{
	static t_fa_job	job;
	t_vheapstats	st;
	uint32_t		i;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	job.seed = 0xfeedfaceull;
	printf("graine churn %llu\n", (unsigned long long)job.seed);
	i = 0;
	while (i++ < 100000)
		fa_job_step(&job);
	fa_job_drain(&job);
	v_heap_stats(&st);
	h_eq_u64("contenu intact", job.errors, 0);
	h_eq_u64("live_blocks", st.live_blocks, 0);
	h_eq_u64("live_bytes", st.live_bytes, 0);
	h_eq_u64("large_bytes", st.large_bytes, 0);
	h_eq_i64("aucune faute", fa_hook_count(), 0);
	h_eq_u64("vfree invalides", g_fsys.vfree_bad, 0);
}

static void	churn_large_cycles(void)
{
	void	*p;
	int		i;

	fake_reset();
	fake_kernel_on();
	i = 0;
	while (i++ < 1000)
	{
		p = malloc(100000);
		free(p);
	}
	h_eq_u64("valloc = vfree", g_fsys.valloc_calls, g_fsys.vfree_calls);
	h_eq_u64("aucun mappage vivant", g_fsys.live_maps, 0);
	h_eq_u64("aucun octet vivant", g_fsys.live_bytes, 0);
	h_eq_u64("large_bytes", g_alloc.stats.large_bytes, 0);
}

static void	churn_stats_zero(void)
{
	t_vheapstats	st;
	void			*p;

	fake_reset();
	fake_kernel_on();
	p = malloc(100);
	free(p);
	p = malloc(100000);
	free(p);
	v_heap_stats(&st);
	h_eq_u64("live_blocks", st.live_blocks, 0);
	h_eq_u64("live_bytes", st.live_bytes, 0);
	h_eq_u64("alloc_calls = free_calls", st.alloc_calls, st.free_calls);
}

int	main(void)
{
	h_begin("a14/malloc_churn");
	h_run("malloc/aleatoire : 100 000 operations", churn_random);
	h_run("malloc/cycles : gros blocs", churn_large_cycles);
	h_run("malloc/etat : compteurs a zero", churn_stats_zero);
	return (h_end());
}
