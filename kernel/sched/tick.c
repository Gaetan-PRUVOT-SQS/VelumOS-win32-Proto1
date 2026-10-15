#include "sched_int.h"
#include "velum/timer.h"

void	sched_quantum_cb(void *ctx)
{
	t_cpusched	*cs;

	cs = ctx;
	__atomic_store_n(&cs->qtimer_pending, 0, __ATOMIC_RELEASE);
	sched_tick(time_now_ns());
}

void	sched_tick(uint64_t now_ns)
{
	t_cpusched	*cs;
	t_kthread	*cur;
	uint64_t	flags;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return ;
	cs = sched_cpu();
	flags = spin_lock_irqsave(&cs->lock);
	cur = (t_kthread *)cs->cpu->current;
	if (policy_tick(&cs->rq, cur, now_ns, &cs->slice_start))
		__atomic_store_n(&cs->need_resched, 1, __ATOMIC_RELEASE);
	spin_unlock_irqrestore(&cs->lock, flags);
}

void	sched_set_quantum_ns(uint64_t ns)
{
	t_cpusched	*cs;
	uint64_t	flags;

	if (ns < QUANTUM_MIN_NS)
		ns = QUANTUM_MIN_NS;
	if (ns > QUANTUM_MAX_NS)
		ns = QUANTUM_MAX_NS;
	cs = &g_sched.bsp;
	flags = spin_lock_irqsave(&cs->lock);
	cs->quantum_ns = ns;
	spin_unlock_irqrestore(&cs->lock, flags);
}
