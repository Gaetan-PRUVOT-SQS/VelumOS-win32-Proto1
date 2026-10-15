#include "sched_int.h"
#include "velum/irqflags.h"
#include "velum/panic.h"

void	spin_release(t_spinlock *l, void *caller)
{
	uint32_t	serving;

	serving = __atomic_load_n(&l->serving, __ATOMIC_RELAXED);
	if (__atomic_load_n(&l->ticket, __ATOMIC_RELAXED) == serving)
		panic("spin_unlock: verrou '%s' non pris (appelant %p)", l->name,
			caller);
	if (SCHED_DEBUG)
		lockdep_release(l);
	__atomic_store_n(&l->serving, serving + 1, __ATOMIC_RELEASE);
}

void	spin_unlock(t_spinlock *l)
{
	spin_release(l, __builtin_return_address(0));
	sched_preempt_enable();
}

uint64_t	spin_lock_irqsave(t_spinlock *l)
{
	uint64_t	flags;

	flags = irq_save();
	sched_preempt_disable();
	spin_acquire(l, __builtin_return_address(0));
	return (flags);
}

void	spin_unlock_irqrestore(t_spinlock *l, uint64_t flags)
{
	spin_release(l, __builtin_return_address(0));
	irq_restore(flags);
	sched_preempt_enable();
}
