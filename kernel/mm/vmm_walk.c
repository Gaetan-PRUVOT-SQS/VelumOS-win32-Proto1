#include "velum/err.h"
#include "vmm_int.h"

uint64_t	*pt_table(uint64_t phys)
{
	return ((uint64_t *)phys_to_virt(phys & PTE_ADDR));
}

uint32_t	pt_index(uintptr_t va, int level)
{
	return ((uint32_t)(va >> (PAGE_SHIFT + 9 * (level - 1))) & 511);
}

uint64_t	pt_span(int level)
{
	return (1ull << (PAGE_SHIFT + 9 * (level - 1)));
}

static int	table_new(t_aspace *as, uint64_t *e, uintptr_t va, int level)
{
	uint64_t	phys;

	if (level == PT_PML4 && va >= USER_HALF_END && g_vmm.ready)
		return (E_INVAL);
	phys = pmm_alloc_zero(PMM_PAGETABLE);
	if (!phys)
		return (E_NOMEM);
	*e = phys | PTE_P | PTE_W;
	if (va < USER_HALF_END)
		*e |= PTE_U;
	as->tables++;
	return (0);
}

uint64_t	*vmm_entry(t_aspace *as, uintptr_t va, int level, bool create)
{
	uint64_t	*table;
	uint64_t	*e;
	int			l;

	table = pt_table(as->pml4);
	l = PT_PML4;
	while (l > level)
	{
		e = &table[pt_index(va, l)];
		if (!(*e & PTE_P) && (!create || table_new(as, e, va, l) < 0))
			return (NULL);
		if (*e & PTE_PS)
			return (NULL);
		table = pt_table(*e);
		l--;
	}
	return (&table[pt_index(va, level)]);
}
