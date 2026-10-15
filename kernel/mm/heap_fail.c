#include "heap_int.h"
#include "velum/err.h"
#include "velum/panic.h"

void	*heap_fail(void)
{
	__atomic_fetch_add(&g_heap.fail_calls, 1, __ATOMIC_RELAXED);
	return (NULL);
}

int	heap_inject_fail(void)
{
	int64_t	left;

	left = __atomic_load_n(&g_heap.fail_after, __ATOMIC_RELAXED);
	while (left > 0)
	{
		if (__atomic_compare_exchange_n(&g_heap.fail_after, &left, left - 1,
				0, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
			return (0);
	}
	return (left == 0);
}

void	heap_fail_after(int64_t n)
{
	if (n < 0)
		n = -1;
	__atomic_store_n(&g_heap.fail_after, n, __ATOMIC_RELAXED);
}

int	heap_unmap(uintptr_t va, size_t npages)
{
	int	rc;

	rc = heap_pages_unmap(va, npages);
	kassert_check(rc == 0 || rc == E_NOMEM, "heap: vmm_unmap a échoué");
	if (rc != 0)
		__atomic_fetch_add(&g_heap.unmap_refused, 1, __ATOMIC_RELAXED);
	return (rc);
}
