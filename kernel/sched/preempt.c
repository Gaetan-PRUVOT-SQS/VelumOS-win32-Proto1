#include "sched_int.h"
#include "velum/panic.h"

void	sched_preempt_disable(void)
{
	t_cpu	*cpu;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return ;
	cpu = cpu_self();
	cpu->preempt++;
	__atomic_signal_fence(__ATOMIC_SEQ_CST);
}

void	sched_preempt_enable(void)
{
	t_cpu		*cpu;
	t_cpusched	*cs;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return ;
	cpu = cpu_self();
	__atomic_signal_fence(__ATOMIC_SEQ_CST);
	if (cpu->preempt == 0)
		panic("sched: préemption réactivée sans désactivation (appelant %p)",
			__builtin_return_address(0));
	cpu->preempt--;
	cs = cpu->sched;
	if (cpu->preempt == 0 && __atomic_load_n(&cs->need_resched,
			__ATOMIC_ACQUIRE) && arch_irqs_enabled())
		sched_resched(SR_PREEMPT);
}

void	sched_irq_exit(void)
{
	t_cpu		*cpu;
	t_cpusched	*cs;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return ;
	cpu = cpu_self();
	cs = cpu->sched;
	if (cpu->preempt == 0 && __atomic_load_n(&cs->need_resched,
			__ATOMIC_ACQUIRE))
		sched_resched(SR_PREEMPT);
}

void	sched_yield(void)
{
	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return ;
	sched_resched(SR_YIELD);
}
