#include "vmm_int.h"

bool	vmm_reg_overlaps(t_aspace *as, uintptr_t lo, uintptr_t hi)
{
	t_vmregion	*r;

	r = as->regions;
	while (r && r->start < hi)
	{
		if (r->end > lo)
			return (true);
		r = r->next;
	}
	return (false);
}

void	vmm_reg_insert(t_aspace *as, const t_vmregion *tpl)
{
	t_vmregion	*d;
	t_vmregion	**link;

	d = vmm_desc_get(as);
	d->start = tpl->start;
	d->end = tpl->end;
	d->flags = tpl->flags;
	d->maxprot = tpl->maxprot;
	link = &as->regions;
	while (*link && (*link)->start < d->start)
		link = &(*link)->next;
	d->next = *link;
	*link = d;
}

t_vmregion	*vmm_reg_from(t_aspace *as, uintptr_t va)
{
	t_vmregion	*r;

	r = as->regions;
	while (r && r->end <= va)
		r = r->next;
	return (r);
}

void	vmm_reg_split(t_aspace *as, uintptr_t at)
{
	t_vmregion	*r;
	t_vmregion	*d;

	r = vmm_reg_from(as, at);
	if (!r || r->start >= at || at >= r->end)
		return ;
	d = vmm_desc_get(as);
	d->start = at;
	d->end = r->end;
	d->flags = r->flags;
	d->maxprot = r->maxprot;
	d->next = r->next;
	r->end = at;
	r->next = d;
}
