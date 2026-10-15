#include "velum/irqflags.h"
#include "a02_fake.h"

t_fakeirq	g_fakeirq;

uint64_t	irq_save(void)
{
	g_fakeirq.depth++;
	g_fakeirq.calls++;
	if (g_fakeirq.hook && g_fakeirq.calls == g_fakeirq.hook_at)
	{
		g_fakeirq.hook_fired = 1;
		g_fakeirq.hook();
	}
	return ((uint64_t)g_fakeirq.depth);
}

void	irq_restore(uint64_t flags)
{
	(void)flags;
	g_fakeirq.depth--;
}

void	cpu_relax(void)
{
	g_fakeirq.relax_calls++;
	if (g_fakeirq.relax_release)
	{
		g_pmm.serving++;
		g_fakeirq.relax_release = 0;
	}
}

int	fake_irq_depth(void)
{
	return (g_fakeirq.depth);
}
