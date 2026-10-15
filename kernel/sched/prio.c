#include "sched_int.h"

void	sched_reprio_locked(t_cpusched *cs, t_kthread *kt)
{
	int32_t		p;
	t_kthread	*cur;

	p = policy_prio(kt);
	if (p == kt->t.prio)
		return ;
	if (kt->t.state == TS_READY)
	{
		runq_remove(&cs->rq, kt);
		kt->t.prio = p;
		runq_push(&cs->rq, kt, false);
	}
	else
		kt->t.prio = p;
	cur = (t_kthread *)cs->cpu->current;
	if ((kt->t.state == TS_READY && policy_should_preempt(cur, kt))
		|| (kt == cur && runq_top(&cs->rq) > p))
		__atomic_store_n(&cs->need_resched, 1, __ATOMIC_RELEASE);
}

static void	prio_apply(t_kthread *kt)
{
	t_cpusched	*cs;
	uint64_t	flags;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
	{
		kt->t.prio = policy_prio(kt);
		return ;
	}
	cs = sched_cpu();
	flags = spin_lock_irqsave(&cs->lock);
	sched_reprio_locked(cs, kt);
	spin_unlock_irqrestore(&cs->lock, flags);
}

void	thread_set_prio(t_thread *t, int prio)
{
	t_kthread	*kt;

	if (!t || prio < 0 || prio > PRIO_MAX)
		return ;
	kt = (t_kthread *)t;
	if (kt->kflags & KT_IDLE)
		return ;
	__atomic_store_n(&kt->t.base_prio, prio, __ATOMIC_RELEASE);
	prio_apply(kt);
}

void	sched_prio_inherit(t_kthread *owner, int32_t prio)
{
	if (!owner || prio <= owner->inherit)
		return ;
	__atomic_store_n(&owner->inherit, prio, __ATOMIC_RELEASE);
	prio_apply(owner);
}

void	sched_prio_uninherit(t_kthread *kt)
{
	if (kt->inherit == 0)
		return ;
	__atomic_store_n(&kt->inherit, 0, __ATOMIC_RELEASE);
	prio_apply(kt);
}
