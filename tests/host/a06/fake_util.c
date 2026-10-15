#include "fake_sched.h"
#include <string.h>
#include "velum/err.h"

void	fu_waiter(void *arg)
{
	t_fuwaiter	*w;
	uint64_t	flags;

	w = arg;
	w->kt = sched_kself();
	w->kt->t.base_prio = w->prio;
	w->kt->t.prio = w->prio;
	flags = 0;
	if (w->held)
		flags = spin_lock_irqsave(w->held);
	w->rc = waitq_wait(w->wq, w->held, w->timeout);
	if (w->held)
	{
		w->held_ok = (w->held->ticket != w->held->serving);
		spin_unlock_irqrestore(w->held, flags);
	}
	fh_lock();
	if (w->order)
		w->order[(*w->norder)++] = w->tag;
	fh_unlock();
}

uint32_t	fu_wq_len(t_waitq *wq)
{
	uint64_t	flags;
	uint32_t	n;
	t_thread	*t;

	flags = spin_lock_irqsave(&wq->lock);
	n = 0;
	t = wq->head;
	while (t)
	{
		n++;
		t = t->next;
	}
	spin_unlock_irqrestore(&wq->lock, flags);
	return (n);
}

bool	fu_wait_len(t_waitq *wq, uint32_t n)
{
	uint64_t	start;

	start = fh_now_ns();
	while (fu_wq_len(wq) != n)
	{
		if (fh_now_ns() - start > 5000000000ull)
			return (false);
		fh_sleep_us(100);
	}
	return (true);
}

void	fu_kt_init(t_kthread *kt, int32_t prio)
{
	memset(kt, 0, sizeof(*kt));
	kt->t.prio = prio;
	kt->t.base_prio = prio;
	kt->t.state = TS_READY;
}

void	fu_ev_waiter(void *arg)
{
	t_fuwaiter	*w;

	w = arg;
	w->kt = sched_kself();
	w->rc = event_wait(w->ev, w->timeout);
	fh_lock();
	if (w->order)
		w->order[(*w->norder)++] = w->tag;
	fh_unlock();
}
