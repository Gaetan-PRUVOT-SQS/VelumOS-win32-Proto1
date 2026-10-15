#include "vmm_int.h"

static bool	table_empty(const uint64_t *t)
{
	uint32_t	i;

	i = 0;
	while (i < PT_ENTRIES)
	{
		if (t[i])
			return (false);
		i++;
	}
	return (true);
}

static void	prune_entry(t_aspace *as, uint64_t *e, uintptr_t va)
{
	uint64_t	child;

	if (!(*e & PTE_P) || (*e & PTE_PS))
		return ;
	child = *e & PTE_ADDR;
	if (!table_empty(pt_table(child)))
		return ;
	*e = 0;
	vmm_flush(as, va);
	pmm_free(child, PMM_PAGETABLE);
	as->tables--;
}

static void	prune_level(t_aspace *as, uintptr_t lo, uintptr_t hi, int level)
{
	uint64_t	*e;
	uint64_t	step;
	uintptr_t	va;
	uintptr_t	next;

	va = align_down(lo, pt_span(level));
	while (va < hi)
	{
		step = vmm_reach(as, va, level, &e);
		if (e)
			prune_entry(as, e, va);
		next = align_down(va, step) + step;
		if (next <= va)
			return ;
		va = next;
	}
}

void	vmm_prune(t_aspace *as, uintptr_t lo, uintptr_t hi)
{
	prune_level(as, lo, hi, PT_PD);
	prune_level(as, lo, hi, PT_PDPT);
	if (hi <= USER_HALF_END)
		prune_level(as, lo, hi, PT_PML4);
}
