#include "fake_sched.h"

uint32_t	g_kicks;

void	sched_smp_kick(uint32_t cpu)
{
	(void)cpu;
	__atomic_fetch_add(&g_kicks, 1, __ATOMIC_RELAXED);
}
