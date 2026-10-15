#include "vmm_int.h"

uint64_t	vmm_reach(t_aspace *as, uintptr_t va, int level, uint64_t **out)
{
	uint64_t	*table;
	uint64_t	e;
	int			l;

	table = pt_table(as->pml4);
	l = PT_PML4;
	*out = NULL;
	while (l > level)
	{
		e = table[pt_index(va, l)];
		if (!(e & PTE_P) || (e & PTE_PS))
			return (pt_span(l));
		table = pt_table(e);
		l--;
	}
	*out = &table[pt_index(va, level)];
	return (pt_span(level));
}

static bool	canonical(uintptr_t va)
{
	return (va < USER_HALF_END || va >= HHDM_DEFAULT);
}

static bool	look_leaf(t_ptlook *out, uint64_t e, uintptr_t va, int l)
{
	out->pte = e;
	out->size = pt_span(l);
	out->pa = (e & PTE_ADDR & ~(out->size - 1)) | (va & (out->size - 1));
	return (true);
}

bool	vmm_lookup(t_aspace *as, uintptr_t va, t_ptlook *out)
{
	uint64_t	*table;
	uint64_t	e;
	uint64_t	wu;
	uint64_t	nx;
	int			l;

	if (!canonical(va))
		return (false);
	table = pt_table(as->pml4);
	wu = PTE_W | PTE_U;
	nx = 0;
	l = PT_PML4;
	while (l >= PT_LEAF)
	{
		e = table[pt_index(va, l)];
		if (!(e & PTE_P))
			return (false);
		wu &= e;
		nx |= e & PTE_NX;
		if (l == PT_LEAF || (e & PTE_PS))
			return (look_leaf(out, (e & ~(PTE_W | PTE_U)) | wu | nx, va, l));
		table = pt_table(e);
		l--;
	}
	return (false);
}
