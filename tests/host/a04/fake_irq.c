#include <sched.h>
#include "a04_fake.h"
#include "velum/irqflags.h"

static __thread int	g_depth;
static int			g_errors;

uint64_t	irq_save(void)
{
	g_depth++;
	return (0x200);
}

void	irq_restore(uint64_t flags)
{
	(void)flags;
	g_depth--;
	if (g_depth < 0)
		__atomic_fetch_add(&g_errors, 1, __ATOMIC_RELAXED);
}

void	cpu_relax(void)
{
	sched_yield();
}

int	fake_irq_depth(void)
{
	return (g_depth);
}

int	fake_irq_errors(void)
{
	return (g_errors);
}
