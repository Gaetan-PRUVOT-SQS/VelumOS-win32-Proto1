#include "velum/err.h"
#include "velum/panic.h"
#include "vmm_int.h"

void	vmm_desc_put(t_aspace *as, t_vmregion *d)
{
	d->start = 0;
	d->end = 0;
	d->flags = 0;
	d->maxprot = 0;
	d->next = as->free_desc;
	as->free_desc = d;
	as->nfree++;
}

void	vmm_pool_carve(t_aspace *as, void *page, uint32_t first)
{
	t_vmregion	*slot;
	uint32_t	i;

	slot = (t_vmregion *)page;
	i = first;
	while (i < POOL_SLOTS)
	{
		vmm_desc_put(as, &slot[i]);
		i++;
	}
}

static int	pool_grow(t_aspace *as)
{
	uint64_t	phys;
	uint64_t	*page;

	phys = pmm_alloc_zero(PMM_KERNEL);
	if (!phys)
		return (E_NOMEM);
	page = phys_to_virt(phys);
	page[0] = as->pool_next;
	as->pool_next = phys;
	as->pool_pages++;
	vmm_pool_carve(as, page, 1);
	return (0);
}

int	vmm_pool_reserve(t_aspace *as, uint32_t n)
{
	while (as->nfree < n)
	{
		if (pool_grow(as) < 0)
			return (E_NOMEM);
	}
	return (0);
}

t_vmregion	*vmm_desc_get(t_aspace *as)
{
	t_vmregion	*d;

	d = as->free_desc;
	kassert_check(d != NULL, "vmm: descripteur de region manquant");
	as->free_desc = d->next;
	as->nfree--;
	d->next = NULL;
	return (d);
}
