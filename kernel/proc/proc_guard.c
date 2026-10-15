#include "proc_int.h"
#include "velum/err.h"
#include "velum/util.h"

int	proc_stack_map(t_aspace *as, uint64_t top)
{
	uintptr_t	guard;

	if (top < USTACK_SIZE + PAGE_SIZE + USER_MIN)
		return (E_INVAL);
	guard = top - USTACK_SIZE - PAGE_SIZE;
	return (vmm_stack_map(as, &guard, USTACK_SIZE));
}

int	proc_count(void)
{
	t_proctab	*tab;
	uint64_t	fl;
	uint32_t	n;

	tab = proc_tab();
	fl = spin_lock_irqsave(&tab->lock);
	n = tab->count;
	spin_unlock_irqrestore(&tab->lock, fl);
	return ((int)n);
}
