#include "sched_int.h"
#include "velum/timer.h"

void	sched_quantum_cb(void *ctx)
{
	t_cpusched	*cs;

	cs = ctx;
	__atomic_store_n(&cs->qtimer_pending, 0, __ATOMIC_RELEASE);
	sched_tick_cs(cs, time_now_ns());
}

void	sched_tick_cs(t_cpusched *cs, uint64_t now_ns)
{
	t_kthread	*cur;
	uint64_t	flags;
	bool		waiting;
	bool		resched;

	flags = spin_lock_irqsave(&cs->lock);
	cur = (t_kthread *)cs->cpu->current;
	resched = policy_tick(&cs->rq, cur, now_ns, &cs->slice_start);
	if (resched)
		__atomic_store_n(&cs->need_resched, 1, __ATOMIC_RELEASE);
	waiting = cs->rq.count > 0 && !(cur->kflags & KT_IDLE);
	spin_unlock_irqrestore(&cs->lock, flags);
	if (resched)
		sched_kick(cs);
	if (waiting)
		sched_kick_idle(cs);
}

void	sched_tick(uint64_t now_ns)
{
	t_cpusched	*cs;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return ;
	cs = sched_cpu();
	if (cs)
		sched_tick_cs(cs, now_ns);
}

void	sched_set_quantum_ns(uint64_t ns)
{
	t_cpusched	*cs;
	uint64_t	flags;
	uint32_t	i;

	if (ns < QUANTUM_MIN_NS)
		ns = QUANTUM_MIN_NS;
	if (ns > QUANTUM_MAX_NS)
		ns = QUANTUM_MAX_NS;
	i = 0;
	while (i < __atomic_load_n(&g_sched.span, __ATOMIC_ACQUIRE))
	{
		cs = g_sched.cpus[i++];
		if (!cs)
			continue ;
		flags = spin_lock_irqsave(&cs->lock);
		cs->quantum_ns = ns;
		spin_unlock_irqrestore(&cs->lock, flags);
	}
}
