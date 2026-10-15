#include "sched_st.h"
#include "velum/err.h"
#include "velum/klog.h"

static t_stfair	g_fair;

static void	fair_worker(void *arg)
{
	t_stfair	*f;

	f = arg;
	while (!f->stop)
	{
		sched_preempt_disable();
		sched_preempt_enable();
	}
}

static int	fair_judge(t_stfair *f)
{
	uint64_t	lo;
	uint64_t	hi;
	uint64_t	sum;
	int			i;

	lo = UINT64_MAX;
	hi = 0;
	sum = 0;
	i = 0;
	while (i < ST_FAIR_THREADS)
	{
		if (f->t[i]->cpu_time_ns < lo)
			lo = f->t[i]->cpu_time_ns;
		if (f->t[i]->cpu_time_ns > hi)
			hi = f->t[i]->cpu_time_ns;
		sum += f->t[i]->cpu_time_ns;
		i++;
	}
	klog_info("sched: équité, temps CPU min %llu us, max %llu us, "
		"total %llu us", lo / 1000, hi / 1000, sum / 1000);
	if (sum == 0 || (hi - lo) * 100 * ST_FAIR_THREADS
		>= sum * ST_FAIR_TOLERANCE_PCT)
		return (E_RANGE);
	return (0);
}

static int	fair_finish(t_stfair *f, int n)
{
	int	rc;
	int	i;

	f->stop = 1;
	rc = 0;
	i = 0;
	while (i < n)
	{
		if (thread_join(f->t[i], ST_JOIN_NS) != 0)
			return (E_TIMEOUT);
		i++;
	}
	if (n == ST_FAIR_THREADS)
		rc = fair_judge(f);
	i = 0;
	while (i < n)
	{
		thread_unref(f->t[i]);
		i++;
	}
	return (rc);
}

int	st_fair(void)
{
	int	n;
	int	rc;

	g_fair.stop = 0;
	n = 0;
	while (n < ST_FAIR_THREADS)
	{
		if (st_spawn(fair_worker, &g_fair, PRIO_NORMAL, &g_fair.t[n]) != 0)
			break ;
		n++;
	}
	if (n == ST_FAIR_THREADS)
		sched_sleep_ns(ST_FAIR_RUN_NS);
	rc = fair_finish(&g_fair, n);
	if (rc == 0 && n < ST_FAIR_THREADS)
		rc = E_NOMEM;
	return (rc);
}
