#include "velum/libk.h"
#include "pmm_int.h"

static int	free_check(uint64_t phys, uint64_t n, t_pmm_owner o)
{
	uint64_t	f;
	uint64_t	i;

	f = phys >> PAGE_SHIFT;
	if (!is_aligned(phys, PAGE_SIZE))
		return (PMM_FAULT_ALIGN);
	if (f >= g_pmm.span || n > g_pmm.span - f)
		return (PMM_FAULT_RANGE);
	if ((int)o < PMM_KERNEL || (int)o >= PMM_OWNERS)
		return (PMM_FAULT_OWNER);
	i = 0;
	while (i < n)
	{
		if (g_pmm.owner[f + i] == PMM_FREE)
			return (PMM_FAULT_DOUBLE);
		if (g_pmm.owner[f + i] != o)
			return (PMM_FAULT_OWNER);
		i++;
	}
	return (PMM_FAULT_NONE);
}

static void	free_commit(uint64_t f, uint64_t n, t_pmm_owner o)
{
	if (PMM_DEBUG)
		memset(pmm_virt(f << PAGE_SHIFT), PMM_POISON, n << PAGE_SHIFT);
	pmm_bits_fill(f, f + n, 0);
	memset(g_pmm.owner + f, PMM_FREE, n);
	g_pmm.stats.free_pages += n;
	g_pmm.stats.owned[o] -= n;
	g_pmm.stats.free_calls++;
}

static void	free_n(uint64_t phys, uint64_t n, t_pmm_owner o, void *from)
{
	uint64_t	flags;
	int			fault;

	if (!n)
		return ;
	flags = pmm_lock();
	fault = free_check(phys, n, o);
	if (!fault)
		free_commit(phys >> PAGE_SHIFT, n, o);
	pmm_unlock(flags);
	if (fault)
		pmm_fault_report(fault, phys, n, from);
}

void	pmm_free_pages(uint64_t phys, size_t n, t_pmm_owner owner)
{
	free_n(phys, n, owner, __builtin_return_address(0));
}

void	pmm_free(uint64_t phys, t_pmm_owner owner)
{
	free_n(phys, 1, owner, __builtin_return_address(0));
}
