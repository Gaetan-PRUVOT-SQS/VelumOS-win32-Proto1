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

t_cpusched	*sched_lock_home(t_kthread *kt, uint64_t *flags)
{
	t_cpusched	*cs;

	while (1)
	{
		cs = g_sched.cpus[__atomic_load_n(&kt->home, __ATOMIC_ACQUIRE)];
		*flags = spin_lock_irqsave(&cs->lock);
		if (__atomic_load_n(&kt->home, __ATOMIC_ACQUIRE) == cs->id)
			return (cs);
		spin_unlock_irqrestore(&cs->lock, *flags);
	}
}

static void	unpark_place_new(t_kthread *kt)
{
	uint32_t	best;

	if (kt->t.state != TS_NEW || (kt->kflags & KT_PINNED))
		return ;
	best = sched_least_loaded(g_sched.cpus,
			__atomic_load_n(&g_sched.span, __ATOMIC_ACQUIRE));
	if (best < g_sched.span)
		__atomic_store_n(&kt->home, best, __ATOMIC_RELEASE);
}

void	sched_unpark(t_kthread *kt, bool boost)
{
	t_cpusched	*cs;
	uint64_t	flags;
	bool		kick;
	bool		crowded;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
	{
		kt->token = 1;
		return ;
	}
	unpark_place_new(kt);
	cs = sched_lock_home(kt, &flags);
	sched_unpark_locked(cs, kt, boost);
	kick = __atomic_load_n(&cs->need_resched, __ATOMIC_ACQUIRE) != 0;
	crowded = sched_load(cs) >= 2;
	spin_unlock_irqrestore(&cs->lock, flags);
	if (kick)
		sched_kick(cs);
	if (crowded)
		sched_kick_idle(cs);
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
		cs = sched_cpu();
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
