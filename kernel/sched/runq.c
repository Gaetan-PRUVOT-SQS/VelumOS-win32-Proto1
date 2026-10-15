#include "sched_int.h"
#include "velum/libk.h"

void	runq_init(t_runq *rq)
{
	memset(rq, 0, sizeof(*rq));
}

static void	runq_push_head(t_runq *rq, t_kthread *kt, int32_t p)
{
	kt->t.prev = NULL;
	kt->t.next = (t_thread *)rq->head[p];
	if (rq->head[p])
		rq->head[p]->t.prev = &kt->t;
	else
		rq->tail[p] = kt;
	rq->head[p] = kt;
}

static void	runq_push_tail(t_runq *rq, t_kthread *kt, int32_t p)
{
	kt->t.next = NULL;
	kt->t.prev = (t_thread *)rq->tail[p];
	if (rq->tail[p])
		rq->tail[p]->t.next = &kt->t;
	else
		rq->head[p] = kt;
	rq->tail[p] = kt;
}

void	runq_push(t_runq *rq, t_kthread *kt, bool at_head)
{
	int32_t	p;

	p = kt->t.prio;
	if (p < 0)
		p = 0;
	if (p > PRIO_MAX)
		p = PRIO_MAX;
	kt->t.prio = p;
	if (at_head)
		runq_push_head(rq, kt, p);
	else
		runq_push_tail(rq, kt, p);
	rq->bitmap |= 1u << p;
	rq->count++;
}
