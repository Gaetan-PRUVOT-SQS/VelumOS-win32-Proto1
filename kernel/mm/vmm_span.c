#include "velum/err.h"
#include "vmm_int.h"

static int	span_large(t_aspace *as, uintptr_t va, uint64_t pa, uint64_t bits)
{
	uint64_t	*e;

	e = vmm_entry(as, va, PT_PD, true);
	if (!e)
		return (E_NOMEM);
	if (*e & PTE_PS)
		return (0);
	if (*e & PTE_P)
		return (1);
	*e = pa | vmm_pte_large(bits);
	return (0);
}

static int	span_small(t_aspace *as, uintptr_t va, uint64_t pa, uint64_t bits)
{
	t_ptlook	look;
	uint64_t	*e;

	if (vmm_lookup(as, va, &look))
		return (0);
	e = vmm_entry(as, va, PT_LEAF, true);
	if (!e)
		return (E_NOMEM);
	*e = pa | bits;
	return (0);
}

static int	span_step(t_aspace *as, t_vmreq *cur, uint64_t bits)
{
	uint64_t	step;
	int			rc;

	rc = 1;
	step = PAGE_2M;
	if (cur->len >= PAGE_2M && is_aligned(cur->va | cur->pa, PAGE_2M))
		rc = span_large(as, cur->va, cur->pa, bits);
	if (rc == 1)
	{
		step = PAGE_SIZE;
		rc = span_small(as, cur->va, cur->pa, bits);
	}
	if (rc < 0)
		return (rc);
	cur->va += step;
	cur->pa += step;
	cur->len -= step;
	return (0);
}

int	vmm_map_span(t_aspace *as, const t_vmreq *rq, uint64_t bits)
{
	t_vmreq	cur;
	int		rc;

	cur = *rq;
	while (cur.len >= PAGE_SIZE)
	{
		rc = span_step(as, &cur, bits);
		if (rc < 0)
			return (rc);
	}
	return (0);
}
