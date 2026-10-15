#include "velum/libk.h"
#include "pmm_int.h"

static uint64_t	alloc_fail(void)
{
	g_pmm.stats.fail_calls++;
	return (0);
}

static void	claim(uint64_t c, uint64_t n, t_pmm_owner o, int roving)
{
	memset(g_pmm.owner + c, o, n);
	pmm_bits_fill(c, c + n, 1);
	g_pmm.stats.free_pages -= n;
	g_pmm.stats.owned[o] += n;
	if (roving)
		g_pmm.hint = c + n;
}

static uint64_t	alloc_locked(t_pmm_owner o, size_t n, size_t al, uint64_t max)
{
	t_pmm_req	q;
	uint64_t	c;

	g_pmm.stats.alloc_calls++;
	if ((int)o < PMM_KERNEL || (int)o >= PMM_OWNERS)
		return (alloc_fail());
	if (!pmm_req_build(&q, n, al, max))
		return (alloc_fail());
	if (pmm_inject_fail() || g_pmm.stats.free_pages < n)
		return (alloc_fail());
	c = pmm_find_run(&q);
	if (c == PMM_NONE)
		return (alloc_fail());
	claim(c, n, o, max == 0);
	return (c << PAGE_SHIFT);
}

uint64_t	pmm_alloc_pages(t_pmm_owner o, size_t n, size_t al, uint64_t max)
{
	uint64_t	flags;
	uint64_t	phys;

	flags = pmm_lock();
	phys = alloc_locked(o, n, al, max);
	pmm_unlock(flags);
	return (phys);
}

uint64_t	pmm_alloc(t_pmm_owner owner)
{
	return (pmm_alloc_pages(owner, 1, 1, 0));
}
