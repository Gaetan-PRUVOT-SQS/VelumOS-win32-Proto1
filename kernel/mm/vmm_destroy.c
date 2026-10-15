#include "vmm_int.h"

static bool	is_table(uint64_t e)
{
	return ((e & PTE_P) && !(e & PTE_PS));
}

static void	free_pt(t_aspace *as, uint64_t phys)
{
	uint64_t	*t;
	uint32_t	i;

	t = pt_table(phys);
	i = 0;
	while (i < PT_ENTRIES)
	{
		if ((t[i] & PTE_P) && (t[i] & PTE_OWNED))
			pmm_free(t[i] & PTE_ADDR, vmm_owner(as, 0, t[i]));
		i++;
	}
	pmm_free(phys, PMM_PAGETABLE);
}

static void	free_pd(t_aspace *as, uint64_t phys)
{
	uint64_t	*t;
	uint32_t	i;

	t = pt_table(phys);
	i = 0;
	while (i < PT_ENTRIES)
	{
		if (is_table(t[i]))
			free_pt(as, t[i] & PTE_ADDR);
		i++;
	}
	pmm_free(phys, PMM_PAGETABLE);
}

static void	free_pdpt(t_aspace *as, uint64_t phys)
{
	uint64_t	*t;
	uint32_t	i;

	t = pt_table(phys);
	i = 0;
	while (i < PT_ENTRIES)
	{
		if (is_table(t[i]))
			free_pd(as, t[i] & PTE_ADDR);
		i++;
	}
	pmm_free(phys, PMM_PAGETABLE);
}

void	vmm_free_lower(t_aspace *as)
{
	uint64_t	*pml4;
	uint32_t	i;

	pml4 = pt_table(as->pml4);
	i = 0;
	while (i < PT_ENTRIES / 2)
	{
		if (is_table(pml4[i]))
			free_pdpt(as, pml4[i] & PTE_ADDR);
		pml4[i] = 0;
		i++;
	}
}
