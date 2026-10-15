#include "sched_st.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

static int	st_report(const char *name, int rc)
{
	if (rc != 0)
		klog_err("sched: autotest '%s' en échec (%d)", name, rc);
	else
		klog_info("sched: autotest '%s' ok", name);
	return (rc != 0);
}

int	sched_selftest(void)
{
	int	fails;

	fails = 0;
	fails += st_report("équité", st_fair());
	fails += st_report("préemption", st_preempt());
	fails += st_report("ping-pong", st_pingpong());
	fails += st_report("sommeil", st_sleep());
	fails += st_report("mutex", st_mutex());
	fails += st_report("join", st_join());
	fails += st_report("création en masse", st_churn());
	fails += st_report("échecs d'allocation", st_failpath());
	klog_info("sched: %llu bascules depuis le démarrage",
		g_sched.bsp.switches);
	return (fails);
}

int	st_spawn(t_threadfn fn, void *arg, int32_t prio, t_thread **out)
{
	t_threadreq	rq;

	memset(&rq, 0, sizeof(rq));
	rq.name = "autotest";
	rq.fn = fn;
	rq.arg = arg;
	rq.prio = prio;
	*out = thread_create(&rq);
	if (!*out)
		return (E_NOMEM);
	return (0);
}

int	st_join_unref(t_thread *t)
{
	int	rc;

	rc = thread_join(t, ST_JOIN_NS);
	if (rc == 0)
		thread_unref(t);
	return (rc);
}
