#include "sched_int.h"
#include "velum/err.h"
#include "velum/timer.h"

static void	sleep_by_yield(uint64_t start, uint64_t ns)
{
	while (time_now_ns() - start < ns && !thread_cancel_pending())
		sched_yield();
}

void	sched_sleep_ns(uint64_t ns)
{
	t_waitq		wq;
	uint64_t	start;
	int			rc;

	if (ns == 0)
	{
		sched_yield();
		return ;
	}
	start = time_now_ns();
	waitq_init(&wq);
	rc = waitq_wait(&wq, NULL, ns);
	if (rc == E_NOMEM)
		sleep_by_yield(start, ns);
}

void	sched_block(void)
{
	sched_park();
}

void	sched_wake(t_thread *t)
{
	if (t)
		sched_unpark((t_kthread *)t, true);
}
