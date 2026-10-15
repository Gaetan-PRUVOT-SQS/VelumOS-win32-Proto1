#include "sched_int.h"

t_kthread	*wq_wake_one(t_waitq *wq, int32_t result)
{
	uint64_t	flags;
	t_kthread	*kt;

	flags = spin_lock_irqsave(&wq->lock);
	kt = waitq_list_pop(wq);
	if (kt)
	{
		kt->wq = NULL;
		kt->t.wait_result = result;
		sched_unpark(kt, true);
	}
	spin_unlock_irqrestore(&wq->lock, flags);
	return (kt);
}

void	wq_wake_all_locked(t_waitq *wq, int32_t result)
{
	t_kthread	*kt;

	kt = waitq_list_pop(wq);
	while (kt)
	{
		kt->wq = NULL;
		kt->t.wait_result = result;
		sched_unpark(kt, true);
		kt = waitq_list_pop(wq);
	}
}

void	waitq_wake_one(t_waitq *wq)
{
	wq_wake_one(wq, 0);
}

void	waitq_wake_all(t_waitq *wq)
{
	uint64_t	flags;

	flags = spin_lock_irqsave(&wq->lock);
	wq_wake_all_locked(wq, 0);
	spin_unlock_irqrestore(&wq->lock, flags);
}

bool	thread_cancel_pending(void)
{
	return (__atomic_load_n(&sched_kself()->canceled, __ATOMIC_SEQ_CST) != 0);
}
