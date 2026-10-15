#include "velum/err.h"
#include "vmm_int.h"

static int	check_map(t_aspace *as, const t_vmreq *rq)
{
	uint64_t	pend;
	int			rc;

	rc = vmm_check_range(as, rq->va, rq->len);
	if (rc == 0)
		rc = vmm_check_flags(as, rq->flags, false);
	if (rc == 0 && (!is_aligned(rq->pa, PAGE_SIZE)
			|| __builtin_add_overflow(rq->pa, rq->len, &pend)
			|| pend > PHYS_LIMIT))
		rc = E_INVAL;
	if (rc == 0 && vmm_reg_overlaps(as, rq->va, rq->va + rq->len))
		rc = E_EXIST;
	return (rc);
}

static int	map_pages(t_aspace *as, const t_vmreq *rq, uint64_t bits)
{
	size_t	done;
	int		rc;

	done = 0;
	while (done < rq->len)
	{
		rc = vmm_map_one(as, rq->va + done, rq->pa + done, bits);
		if (rc < 0)
		{
			vmm_unmap_pages(as, rq->va, done);
			vmm_prune(as, rq->va, rq->va + rq->len);
			return (rc);
		}
		done += PAGE_SIZE;
	}
	return (0);
}

int	vmm_map_locked(t_aspace *as, const t_vmreq *rq)
{
	t_vmregion	tpl;
	int			rc;

	rc = check_map(as, rq);
	if (rc == 0)
		rc = vmm_pool_reserve(as, 1);
	if (rc == 0)
		rc = map_pages(as, rq, vmm_pte_bits(rq->flags));
	if (rc < 0)
		return (rc);
	tpl.start = rq->va;
	tpl.end = rq->va + rq->len;
	tpl.flags = rq->flags;
	tpl.maxprot = rq->flags & VM_PROT;
	vmm_reg_insert(as, &tpl);
	return (0);
}

int	vmm_map(t_aspace *as, const t_vmreq *rq)
{
	uint64_t	irq;
	int			rc;

	if (!as || !rq)
		return (E_INVAL);
	irq = vmm_lock(as);
	rc = vmm_map_locked(as, rq);
	vmm_unlock(as, irq);
	return (rc);
}
