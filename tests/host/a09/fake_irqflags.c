#include "fake.h"
#include "velum/irqflags.h"

uint64_t	irq_save(void)
{
	g_fake.irq_depth++;
	return (1);
}

void	irq_restore(uint64_t flags)
{
	if (flags != 1)
		g_fake.lock_errors++;
	g_fake.irq_depth--;
}

void	irq_disable(void)
{
	g_fake.irq_depth++;
}

void	irq_enable(void)
{
	g_fake.irq_depth--;
}

void	cpu_relax(void)
{
}
