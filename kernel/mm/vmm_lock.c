#include "velum/irqflags.h"
#include "vmm_int.h"

uint64_t	vmm_lock(t_aspace *as)
{
	uint64_t	flags;
	uint32_t	ticket;

	flags = irq_save();
	ticket = __atomic_fetch_add(&as->ticket, 1, __ATOMIC_RELAXED);
	while (__atomic_load_n(&as->serving, __ATOMIC_ACQUIRE) != ticket)
		cpu_relax();
	return (flags);
}

void	vmm_unlock(t_aspace *as, uint64_t flags)
{
	__atomic_store_n(&as->serving, as->serving + 1, __ATOMIC_RELEASE);
	irq_restore(flags);
}
