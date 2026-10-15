#include "fake.h"
#include "velum/irqflags.h"
#include "velum/sync.h"

void	spin_init(t_spinlock *l, const char *name)
{
	l->ticket = 0;
	l->serving = 0;
	l->name = name;
}

uint64_t	spin_lock_irqsave(t_spinlock *l)
{
	if (l->ticket != l->serving)
		g_fake.lock_errors++;
	l->ticket++;
	g_fake.lock_depth++;
	g_fake.irq_depth++;
	return (1);
}

void	spin_unlock_irqrestore(t_spinlock *l, uint64_t flags)
{
	if (l->ticket == l->serving || flags != 1)
		g_fake.lock_errors++;
	else
		l->serving++;
	g_fake.lock_depth--;
	g_fake.irq_depth--;
}
