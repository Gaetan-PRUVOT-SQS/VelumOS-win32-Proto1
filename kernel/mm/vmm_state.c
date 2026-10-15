#include "vmm_int.h"

t_vmm	g_vmm;

t_aspace	*vmm_kernel_aspace(void)
{
	return (&g_vmm.kas);
}

void	vmm_switch(t_aspace *as)
{
	if (!g_vmm.ready)
		return ;
	if (!as)
		as = &g_vmm.kas;
	if (g_vmm.current == as)
		return ;
	mmu_write_cr3(as->pml4);
	g_vmm.current = as;
}

void	vmm_flush(t_aspace *as, uintptr_t va)
{
	if (!g_vmm.ready)
		return ;
	if (as->kernel || as == g_vmm.current)
		mmu_invlpg(va);
}

uint64_t	vmm_pages_used(const t_aspace *as)
{
	if (!as)
		return (0);
	return (as->owned + as->tables + as->pool_pages);
}
