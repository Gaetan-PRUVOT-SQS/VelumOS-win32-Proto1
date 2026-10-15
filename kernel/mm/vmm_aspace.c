#include "velum/libk.h"
#include "velum/panic.h"
#include "vmm_int.h"

static t_aspace	*aspace_init(uint64_t page, uint64_t pml4)
{
	t_aspace	*as;
	uint32_t	first;

	as = phys_to_virt(page);
	memset(as, 0, sizeof(*as));
	as->self_phys = page;
	as->pml4 = pml4;
	as->tables = 1;
	as->pool_pages = 1;
	as->lo = USER_MIN;
	as->hi = USER_TOP;
	first = (sizeof(t_aspace) + sizeof(t_vmregion) - 1) / sizeof(t_vmregion);
	vmm_pool_carve(as, as, first);
	return (as);
}

t_aspace	*vmm_aspace_create(void)
{
	uint64_t	page;
	uint64_t	pml4;

	if (!g_vmm.ready)
		return (NULL);
	page = pmm_alloc_zero(PMM_KERNEL);
	if (!page)
		return (NULL);
	pml4 = pmm_alloc_zero(PMM_PAGETABLE);
	if (!pml4)
	{
		pmm_free(page, PMM_KERNEL);
		return (NULL);
	}
	memcpy(pt_table(pml4) + PT_ENTRIES / 2,
		pt_table(g_vmm.kas.pml4) + PT_ENTRIES / 2,
		PT_ENTRIES / 2 * sizeof(uint64_t));
	return (aspace_init(page, pml4));
}

static void	free_pool(uint64_t next, uint64_t self)
{
	uint64_t	page;

	while (next)
	{
		page = next;
		next = *(uint64_t *)phys_to_virt(page);
		pmm_free(page, PMM_KERNEL);
	}
	pmm_free(self, PMM_KERNEL);
}

void	vmm_aspace_destroy(t_aspace *as)
{
	if (!as)
		return ;
	kassert_check(!as->kernel, "vmm: destruction de l'espace noyau");
	if (as->kernel)
		return ;
	if (g_vmm.current == as)
		vmm_switch(NULL);
	vmm_free_lower(as);
	pmm_free(as->pml4, PMM_PAGETABLE);
	free_pool(as->pool_next, as->self_phys);
}
