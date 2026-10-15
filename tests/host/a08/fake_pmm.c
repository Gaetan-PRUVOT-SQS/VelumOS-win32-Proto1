#include <stdlib.h>
#include <string.h>
#include "fake.h"

uint64_t	pmm_alloc_zero(t_pmm_owner owner)
{
	void	*p;

	(void)owner;
	if (g_fk.pmm_fail > 0)
	{
		g_fk.pmm_fail--;
		if (g_fk.pmm_fail == 0)
			return (0);
	}
	p = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
	if (!p)
		return (0);
	memset(p, 0, PAGE_SIZE);
	g_fk.pmm_live++;
	return ((uint64_t)(uintptr_t)p);
}

void	pmm_free(uint64_t phys, t_pmm_owner owner)
{
	(void)owner;
	if (!phys)
		return ;
	g_fk.pmm_live--;
	free((void *)(uintptr_t)phys);
}

uint64_t	vmm_pages_used(const t_aspace *as)
{
	(void)as;
	return (g_fk.pages_used);
}

uintptr_t	vmm_find_free(t_aspace *as, size_t len, uintptr_t lo, uintptr_t hi)
{
	uintptr_t	va;

	(void)as;
	if (g_fk.next_va < lo || g_fk.next_va + len > hi)
		return (0);
	va = g_fk.next_va;
	g_fk.next_va += len + PAGE_SIZE;
	return (va);
}
