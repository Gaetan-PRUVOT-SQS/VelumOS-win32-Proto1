#include "proc_int.h"

static int	reclaim_take(t_process *p, int from, t_thread **out)
{
	t_procext	*x;
	uint64_t	fl;
	int			i;

	x = proc_ext(p);
	fl = spin_lock_irqsave(&p->lock);
	i = from;
	while (i < PROC_THREADS_MAX && x->th[i].state != PT_DEAD)
		i++;
	if (i < PROC_THREADS_MAX)
	{
		x->th[i].state = PT_RECLAIM;
		*out = x->th[i].t;
	}
	spin_unlock_irqrestore(&p->lock, fl);
	return (i);
}

static void	reclaim_done(t_process *p, int i, int joined)
{
	t_procext	*x;
	uint64_t	fl;

	x = proc_ext(p);
	fl = spin_lock_irqsave(&p->lock);
	if (joined)
	{
		x->th[i].state = PT_FREE;
		x->th[i].t = NULL;
	}
	else
		x->th[i].state = PT_DEAD;
	spin_unlock_irqrestore(&p->lock, fl);
}

void	proc_thread_release(t_thread *t)
{
	t_object	*o;

	o = t->obj;
	t->obj = NULL;
	if (o)
		obj_unref(o);
	thread_unref(t);
}

void	proc_reclaim(t_process *p)
{
	t_thread	*t;
	int			i;
	int			joined;

	t = NULL;
	i = reclaim_take(p, 0, &t);
	while (i < PROC_THREADS_MAX)
	{
		joined = (thread_join(t, 0) == 0);
		reclaim_done(p, i, joined);
		if (joined)
			proc_thread_release(t);
		i = reclaim_take(p, i + 1, &t);
	}
}
