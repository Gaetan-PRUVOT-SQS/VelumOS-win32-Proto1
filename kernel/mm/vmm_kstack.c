#include "vmm_int.h"

void	*vmm_kstack_alloc(size_t pages)
{
	t_vmreq	rq;

	if (!pages || pages > KSTACK_MAX)
		return (NULL);
	rq.pa = 0;
	rq.len = pages * PAGE_SIZE;
	rq.flags = VM_R | VM_W | VM_GLOBAL;
	if (vmm_kwin_place(&rq, KSTACK_BASE, true) < 0)
		return (NULL);
	return ((void *)rq.va);
}

void	vmm_kstack_free(void *base, size_t pages)
{
	uintptr_t	va;

	va = (uintptr_t)base;
	if (!pages || pages > KSTACK_MAX || va < KSTACK_BASE
		|| va >= KSTACK_BASE + WIN_SIZE)
		return ;
	vmm_unmap(&g_vmm.kas, va, pages * PAGE_SIZE);
}
