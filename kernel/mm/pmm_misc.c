#include "velum/libk.h"
#include "pmm_int.h"

void	pmm_get_stats(t_pmm_stats *out)
{
	uint64_t	flags;

	if (!out)
		return ;
	flags = pmm_lock();
	*out = g_pmm.stats;
	pmm_unlock(flags);
	out->owned[PMM_FREE] = out->free_pages;
}

void	pmm_fail_after(int64_t n)
{
	uint64_t	flags;

	flags = pmm_lock();
	g_pmm.fail_armed = n >= 0;
	g_pmm.fail_left = 0;
	if (n >= 0)
		g_pmm.fail_left = (uint64_t)n;
	pmm_unlock(flags);
}

uint64_t	pmm_alloc_zero(t_pmm_owner owner)
{
	uint64_t	phys;

	phys = pmm_alloc(owner);
	if (phys)
		memset(pmm_virt(phys), 0, PAGE_SIZE);
	return (phys);
}
