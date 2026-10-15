#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include "alloc_int.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"
#include "velum/vtls.h"

#define MT_THREADS 8
#define MT_OPS 50000

static t_fa_job	g_jobs[MT_THREADS];

static void	mt_run_all(void)
{
	pthread_t	th[MT_THREADS];
	int			i;

	i = 0;
	while (i < MT_THREADS)
	{
		g_jobs[i].seed = 0x9e3779b97f4a7c15ull + (uint64_t)i * 7919;
		g_jobs[i].ops = MT_OPS;
		g_jobs[i].errors = 0;
		pthread_create(&th[i], NULL, fa_job_thread, &g_jobs[i]);
		i++;
	}
	while (i--)
		pthread_join(th[i], NULL);
}

static void	mt_contention(void)
{
	t_vheapstats	st;
	uint32_t		errors;
	int				i;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	fa_hook_install();
	printf("graine mt %llu\n", 0x9e3779b97f4a7c15ull);
	mt_run_all();
	errors = 0;
	i = 0;
	while (i < MT_THREADS)
		errors += g_jobs[i++].errors;
	v_heap_stats(&st);
	h_eq_u64("contenus intacts", errors, 0);
	h_eq_u64("live_blocks", st.live_blocks, 0);
	h_eq_u64("live_bytes", st.live_bytes, 0);
	h_eq_u64("large_bytes", st.large_bytes, 0);
	h_eq_i64("aucune faute", fa_hook_count(), 0);
	h_eq_u64("vfree invalides", g_fsys.vfree_bad, 0);
}

int	main(void)
{
	h_begin("a14/malloc_mt");
	h_run("malloc/concurrence : 8 fils x 50 000 operations", mt_contention);
	return (h_end());
}
