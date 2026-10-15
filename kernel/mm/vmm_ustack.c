#include "velum/err.h"
#include "vmm_int.h"

static bool	stack_pair(t_aspace *as, uintptr_t va, size_t len)
{
	t_vmregion	*r;

	r = vmm_reg_from(as, va);
	if (!r || r->start != va || r->end != va + PAGE_SIZE
		|| !(r->flags & VM_RESERVED))
		return (false);
	r = r->next;
	return (r && r->start == va + PAGE_SIZE && (r->flags & VM_HELD)
		&& r->end - r->start == len);
}

static void	stack_guard_insert(t_aspace *as, uintptr_t va)
{
	t_vmregion	tpl;

	vmm_reg_from(as, va + PAGE_SIZE)->flags |= VM_HELD;
	tpl.start = va;
	tpl.end = va + PAGE_SIZE;
	tpl.flags = VM_RESERVED;
	tpl.maxprot = 0;
	vmm_reg_insert(as, &tpl);
}

static int	stack_map_locked(t_aspace *as, uintptr_t *va, size_t len)
{
	size_t	span;
	int		rc;

	if (__builtin_add_overflow(len, PAGE_SIZE, &span))
		return (E_INVAL);
	if (!*va)
		*va = vmm_find_locked(as, span, ASLR_LO, ASLR_HI);
	if (!*va)
		return (E_NOMEM);
	rc = vmm_check_range(as, *va, span);
	if (rc == 0 && vmm_reg_overlaps(as, *va, *va + PAGE_SIZE))
		rc = E_EXIST;
	if (rc == 0)
		rc = vmm_pool_reserve(as, 2);
	if (rc == 0)
		rc = vmm_alloc_locked(as, *va + PAGE_SIZE, len,
				VM_USER | VM_R | VM_W);
	if (rc == 0)
		stack_guard_insert(as, *va);
	return (rc);
}

int	vmm_stack_map(t_aspace *as, uintptr_t *va, size_t len)
{
	uint64_t	irq;
	int			rc;

	if (!as || !va)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = stack_map_locked(as, va, len);
	vmm_unlock(as, irq);
	return (rc);
}

int	vmm_stack_unmap(t_aspace *as, uintptr_t va, size_t len)
{
	uint64_t	irq;
	int			rc;

	if (!as)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = E_NOENT;
	if (stack_pair(as, va, len))
	{
		vmm_unmap_pages(as, va + PAGE_SIZE, len);
		vmm_prune(as, va + PAGE_SIZE, va + PAGE_SIZE + len);
		vmm_reg_drop(as, va + PAGE_SIZE, va + PAGE_SIZE + len, VM_HELD);
		rc = vmm_reg_drop(as, va, va + PAGE_SIZE, VM_RESERVED);
	}
	vmm_unlock(as, irq);
	return (rc);
}
