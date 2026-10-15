#include "sched_st.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/timer.h"

static t_stpre	g_pre;

static void	pre_low(void *arg)
{
	t_stpre	*p;

	p = arg;
	while (!p->flag)
	{
		sched_preempt_disable();
		sched_preempt_enable();
	}
	p->seen_ns = time_now_ns();
}

static void	pre_high(void *arg)
{
	t_stpre	*p;

	p = arg;
	p->target_ns = time_now_ns() + ST_PREEMPT_SLEEP_NS;
	sched_sleep_ns(ST_PREEMPT_SLEEP_NS);
	p->woke_ns = time_now_ns();
	p->flag = 1;
}

static int	pre_judge(t_stpre *p)
{
	uint64_t	late;

	late = 0;
	if (p->woke_ns > p->target_ns)
		late = p->woke_ns - p->target_ns;
	klog_info("sched: préemption, réveil du fil PRIO_HIGH en retard de "
		"%llu us", late / 1000);
	if (p->woke_ns < p->target_ns || late > ST_PREEMPT_MAX_NS)
		return (E_RANGE);
	if (p->seen_ns < p->woke_ns)
		return (E_RANGE);
	return (0);
}

int	st_preempt(void)
{
	t_thread	*low;
	t_thread	*high;
	int			rc;

	g_pre.flag = 0;
	g_pre.seen_ns = 0;
	if (st_spawn(pre_low, &g_pre, PRIO_NORMAL, &low) != 0)
		return (E_NOMEM);
	if (st_spawn(pre_high, &g_pre, PRIO_HIGH, &high) != 0)
	{
		g_pre.flag = 1;
		st_join_unref(low);
		return (E_NOMEM);
	}
	rc = st_join_unref(high);
	if (st_join_unref(low) != 0)
		rc = E_TIMEOUT;
	if (rc == 0)
		rc = pre_judge(&g_pre);
	return (rc);
}
