#include "velum/err.h"
#include "vmm_int.h"

int	vmm_kwin_place(t_vmreq *rq, uintptr_t win, bool anon)
{
	t_aspace	*kas;
	uint64_t	irq;
	uintptr_t	base;
	int			rc;

	kas = &g_vmm.kas;
	if (!g_vmm.ready)
		return (E_INVAL);
	irq = vmm_lock(kas);
	rc = E_NOMEM;
	base = vmm_find_locked(kas, rq->len + PAGE_SIZE, win, win + WIN_SIZE);
	if (base)
	{
		rq->va = base + PAGE_SIZE;
		if (anon)
			rc = vmm_alloc_locked(kas, rq->va, rq->len, rq->flags);
		else
			rc = vmm_map_locked(kas, rq);
	}
	vmm_unlock(kas, irq);
	return (rc);
}

static uint32_t	io_flags(uint32_t fl)
{
	if ((fl & ~(uint32_t)VM_IO_OK) || ((fl & VM_WC) && (fl & VM_NOCACHE)))
		return (0);
	if (!(fl & (VM_R | VM_W)))
		fl |= VM_W;
	if (!(fl & VM_WC))
		fl |= VM_NOCACHE;
	return (fl | VM_R | VM_GLOBAL);
}

void	*vmm_io_map(uint64_t phys, size_t len, uint32_t flags)
{
	t_vmreq			rq;
	const t_hspan	*alias;
	uint64_t		off;
	uint64_t		end;

	rq.flags = io_flags(flags);
	if (!len || !rq.flags || __builtin_add_overflow(phys, len, &end)
		|| end > PHYS_LIMIT || !g_vmm.ready)
		return (NULL);
	alias = vmm_hhdm_find(phys, len);
	if (alias && (rq.flags & VM_W) && !(alias->bits & PTE_W))
		return (NULL);
	if (alias)
		return (phys_to_virt(phys));
	off = phys & (PAGE_SIZE - 1);
	rq.pa = phys - off;
	rq.len = align_up(end, PAGE_SIZE) - rq.pa;
	if (vmm_kwin_place(&rq, KIO_BASE, false) < 0)
		return (NULL);
	return ((void *)(rq.va + off));
}

void	vmm_io_unmap(void *virt, size_t len)
{
	uintptr_t	va;
	uintptr_t	end;

	va = (uintptr_t)virt;
	if (!len || __builtin_add_overflow(va, len, &end) || va < KIO_BASE
		|| end > KIO_BASE + WIN_SIZE)
		return ;
	va = align_down(va, PAGE_SIZE);
	vmm_unmap(&g_vmm.kas, va, align_up(end, PAGE_SIZE) - va);
}
