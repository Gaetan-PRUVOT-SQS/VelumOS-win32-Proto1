#include "sched_int.h"

void	runq_remove(t_runq *rq, t_kthread *kt)
{
	int32_t	p;

	p = kt->t.prio;
	if (kt->t.prev)
		kt->t.prev->next = kt->t.next;
	else
		rq->head[p] = (t_kthread *)kt->t.next;
	if (kt->t.next)
		kt->t.next->prev = kt->t.prev;
	else
		rq->tail[p] = (t_kthread *)kt->t.prev;
	kt->t.next = NULL;
	kt->t.prev = NULL;
	if (!rq->head[p])
		rq->bitmap &= ~(1u << p);
	rq->count--;
}

int32_t	runq_top(const t_runq *rq)
{
	if (rq->bitmap == 0)
		return (-1);
	return (31 - __builtin_clz(rq->bitmap));
}

t_kthread	*runq_pop_top(t_runq *rq)
{
	int32_t		p;
	t_kthread	*kt;

	p = runq_top(rq);
	if (p < 0)
		return (NULL);
	kt = rq->head[p];
	runq_remove(rq, kt);
	return (kt);
}
