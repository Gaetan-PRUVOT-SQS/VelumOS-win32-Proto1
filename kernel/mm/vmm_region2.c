#include "vmm_int.h"

void	vmm_reg_cut(t_aspace *as, uintptr_t lo, uintptr_t hi)
{
	t_vmregion	**link;
	t_vmregion	*r;

	vmm_reg_split(as, lo);
	vmm_reg_split(as, hi);
	link = &as->regions;
	while (*link && (*link)->start < hi)
	{
		r = *link;
		if (r->start >= lo)
		{
			*link = r->next;
			vmm_desc_put(as, r);
		}
		else
			link = &r->next;
	}
}

void	vmm_reg_reflag(t_aspace *as, uintptr_t a, uintptr_t b, uint32_t f)
{
	t_vmregion	*r;

	vmm_reg_split(as, a);
	vmm_reg_split(as, b);
	r = vmm_reg_from(as, a);
	while (r && r->start < b)
	{
		r->flags = (r->flags & ~(uint32_t)VM_PROT) | f;
		r = r->next;
	}
}

bool	vmm_reg_allows(t_aspace *as, uintptr_t a, uintptr_t b, uint32_t f)
{
	t_vmregion	*r;
	uintptr_t	covered;

	covered = a;
	r = vmm_reg_from(as, a);
	while (r && r->start < b)
	{
		if (r->start > covered || (f & ~r->maxprot))
			return (false);
		covered = r->end;
		r = r->next;
	}
	return (covered >= b);
}
