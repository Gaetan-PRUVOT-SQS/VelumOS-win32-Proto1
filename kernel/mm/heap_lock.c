#include "heap_int.h"
#include "velum/irqflags.h"
#include "velum/panic.h"

void	heap_lock_init(t_spinlock *l, const char *name)
{
	l->ticket = 0;
	l->serving = 0;
	l->name = name;
}

uint64_t	heap_lock(t_spinlock *l)
{
	uint64_t	flags;
	uint64_t	spins;
	uint32_t	mine;

	flags = irq_save();
	mine = __atomic_fetch_add(&l->ticket, 1, __ATOMIC_RELAXED);
	spins = 0;
	while (__atomic_load_n(&l->serving, __ATOMIC_ACQUIRE) != mine)
	{
		cpu_relax();
		spins++;
		if (spins == HEAP_SPIN_LIMIT)
			panic("heap: verrou %s bloqué (appelant %p)", l->name,
				__builtin_return_address(0));
	}
	return (flags);
}

void	heap_unlock(t_spinlock *l, uint64_t flags)
{
	uint32_t	next;

	next = l->serving + 1;
	__atomic_store_n(&l->serving, next, __ATOMIC_RELEASE);
	irq_restore(flags);
}
