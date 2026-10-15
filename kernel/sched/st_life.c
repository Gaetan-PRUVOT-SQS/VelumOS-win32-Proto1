#include "sched_st.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/timer.h"

static t_event	g_join_ev;

int	st_sleep(void)
{
	uint64_t	start;
	uint64_t	took;

	start = time_now_ns();
	sched_sleep_ns(ST_SLEEP_NS);
	took = time_now_ns() - start;
	klog_info("sched: sommeil de 20 ms mesuré à %llu us", took / 1000);
	if (took + ST_SLEEP_TOL_NS < ST_SLEEP_NS
		|| took > ST_SLEEP_NS + ST_SLEEP_TOL_NS)
		return (E_RANGE);
	return (0);
}

static void	join_exit42(void *arg)
{
	(void)arg;
	thread_exit(42);
}

static void	join_waiter(void *arg)
{
	event_wait(arg, TIMEOUT_NONE);
}

static int	join_timeout(void)
{
	t_thread	*t;
	int			rc;

	event_init(&g_join_ev, true, false);
	if (st_spawn(join_waiter, &g_join_ev, PRIO_NORMAL, &t) != 0)
		return (E_NOMEM);
	rc = thread_join(t, 10 * ST_MS);
	event_set(&g_join_ev);
	if (st_join_unref(t) != 0)
		return (E_TIMEOUT);
	if (rc != E_TIMEOUT)
		return (E_RANGE);
	return (0);
}

int	st_join(void)
{
	t_thread	*t;
	int			rc;

	if (thread_join(sched_current(), ST_MS) != E_DEADLK
		|| thread_join(NULL, ST_MS) != E_INVAL)
		return (E_RANGE);
	if (st_spawn(join_exit42, NULL, PRIO_NORMAL, &t) != 0)
		return (E_NOMEM);
	rc = thread_join(t, ST_JOIN_NS);
	if (rc == 0 && t->exit_code != 42)
		rc = E_RANGE;
	if (rc == 0)
		thread_unref(t);
	if (rc != 0)
		return (rc);
	return (join_timeout());
}
