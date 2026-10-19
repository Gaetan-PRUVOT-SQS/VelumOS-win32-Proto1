#include "vmm_int.h"

t_vmm	g_vmm;

t_aspace	*vmm_kernel_aspace(void)
{
	return (&g_vmm.kas);
}

void	vmm_switch(t_aspace *as)
{
	t_aspace	*old;

	if (!g_vmm.ready)
		return ;
	if (!as)
		as = &g_vmm.kas;
	old = vmm_cpu_current();
	if (old == as)
		return ;
	vmm_tlb_attach(as);
	mmu_write_cr3(as->pml4);
	vmm_cpu_set_current(as);
	vmm_tlb_detach(old);
}

void	vmm_flush(t_aspace *as, uintptr_t va)
{
	if (!g_vmm.ready)
		return ;
	if (as->kernel || as == vmm_cpu_current())
		mmu_invlpg(va);
}

uint64_t	vmm_pages_used(const t_aspace *as)
{
	if (!as)
		return (0);
	return (as->owned + as->tables + as->pool_pages);
}
