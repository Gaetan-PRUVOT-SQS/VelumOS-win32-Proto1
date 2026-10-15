#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/pmm.h"
#include "velum/vmm.h"

uint64_t	pmm_alloc_pages(t_pmm_owner o, size_t n, size_t al, uint64_t max)
{
	void	*p;

	(void)al;
	(void)max;
	if (o != PMM_DMA || n == 0 || fk_should_fail())
		return (0);
	p = aligned_alloc(4096, n * 4096);
	if (!p)
		return (0);
	memset(p, 0xab, n * 4096);
	return ((uint64_t)(uintptr_t)fk_track(p));
}

void	pmm_free_pages(uint64_t phys, size_t n, t_pmm_owner owner)
{
	(void)n;
	if (owner != PMM_DMA)
		abort();
	fk_untrack((void *)(uintptr_t)phys);
	free((void *)(uintptr_t)phys);
}

void	*phys_to_virt(uint64_t phys)
{
	return ((void *)(uintptr_t)phys);
}

uint64_t	virt_to_phys(const void *kvirt)
{
	return ((uint64_t)(uintptr_t)kvirt);
}

void	fk_fail_after(int64_t n)
{
	g_fk.armed = (n >= 0);
	g_fk.fail_after = n;
}
