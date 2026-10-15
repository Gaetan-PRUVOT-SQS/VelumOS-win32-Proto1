#include "vmm_int.h"

void	*phys_to_virt(uint64_t phys)
{
	return ((void *)(uintptr_t)(phys + boot_info()->hhdm));
}

uint64_t	virt_to_phys(const void *kvirt)
{
	const t_bootinfo	*bi;
	t_ptlook			look;
	uintptr_t			va;
	uint64_t			irq;
	bool				ok;

	bi = boot_info();
	va = (uintptr_t)kvirt;
	if (va >= KERNEL_BASE && bi->kernel_virt && va >= bi->kernel_virt)
		return (bi->kernel_phys + (va - bi->kernel_virt));
	if (va >= bi->hhdm && va - bi->hhdm < HHDM_MAX)
		return (va - bi->hhdm);
	if (!g_vmm.ready)
		return (0);
	irq = vmm_lock(&g_vmm.kas);
	ok = vmm_lookup(&g_vmm.kas, va, &look);
	vmm_unlock(&g_vmm.kas, irq);
	if (!ok)
		return (0);
	return (look.pa);
}
