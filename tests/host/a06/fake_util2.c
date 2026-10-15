#include "fake_sched.h"

void	fu_sem_waiter(void *arg)
{
	t_fuwaiter	*w;

	w = arg;
	w->kt = sched_kself();
	w->rc = sem_wait(w->sem, w->timeout);
	fh_lock();
	if (w->order)
		w->order[(*w->norder)++] = w->tag;
	fh_unlock();
}

void	fu_pi_waiter(void *arg)
{
	t_fuwaiter	*w;

	w = arg;
	w->kt = sched_kself();
	w->kt->t.base_prio = w->prio;
	w->kt->t.prio = w->prio;
	mutex_lock(w->m);
	w->owner_ok = (w->m->owner == &w->kt->t);
	w->seen_prio = w->kt->t.prio;
	fh_lock();
	if (w->order)
		w->order[(*w->norder)++] = w->tag;
	fh_unlock();
	mutex_unlock(w->m);
}

void	fu_wait_flag(volatile int *flag, int value)
{
	uint64_t	start;

	start = fh_now_ns();
	while (*flag != value && fh_now_ns() - start < 5000000000ull)
		fh_sleep_us(50);
}
