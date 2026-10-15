#include "heap_int.h"
#include "velum/err.h"
#include "velum/vmm.h"

int	heap_pages_layout(t_heap_layout *out)
{
	t_vminfo	info;

	if (vmm_query(vmm_kernel_aspace(), KHEAP_BASE, &info))
		return (E_BUSY);
	out->base = KHEAP_BASE;
	out->slab_shift = HEAP_SHIFT_KERNEL;
	out->large_pages = HEAP_LARGE_KERNEL;
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
