#include "proc_int.h"

static int	slot_reserve(t_process *p)
{
	t_procext	*x;
	uint64_t	fl;
	int			i;

	x = proc_ext(p);
	fl = spin_lock_irqsave(&p->lock);
	i = 0;
	while (i < PROC_THREADS_MAX && x->th[i].state != PT_FREE)
		i++;
	if (i == PROC_THREADS_MAX || p->state != PS_RUNNING)
		i = -1;
	else
		x->th[i].state = PT_RESERVED;
	spin_unlock_irqrestore(&p->lock, fl);
	return (i);
}

static void	slot_fill(t_process *p, int i, t_thread *t, const t_pthread *st)
{
	t_procext	*x;
	uint64_t	fl;

	x = proc_ext(p);
	fl = spin_lock_irqsave(&p->lock);
	if (t)
	{
		x->th[i].t = t;
		x->th[i].stack_va = st->stack_va;
		x->th[i].stack_len = st->stack_len;
		x->th[i].state = PT_LIVE;
		t->proc_next = p->threads;
		p->threads = t;
		p->nthreads++;
	}
	else
		x->th[i].state = PT_FREE;
	spin_unlock_irqrestore(&p->lock, fl);
}

t_thread	*proc_thread_add(t_process *p, const t_threadreq *rq,
				const t_pthread *st)
{
	t_thread	*t;
	int			i;

	proc_reclaim(p);
	i = slot_reserve(p);
	if (i < 0)
		return (NULL);
	sched_preempt_disable();
	t = thread_create(rq);
	if (t)
		thread_ref(t);
	if (t && obj_thread_new)
		t->obj = obj_thread_new(t);
	slot_fill(p, i, t, st);
	sched_preempt_enable();
	return (t);
}
