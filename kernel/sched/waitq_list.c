#include "sched_int.h"

void	waitq_init(t_waitq *wq)
{
	spin_init(&wq->lock, "waitq");
	wq->head = NULL;
	wq->tail = NULL;
}

void	waitq_list_append(t_waitq *wq, t_kthread *kt)
{
	kt->t.next = NULL;
	kt->t.prev = wq->tail;
	if (wq->tail)
		wq->tail->next = &kt->t;
	else
		wq->head = &kt->t;
	wq->tail = &kt->t;
}

void	waitq_list_remove(t_waitq *wq, t_kthread *kt)
{
	if (kt->t.prev)
		kt->t.prev->next = kt->t.next;
	else
		wq->head = kt->t.next;
	if (kt->t.next)
		kt->t.next->prev = kt->t.prev;
	else
		wq->tail = kt->t.prev;
	kt->t.next = NULL;
	kt->t.prev = NULL;
}

t_kthread	*waitq_list_pop(t_waitq *wq)
{
	t_kthread	*kt;

	kt = (t_kthread *)wq->head;
	if (kt)
		waitq_list_remove(wq, kt);
	return (kt);
}

int32_t	waitq_list_max_prio(t_waitq *wq)
{
	uint64_t	flags;
	t_thread	*t;
	int32_t		best;

	best = -1;
	flags = spin_lock_irqsave(&wq->lock);
	t = wq->head;
	while (t)
	{
		if (t->prio > best)
			best = t->prio;
		t = t->next;
	}
	spin_unlock_irqrestore(&wq->lock, flags);
	return (best);
}
