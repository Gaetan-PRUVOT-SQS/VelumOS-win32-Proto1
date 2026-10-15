#include "velum/err.h"
#include "vmm_int.h"

bool	vmm_reg_has(t_aspace *as, uintptr_t lo, uintptr_t hi, uint32_t mask)
{
	t_vmregion	*r;

	r = vmm_reg_from(as, lo);
	while (r && r->start < hi)
	{
		if (r->flags & mask)
			return (true);
		r = r->next;
	}
	return (false);
}

int	vmm_reg_drop(t_aspace *as, uintptr_t lo, uintptr_t hi, uint32_t kind)
{
	t_vmregion	**link;
	t_vmregion	*r;

	link = &as->regions;
	while (*link && (*link)->start < lo)
		link = &(*link)->next;
	r = *link;
	if (!r || r->start != lo || r->end != hi || !(r->flags & kind))
		return (E_NOENT);
	*link = r->next;
	vmm_desc_put(as, r);
	return (0);
}

int	vmm_reserve(t_aspace *as, uintptr_t va, size_t len)
{
	t_vmregion	tpl;
	uint64_t	irq;
	int			rc;

	if (!as)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = vmm_check_range(as, va, len);
	if (rc == 0 && vmm_reg_overlaps(as, va, va + len))
		rc = E_EXIST;
	if (rc == 0)
		rc = vmm_pool_reserve(as, 1);
	tpl.start = va;
	tpl.end = va + len;
	tpl.flags = VM_RESERVED;
	tpl.maxprot = 0;
	if (rc == 0)
		vmm_reg_insert(as, &tpl);
	vmm_unlock(as, irq);
	return (rc);
}

int	vmm_unreserve(t_aspace *as, uintptr_t va, size_t len)
{
	uint64_t	irq;
	int			rc;

	if (!as)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = vmm_check_range(as, va, len);
	if (rc == 0)
		rc = vmm_reg_drop(as, va, va + len, VM_RESERVED);
	vmm_unlock(as, irq);
	return (rc);
}

uint64_t	vmm_region_count(t_aspace *as)
{
	t_vmregion	*r;
	uint64_t	irq;
	uint64_t	n;

	if (!as)
		return (0);
	irq = vmm_lock(as);
	n = 0;
	r = as->regions;
	while (r)
	{
		n++;
		r = r->next;
	}
	vmm_unlock(as, irq);
	return (n);
}
