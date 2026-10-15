#include "velum/err.h"
#include "vmm_int.h"

static void	unmap_one(t_aspace *as, uint64_t *e, uintptr_t va)
{
	uint64_t	old;

	old = *e;
	if (!(old & PTE_P))
		return ;
	*e = 0;
	vmm_flush(as, va);
	if (old & PTE_OWNED)
	{
		pmm_free(old & PTE_ADDR, vmm_owner(as, va, old));
		as->owned--;
	}
}

void	vmm_unmap_pages(t_aspace *as, uintptr_t va, size_t len)
{
	uint64_t	*e;
	uint64_t	step;
	uintptr_t	end;
	uintptr_t	next;

	end = va + len;
	while (va < end)
	{
		step = vmm_reach(as, va, PT_LEAF, &e);
		if (e)
			unmap_one(as, e, va);
		next = align_down(va, step) + step;
		if (next <= va)
			return ;
		va = next;
	}
}

int	vmm_unmap_locked(t_aspace *as, uintptr_t va, size_t len)
{
	int	rc;

	rc = vmm_check_range(as, va, len);
	if (rc == 0 && vmm_reg_has(as, va, va + len, VM_KEPT))
		rc = E_ACCES;
	if (rc == 0)
		rc = vmm_pool_reserve(as, 2);
	if (rc < 0)
		return (rc);
	vmm_unmap_pages(as, va, len);
	vmm_prune(as, va, va + len);
	vmm_reg_cut(as, va, va + len);
	return (0);
}

int	vmm_unmap(t_aspace *as, uintptr_t va, size_t len)
{
	uint64_t	irq;
	int			rc;

	if (!as)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = vmm_unmap_locked(as, va, len);
	vmm_unlock(as, irq);
	return (rc);
}
