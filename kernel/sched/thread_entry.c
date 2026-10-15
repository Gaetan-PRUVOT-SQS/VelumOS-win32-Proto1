#include "sched_int.h"
#include "velum/arch.h"
#include "velum/irqflags.h"

void	sched_thread_entry(t_kthread *kt)
{
	t_cpusched	*cs;

	cs = sched_cpu();
	spin_unlock(&cs->lock);
	sched_finish(cs);
	if (kt->t.flags & THREAD_USER)
		arch_enter_user(kt->entry, kt->user_sp, (uint64_t)kt->arg);
	irq_enable();
	kt->fn(kt->arg);
	thread_exit(0);
}

void	sched_idle_main(void *arg)
{
	t_cpusched	*cs;

	cs = arg;
	while (1)
	{
		irq_disable();
		if (__atomic_load_n(&cs->need_resched, __ATOMIC_ACQUIRE)
			|| __atomic_load_n(&cs->rq.count, __ATOMIC_ACQUIRE) > 0)
		{
			irq_enable();
			sched_resched(SR_YIELD);
		}
		else
			arch_idle_wait();
	}
}

void	sched_idle(void)
{
	t_kthread	*kt;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		arch_halt_forever();
	kt = sched_kself();
	if (kt->kflags & KT_IDLE)
		sched_idle_main(sched_cpu());
	thread_exit(0);
}
