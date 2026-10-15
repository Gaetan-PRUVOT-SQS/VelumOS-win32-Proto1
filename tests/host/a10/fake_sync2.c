#include "fake.h"

void	spin_unlock_irqrestore(t_spinlock *l, uint64_t flags)
{
	if (flags != 0x246)
		g_fsync.errors++;
	spin_unlock(l);
}
