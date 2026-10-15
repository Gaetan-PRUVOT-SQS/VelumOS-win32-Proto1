#include "sched_st.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/libk.h"
#include "velum/pmm.h"

static void	fail_noop(void *arg)
{
	(void)arg;
}

static bool	fail_bad_requests(void)
{
	t_threadreq	rq;

	if (thread_create(NULL))
		return (false);
	memset(&rq, 0, sizeof(rq));
	if (thread_create(&rq))
		return (false);
	rq.fn = fail_noop;
	rq.prio = PRIO_MAX + 1;
	if (thread_create(&rq))
		return (false);
	rq.prio = PRIO_NORMAL;
	rq.flags = 0x80;
	if (thread_create(&rq))
		return (false);
	rq.flags = THREAD_USER;
	return (thread_create(&rq) == NULL);
}

static bool	fail_injected(void)
{
	t_threadreq	rq;
	t_thread	*t;

	memset(&rq, 0, sizeof(rq));
	rq.fn = fail_noop;
	rq.prio = PRIO_NORMAL;
	heap_fail_after(0);
	t = thread_create(&rq);
	heap_fail_after(-1);
	if (t)
		return (false);
	pmm_fail_after(0);
	t = thread_create(&rq);
	pmm_fail_after(-1);
	return (t == NULL);
}

int	st_failpath(void)
{
	t_stsnap	before;
	t_stsnap	after;

	st_snapshot(&before);
	if (!fail_bad_requests())
		return (E_INVAL);
	if (!fail_injected())
		return (E_NOMEM);
	st_snapshot(&after);
	if (before.heap_objs != after.heap_objs
		|| before.pmm_free != after.pmm_free)
		return (E_RANGE);
	return (0);
}
