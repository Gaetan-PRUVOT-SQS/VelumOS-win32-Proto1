#include "obj_int.h"

void	wblock_fire(t_wblock *wb)
{
	uint64_t	fl;

	fl = spin_lock_irqsave(&wb->lock);
	wb->fired = 1;
	waitq_wake_all(&wb->wq);
	spin_unlock_irqrestore(&wb->lock, fl);
}

static void	pulse_link(t_wlink *l)
{
	__atomic_store_n(&l->pulsed, 1, __ATOMIC_RELEASE);
	wblock_fire(l->wb);
}

void	wreg_pulse(t_object *o, bool all)
{
	t_wlink		*l;
	t_wlink		*oldest;
	uint64_t	fl;

	fl = spin_lock_irqsave(&g_wreg.lock);
	l = g_wreg.heads[wreg_bucket(o)];
	oldest = NULL;
	while (l)
	{
		if (l->obj == o && all)
			pulse_link(l);
		if (l->obj == o)
			oldest = l;
		l = l->next;
	}
	if (oldest && !all)
		pulse_link(oldest);
	spin_unlock_irqrestore(&g_wreg.lock, fl);
}
