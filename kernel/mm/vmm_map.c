#include "velum/err.h"
#include "vmm_int.h"

int	vmm_map_one(t_aspace *as, uintptr_t va, uint64_t pa, uint64_t bits)
{
	uint64_t	*e;

	e = vmm_entry(as, va, PT_LEAF, true);
	if (!e)
		return (E_NOMEM);
	if (*e & PTE_P)
		return (E_EXIST);
	*e = (pa & PTE_ADDR) | bits;
	return (0);
}

static int	alloc_pages(t_aspace *as, uintptr_t va, size_t len, uint64_t bits)
{
	size_t		done;
	uint64_t	pa;
	t_pmm_owner	owner;
	int			rc;

	done = 0;
	owner = vmm_owner(as, va, bits);
	while (done < len)
	{
		pa = pmm_alloc_zero(owner);
		rc = E_NOMEM;
		if (pa)
			rc = vmm_map_one(as, va + done, pa, bits);
		if (rc < 0)
		{
			if (pa)
				pmm_free(pa, owner);
			vmm_unmap_pages(as, va, done);
			vmm_prune(as, va, va + len);
			return (rc);
		}
		as->owned++;
		done += PAGE_SIZE;
	}
	return (0);
}

int	vmm_alloc_locked(t_aspace *as, uintptr_t va, size_t n, uint32_t f)
{
	t_vmregion	tpl;
	int			rc;

	rc = vmm_check_range(as, va, n);
	if (rc == 0)
		rc = vmm_check_flags(as, f, true);
	if (rc == 0 && vmm_reg_overlaps(as, va, va + n))
		rc = E_EXIST;
	if (rc == 0)
		rc = vmm_pool_reserve(as, 1);
	if (rc == 0)
		rc = alloc_pages(as, va, n, vmm_pte_bits(f) | PTE_OWNED);
	if (rc < 0)
		return (rc);
	tpl.start = va;
	tpl.end = va + n;
	tpl.flags = f;
	tpl.maxprot = VM_PROT;
	vmm_reg_insert(as, &tpl);
	return (0);
}

int	vmm_alloc(t_aspace *as, uintptr_t va, size_t len, uint32_t fl)
{
	uint64_t	irq;
	int			rc;

	if (!as)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = vmm_alloc_locked(as, va, len, fl);
	vmm_unlock(as, irq);
	return (rc);
}
