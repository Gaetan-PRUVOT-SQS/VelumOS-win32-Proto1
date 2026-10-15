#include "sched_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/panic.h"
#include "velum/timer.h"

bool	wq_enter(t_waitq *wq, t_kthread *kt, bool cancelable)
{
	sched_park_prepare(kt);
	__atomic_store_n(&kt->reason, 0, __ATOMIC_SEQ_CST);
	if (cancelable && __atomic_load_n(&kt->canceled, __ATOMIC_SEQ_CST))
		return (false);
	kt->cancelable = cancelable;
	kt->t.wait_result = 0;
	waitq_list_append(wq, kt);
	kt->wq = wq;
	return (true);
}

static void	wq_timeout_cb(void *ctx)
{
	t_kthread	*kt;
	int32_t		zero;

	kt = ctx;
	zero = 0;
	if (__atomic_compare_exchange_n(&kt->reason, &zero, E_TIMEOUT, false,
			__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		sched_unpark(kt, true);
	__atomic_store_n(&kt->tmo_done, 1, __ATOMIC_RELEASE);
}

void	wq_arm(t_kthread *kt, uint64_t deadline)
{
	int64_t	id;
	int32_t	zero;

	kt->tmo_id = 0;
	__atomic_store_n(&kt->tmo_done, 0, __ATOMIC_RELEASE);
	if (deadline == TIMEOUT_NONE)
		return ;
	id = timer_arm(deadline, wq_timeout_cb, kt);
	if (id > 0)
	{
		kt->tmo_id = id;
		return ;
	}
	zero = 0;
	if (__atomic_compare_exchange_n(&kt->reason, &zero, E_NOMEM, false,
			__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		sched_unpark(kt, false);
}

void	wq_disarm(t_kthread *kt)
{
	uint64_t	start;

	if (kt->tmo_id <= 0)
		return ;
	if (!timer_cancel(kt->tmo_id))
	{
		start = time_now_ns();
		while (!__atomic_load_n(&kt->tmo_done, __ATOMIC_ACQUIRE))
		{
			cpu_relax();
			if (time_now_ns() - start > SPIN_STUCK_NS)
				panic("waitq: minuterie %lld du fil %u jamais terminée",
					kt->tmo_id, kt->t.tid);
		}
	}
	kt->tmo_id = 0;
}

void	thread_cancel(t_thread *t)
{
	t_kthread	*kt;
	int32_t		zero;

	if (!t)
		return ;
	kt = (t_kthread *)t;
	__atomic_store_n(&kt->canceled, 1, __ATOMIC_SEQ_CST);
	zero = 0;
	if (__atomic_compare_exchange_n(&kt->reason, &zero, E_CANCELED, false,
			__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		sched_unpark(kt, false);
}
