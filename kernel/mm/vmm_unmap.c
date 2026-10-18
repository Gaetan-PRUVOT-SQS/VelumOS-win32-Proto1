#include "velum/err.h"
#include "vmm_int.h"

typedef struct s_freebatch
{
	t_aspace	*as;
	uint64_t	pa[UNMAP_BATCH];
	t_pmm_owner	owner[UNMAP_BATCH];
	uint32_t	n;
	uintptr_t	lo;
	uintptr_t	hi;
	bool		dirty;
}	t_freebatch;

static void	batch_release(t_freebatch *b)
{
	uint32_t	i;

	if (!b->dirty)
		return ;
	vmm_tlb_shoot(b->as, b->lo, b->hi);
	i = 0;
	while (i < b->n)
	{
		pmm_free(b->pa[i], b->owner[i]);
		i++;
	}
	b->n = 0;
	b->dirty = false;
}

static void	batch_note(t_freebatch *b, uintptr_t va, uint64_t step)
{
	if (!b->dirty || va < b->lo)
		b->lo = va;
	if (!b->dirty || va + step > b->hi)
		b->hi = va + step;
	b->dirty = true;
}

static void	unmap_one(t_freebatch *b, uint64_t *e, uintptr_t va, uint64_t step)
{
	uint64_t	old;

	old = *e;
	if (!(old & PTE_P))
		return ;
	*e = 0;
	vmm_flush(b->as, va);
	batch_note(b, va, step);
	if (old & PTE_OWNED)
	{
		b->pa[b->n] = old & PTE_ADDR;
		b->owner[b->n] = vmm_owner(b->as, va, old);
		b->n++;
		b->as->owned--;
	}
	if (b->n == UNMAP_BATCH)
		batch_release(b);
}

void	vmm_unmap_pages(t_aspace *as, uintptr_t va, size_t len)
{
	t_freebatch	b;
	uint64_t	*e;
	uint64_t	step;
	uintptr_t	end;
	uintptr_t	next;

	b.as = as;
	b.n = 0;
	b.dirty = false;
	end = va + len;
	while (va < end)
	{
		step = vmm_reach(as, va, PT_LEAF, &e);
		if (e)
			unmap_one(&b, e, va, step);
		next = align_down(va, step) + step;
		if (next <= va)
			break ;
		va = next;
	}
	batch_release(&b);
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
