#include "vmm_int.h"

uintptr_t	vmm_find_locked(t_aspace *as, size_t n, uintptr_t lo, uintptr_t hi)
{
	t_vmregion	*r;
	uintptr_t	cand;

	if (!n || !is_aligned(n, PAGE_SIZE) || lo >= hi)
		return (0);
	cand = max_u64(align_up(lo, PAGE_SIZE), as->lo);
	hi = min_u64(hi, as->hi);
	r = vmm_reg_from(as, cand);
	while (cand < hi && hi - cand >= n)
	{
		if (!r || r->start >= cand + n)
			return (cand);
		cand = max_u64(cand, r->end);
		r = r->next;
	}
	return (0);
}

uintptr_t	vmm_find_free(t_aspace *as, size_t len, uintptr_t lo, uintptr_t hi)
{
	uint64_t	irq;
	uintptr_t	va;

	if (!as)
		return (0);
	irq = vmm_lock(as);
	va = vmm_find_locked(as, len, lo, hi);
	vmm_unlock(as, irq);
	return (va);
}
