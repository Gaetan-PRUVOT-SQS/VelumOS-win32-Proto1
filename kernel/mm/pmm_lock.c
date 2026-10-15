#include "velum/irqflags.h"
#include "pmm_int.h"

uint64_t	pmm_lock(void)
{
	uint64_t	flags;
	uint32_t	mine;

	flags = irq_save();
	mine = __atomic_fetch_add(&g_pmm.ticket, 1, __ATOMIC_RELAXED);
	while (__atomic_load_n(&g_pmm.serving, __ATOMIC_ACQUIRE) != mine)
		cpu_relax();
	return (flags);
}

void	pmm_unlock(uint64_t flags)
{
	uint32_t	next;

	next = __atomic_load_n(&g_pmm.serving, __ATOMIC_RELAXED) + 1;
	__atomic_store_n(&g_pmm.serving, next, __ATOMIC_RELEASE);
	irq_restore(flags);
}
