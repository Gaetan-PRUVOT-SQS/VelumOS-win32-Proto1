#include "fake.h"

t_fsync	g_fsync;

void	fsync_reset(void)
{
	g_fsync.held = 0;
	g_fsync.errors = 0;
}

void	spin_init(t_spinlock *l, const char *name)
{
	l->ticket = 0;
	l->serving = 0;
	l->name = name;
}

void	spin_lock(t_spinlock *l)
{
	uint32_t	mine;
	uint32_t	spins;

	mine = __atomic_fetch_add(&l->ticket, 1, __ATOMIC_ACQ_REL);
	spins = 0;
	while (__atomic_load_n(&l->serving, __ATOMIC_ACQUIRE) != mine)
	{
		spins++;
		if ((spins & 255) == 0)
			th_relax();
		if (spins > 20000000u)
		{
			__atomic_fetch_add(&g_fsync.errors, 1, __ATOMIC_RELAXED);
			break ;
		}
	}
	__atomic_fetch_add(&g_fsync.held, 1, __ATOMIC_RELAXED);
}

void	spin_unlock(t_spinlock *l)
{
	if (l->ticket == l->serving)
		__atomic_fetch_add(&g_fsync.errors, 1, __ATOMIC_RELAXED);
	else
		__atomic_fetch_add(&l->serving, 1, __ATOMIC_RELEASE);
	__atomic_fetch_sub(&g_fsync.held, 1, __ATOMIC_RELAXED);
}

uint64_t	spin_lock_irqsave(t_spinlock *l)
{
	spin_lock(l);
	return (0x246);
}
