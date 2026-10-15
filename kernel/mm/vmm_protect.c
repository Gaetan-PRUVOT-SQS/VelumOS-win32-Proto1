#include "velum/err.h"
#include "vmm_int.h"

static int	all_mapped(t_aspace *as, uintptr_t va, uintptr_t end)
{
	uint64_t	*e;

	while (va < end)
	{
		vmm_reach(as, va, PT_LEAF, &e);
		if (!e || !(*e & PTE_P))
			return (E_NOENT);
		va += PAGE_SIZE;
	}
	return (0);
}

static void	set_prot(t_aspace *as, uintptr_t va, uintptr_t end, uint32_t p)
{
	uint64_t	*e;
	uint64_t	want;

	want = vmm_pte_bits(p) & (PTE_W | PTE_NX);
	while (va < end)
	{
		vmm_reach(as, va, PT_LEAF, &e);
		*e = (*e & ~(PTE_W | PTE_NX)) | want;
		vmm_flush(as, va);
		va += PAGE_SIZE;
	}
}

static int	protect_locked(t_aspace *as, uintptr_t va, size_t len, uint32_t p)
{
	int	rc;

	rc = vmm_check_range(as, va, len);
	if (rc == 0 && ((p & ~(uint32_t)VM_PROT) || !(p & VM_R)
			|| ((p & VM_W) && (p & VM_X))))
		rc = E_INVAL;
	if (rc == 0)
		rc = all_mapped(as, va, va + len);
	if (rc == 0 && (vmm_reg_has(as, va, va + len, VM_KEPT)
			|| !vmm_reg_allows(as, va, va + len, p)))
		rc = E_ACCES;
	if (rc == 0)
		rc = vmm_pool_reserve(as, 2);
	if (rc < 0)
		return (rc);
	set_prot(as, va, va + len, p);
	vmm_reg_reflag(as, va, va + len, p);
	return (0);
}

int	vmm_protect(t_aspace *as, uintptr_t va, size_t len, uint32_t fl)
{
	uint64_t	irq;
	int			rc;

	if (!as)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = protect_locked(as, va, len, fl);
	vmm_unlock(as, irq);
	return (rc);
}

bool	vmm_query(t_aspace *as, uintptr_t va, t_vminfo *out)
{
	t_ptlook	look;
	uint64_t	irq;
	bool		ok;

	if (!as || !out)
		return (false);
	irq = vmm_lock(as);
	ok = vmm_lookup(as, va, &look);
	vmm_unlock(as, irq);
	if (!ok)
		return (false);
	out->pa = look.pa;
	out->flags = vmm_pte_flags(look.pte, look.size);
	return (true);
}
