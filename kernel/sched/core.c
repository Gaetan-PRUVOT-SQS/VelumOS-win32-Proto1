#include "sched_int.h"

t_schedglob	g_sched;

t_cpusched	*sched_cpu(void)
{
	return (cpu_self()->sched);
}

t_kthread	*sched_kself(void)
{
	t_thread	*cur;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return (&g_sched.boot);
	cur = cpu_self()->current;
	if (!cur)
		return (&g_sched.boot);
	return ((t_kthread *)cur);
}

t_thread	*sched_current(void)
{
	return (&sched_kself()->t);
}

t_lockstack	*sched_lockstack(void)
{
	t_cpusched	*cs;

	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return (NULL);
	cs = sched_cpu();
	if (!cs)
		return (NULL);
	return (&cs->locks);
}

uint32_t	sched_preempt_count(void)
{
	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE))
		return (0);
	return (cpu_self()->preempt);
}
