#include "heap_int.h"
#include "velum/err.h"
#include "velum/pmm.h"
#include "velum/vmm.h"

int	heap_pages_layout(t_heap_layout *out)
{
	t_vminfo	info;
	t_pmm_stats	ps;

	if (vmm_query(vmm_kernel_aspace(), KHEAP_BASE, &info))
		return (E_BUSY);
	out->base = KHEAP_BASE;
	pmm_get_stats(&ps);
	heap_layout_for_ram(ps.total_pages << PAGE_SHIFT, out);
	return (0);
}

int	heap_pages_map(uintptr_t va, size_t npages)
{
	return (vmm_alloc(vmm_kernel_aspace(), va, npages * PAGE_SIZE,
			VM_R | VM_W));
}

int	heap_pages_unmap(uintptr_t va, size_t npages)
{
	return (vmm_unmap(vmm_kernel_aspace(), va, npages * PAGE_SIZE));
}
