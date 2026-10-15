#include "velum/err.h"
#include "vmm_int.h"

static bool	in_window(uintptr_t va, uintptr_t end, uintptr_t base)
{
	return (va >= base && end <= base + WIN_SIZE);
}

int	vmm_check_range(const t_aspace *as, uintptr_t va, size_t len)
{
	uintptr_t	end;

	if (!as || !len || !is_aligned(va, PAGE_SIZE)
		|| !is_aligned(len, PAGE_SIZE))
		return (E_INVAL);
	if (__builtin_add_overflow(va, len, &end))
		return (E_INVAL);
	if (!as->kernel)
	{
		if (va < USER_MIN || end > USER_TOP)
			return (E_INVAL);
		return (0);
	}
	if (in_window(va, end, KHEAP_BASE) || in_window(va, end, KIO_BASE)
		|| in_window(va, end, KSTACK_BASE))
		return (0);
	return (E_INVAL);
}

int	vmm_check_flags(const t_aspace *as, uint32_t fl, bool anon)
{
	if ((fl & ~(uint32_t)VM_KNOWN) || !(fl & VM_R))
		return (E_INVAL);
	if ((fl & VM_W) && (fl & VM_X))
		return (E_INVAL);
	if ((fl & VM_NOCACHE) && (fl & VM_WC))
		return (E_INVAL);
	if (anon && (fl & (VM_NOCACHE | VM_WC | VM_SHARED)))
		return (E_INVAL);
	if (as->kernel && (fl & VM_USER))
		return (E_INVAL);
	if (!as->kernel && (fl & VM_GLOBAL))
		return (E_INVAL);
	return (0);
}

t_pmm_owner	vmm_owner(const t_aspace *as, uintptr_t va, uint64_t pte)
{
	if (!as->kernel)
	{
		if (pte & PTE_U)
			return (PMM_USER);
		return (PMM_KERNEL);
	}
	if (va >= KSTACK_BASE && va < KSTACK_BASE + WIN_SIZE)
		return (PMM_STACK);
	if (va >= KHEAP_BASE && va < KHEAP_BASE + WIN_SIZE)
		return (PMM_HEAP);
	return (PMM_KERNEL);
}
