#include "velum/err.h"
#include "vmm_int.h"

static int	kas_init(void)
{
	t_aspace	*kas;

	kas = &g_vmm.kas;
	kas->kernel = true;
	kas->lo = KHEAP_BASE;
	kas->hi = KSTACK_BASE + WIN_SIZE;
	kas->pml4 = pmm_alloc_zero(PMM_PAGETABLE);
	if (!kas->pml4)
		return (E_NOMEM);
	kas->tables = 1;
	return (0);
}

static int	image_part(const t_bootinfo *bi, uint64_t lo, uint64_t hi,
	uint64_t bits)
{
	t_vmreq	rq;

	rq.va = lo;
	rq.pa = bi->kernel_phys + (lo - bi->kernel_virt);
	rq.len = align_up(hi, PAGE_SIZE) - lo;
	rq.flags = 0;
	return (vmm_map_span(&g_vmm.kas, &rq, bits));
}

static int	map_image(const t_bootinfo *bi)
{
	uint64_t	l[6];
	int			rc;

	if (!bi->kernel_virt || !bi->kernel_phys)
		return (E_PROTO);
	mmu_image_layout(l);
	if (l[0] != bi->kernel_virt || l[2] < l[1] || l[4] < l[3])
		return (E_PROTO);
	rc = image_part(bi, l[0], l[2], PTE_P | PTE_G);
	if (rc == 0)
		rc = image_part(bi, l[2], l[3], PTE_P | PTE_G | g_vmm.nx);
	if (rc == 0)
		rc = image_part(bi, align_down(l[4], PAGE_SIZE), l[5],
				PTE_P | PTE_W | PTE_G | g_vmm.nx);
	return (rc);
}

static int	precreate_windows(void)
{
	uintptr_t	va;
	uint32_t	i;

	i = 0;
	while (i < 6)
	{
		va = KHEAP_BASE + (i >> 1) * WIN_STRIDE + (i & 1) * PML4_SPAN;
		if (!vmm_entry(&g_vmm.kas, va, PT_PDPT, true))
			return (E_NOMEM);
		i++;
	}
	return (0);
}

int	vmm_boot_init(void)
{
	const t_bootinfo	*bi;
	int					rc;

	bi = boot_info();
	vmm_cpu_detect();
	rc = E_PROTO;
	if (bi->hhdm >= HHDM_DEFAULT && bi->hhdm <= KHEAP_BASE - HHDM_MAX
		&& vmm_hhdm_build(bi) > 0)
		rc = kas_init();
	if (rc == 0)
		rc = map_image(bi);
	if (rc == 0)
		rc = vmm_hhdm_map(&g_vmm.kas, bi->hhdm);
	if (rc == 0)
		rc = precreate_windows();
	if (rc < 0)
		return (rc);
	mmu_write_cr3(g_vmm.kas.pml4);
	mmu_flush_global();
	g_vmm.current = &g_vmm.kas;
	g_vmm.ready = true;
	vmm_boot_finish(bi);
	return (0);
}
