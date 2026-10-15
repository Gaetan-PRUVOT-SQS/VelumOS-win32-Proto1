#include "a05_lock.h"
#include "velum/irqflags.h"
#include "velum/panic.h"

uint64_t	a05_lock(t_a05lock *l)
{
	uint64_t	flags;
	uint64_t	spins;

	flags = irq_save();
	spins = 0;
	while (__atomic_test_and_set(&l->taken, __ATOMIC_ACQUIRE))
	{
		spins++;
		if (spins == A05_LOCK_SPIN_MAX)
			panic("a05: verrou %s bloqué", l->name);
		cpu_relax();
	}
	return (flags);
}

void	a05_unlock(t_a05lock *l, uint64_t flags)
{
	__atomic_clear(&l->taken, __ATOMIC_RELEASE);
	irq_restore(flags);
}
