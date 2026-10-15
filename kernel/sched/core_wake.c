#include "sched_int.h"
#include "velum/irqflags.h"
#include "velum/panic.h"
#include "velum/timer.h"

void	sched_unpark_locked(t_cpusched *cs, t_kthread *kt, bool boost)
{
	t_kthread	*cur;

	if (kt->t.state != TS_BLOCKED && kt->t.state != TS_NEW)
	{
		kt->token = 1;
		return ;
	}
	kt->t.state = TS_READY;
	kt->ready_ns = time_now_ns();
	kt->t.quantum_left = (uint32_t)cs->quantum_ns;
	if (boost)
		policy_wake_boost(kt);
	kt->t.prio = policy_prio(kt);
	runq_push(&cs->rq, kt, false);
	cur = (t_kthread *)cs->cpu->current;
	if (policy_should_preempt(cur, kt)
		|| !__atomic_load_n(&cs->qtimer_pending, __ATOMIC_ACQUIRE))
		__atomic_store_n(&cs->need_resched, 1, __ATOMIC_RELEASE);
}

void	sched_unpark(t_kthread *kt, bool boost)
{
	t_cpusched	*cs;
	uint64_t	flags;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
	{
		kt->token = 1;
		return ;
	}
	cs = sched_cpu();
	flags = spin_lock_irqsave(&cs->lock);
	sched_unpark_locked(cs, kt, boost);
	spin_unlock_irqrestore(&cs->lock, flags);
}

void	sched_park(void)
{
	t_cpusched	*cs;
	t_kthread	*kt;
	uint64_t	flags;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		panic("sched: attente bloquante avant sched_boot_init (appelant %p)",
			__builtin_return_address(0));
	cs = sched_cpu();
	kt = sched_kself();
	if (cs->cpu->irq_depth != 0 || (kt->kflags & KT_IDLE))
		panic("sched: attente bloquante interdite ici (fil %u, irq %u)",
			kt->t.tid, cs->cpu->irq_depth);
	flags = spin_lock_irqsave(&cs->lock);
	if (kt->token)
		kt->token = 0;
	else
	{
		kt->t.state = TS_BLOCKED;
		sched_pass_locked(cs, SR_BLOCK);
	}
	spin_unlock(&cs->lock);
	sched_finish(cs);
	irq_restore(flags);
}

void	sched_park_prepare(t_kthread *kt)
{
	t_cpusched	*cs;
	uint64_t	flags;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
	{
		kt->token = 0;
		return ;
	}
	cs = sched_cpu();
	flags = spin_lock_irqsave(&cs->lock);
	kt->token = 0;
	spin_unlock_irqrestore(&cs->lock, flags);
}
