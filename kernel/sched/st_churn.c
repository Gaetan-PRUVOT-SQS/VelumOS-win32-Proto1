#include "sched_st.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/pmm.h"

static void	churn_noop(void *arg)
{
	(void)arg;
}

void	st_snapshot(t_stsnap *out)
{
	t_heap_stats	hs;
	t_pmm_stats		ps;

	heap_get_stats(&hs);
	pmm_get_stats(&ps);
	out->heap_objs = hs.allocs_live[HEAP_SCHED];
	out->heap_bytes = hs.bytes_live[HEAP_SCHED];
	out->pmm_free = ps.free_pages;
}

int	st_reaper_drain(void)
{
	t_cpusched	*cs;
	int			tries;

	cs = sched_cpu();
	tries = 0;
	while (tries < 2000)
	{
		if (__atomic_load_n(&cs->dead, __ATOMIC_ACQUIRE) == NULL
			&& cs->reaper->t.state == TS_BLOCKED)
			return (0);
		sched_sleep_ns(ST_MS);
		tries++;
	}
	return (E_TIMEOUT);
}

static int	churn_rounds(int n)
{
	t_thread	*t;
	int			i;

	i = 0;
	while (i < n)
	{
		if (st_spawn(churn_noop, NULL, PRIO_NORMAL, &t) != 0)
			return (E_NOMEM);
		if (st_join_unref(t) != 0)
			return (E_TIMEOUT);
		i++;
	}
	return (st_reaper_drain());
}

int	st_churn(void)
{
	t_stsnap	before;
	t_stsnap	after;
	int			rc;

	rc = churn_rounds(ST_CHURN_WARMUP);
	if (rc != 0)
		return (rc);
	st_snapshot(&before);
	rc = churn_rounds(ST_CHURN);
	st_snapshot(&after);
	klog_info("sched: %d fils, tas sched %llu -> %llu objets, pages libres "
		"%llu -> %llu", ST_CHURN, before.heap_objs, after.heap_objs,
		before.pmm_free, after.pmm_free);
	if (rc == 0 && (before.heap_objs != after.heap_objs
			|| before.heap_bytes != after.heap_bytes
			|| before.pmm_free != after.pmm_free))
		rc = E_RANGE;
	return (rc);
}
