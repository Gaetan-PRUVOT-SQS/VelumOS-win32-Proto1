#include "pmm_int.h"

static void	zone_bounds(t_pmm_req *q, uint64_t max)
{
	q->lo = PMM_LOW_FRAMES;
	q->hi = g_pmm.span;
	if (max && max <= PMM_LOW_END)
		q->lo = 1;
	if (max)
		q->hi = min_u64(q->hi, max >> PAGE_SHIFT);
}

int	pmm_req_build(t_pmm_req *q, uint64_t n, uint64_t al, uint64_t max)
{
	if (!al)
		al = 1;
	if (!n || n > g_pmm.span || (al & (al - 1)) || al > g_pmm.span)
		return (0);
	q->count = n;
	q->align = al;
	zone_bounds(q, max);
	q->start = q->lo;
	if (!max && g_pmm.hint >= q->lo && g_pmm.hint < q->hi)
		q->start = g_pmm.hint;
	return (q->lo < q->hi);
}

int	pmm_inject_fail(void)
{
	if (!g_pmm.fail_armed)
		return (0);
	if (!g_pmm.fail_left)
		return (1);
	g_pmm.fail_left--;
	return (0);
}
