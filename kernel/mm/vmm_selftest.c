#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "vmm_int.h"
#include "vmm_weak.h"

#define ST_PAGES 1000

static int	st_fill(uintptr_t va)
{
	uint32_t	i;
	int			rc;

	i = 0;
	rc = 0;
	while (rc == 0 && i < ST_PAGES)
	{
		rc = vmm_alloc(&g_vmm.kas, va + i * PAGE_SIZE, PAGE_SIZE,
				VM_R | VM_W | VM_GLOBAL);
		if (rc == 0 && *(volatile uint64_t *)(va + i * PAGE_SIZE) != 0)
			rc = E_IO;
		if (rc == 0)
			*(volatile uint64_t *)(va + i * PAGE_SIZE) = i;
		i++;
	}
	return (rc);
}

static int	st_kernel_pages(void)
{
	t_pmm_stats	before;
	t_pmm_stats	after;
	uintptr_t	va;
	int			rc;

	pmm_get_stats(&before);
	va = vmm_find_free(&g_vmm.kas, ST_PAGES * PAGE_SIZE, KHEAP_BASE,
			KHEAP_BASE + WIN_SIZE);
	rc = E_NOMEM;
	if (va)
		rc = st_fill(va);
	if (va && rc == 0 && *(volatile uint64_t *)(va + 999 * PAGE_SIZE) != 999)
		rc = E_IO;
	if (va)
		vmm_unmap(&g_vmm.kas, va, ST_PAGES * PAGE_SIZE);
	pmm_get_stats(&after);
	if (rc == 0 && (before.owned[PMM_HEAP] != after.owned[PMM_HEAP]
			|| before.owned[PMM_PAGETABLE] != after.owned[PMM_PAGETABLE]))
		rc = E_BUSY;
	if (rc)
		klog_err("vmm selftest: 1000 pages noyau (%d)", rc);
	return (rc != 0);
}

static int	st_audit(void)
{
	int	rc;

	if (cpu_features && cpu_features()->nx && !g_vmm.nx)
	{
		klog_err("vmm selftest: NX annonce mais absent des tables");
		return (1);
	}
	rc = vmm_audit_wx();
	if (rc == E_NOTSUP)
		klog_warn("vmm selftest: audit W^X sans NX");
	else if (rc != 0)
		klog_err("vmm selftest: %d pages W et X dans le noyau", rc);
	return (rc != 0 && rc != E_NOTSUP);
}

static void	st_guard(void)
{
	const char	*mode;
	uint8_t		*base;

	mode = boot_cmdline_get("fault");
	if (!VMM_DEBUG || !mode || strcmp(mode, "guard"))
		return ;
	base = vmm_kstack_alloc(4);
	if (!base)
		return ;
	klog_info("vmm selftest: ecriture sous la pile (page de garde)");
	*(volatile uint64_t *)(base - 8) = 1;
	klog_err("vmm selftest: page de garde non detectee");
}

int	vmm_selftest(void)
{
	int	fails;

	fails = st_kernel_pages();
	fails += st_audit();
	fails += vmm_st_user();
	st_guard();
	return (fails);
}
