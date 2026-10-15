#include <string.h>
#include <sys/mman.h>
#include "a04_fake.h"
#include "velum/err.h"

int	heap_pages_unmap(uintptr_t va, size_t npages)
{
	int	rc;

	pthread_mutex_lock(&g_fake.lock);
	rc = 0;
	if (g_fake.unmap_fail > 0)
	{
		g_fake.unmap_fail--;
		rc = g_fake.unmap_rc;
	}
	else if (!fake_pages_in_state(va, npages, 1))
		g_fake.violations++;
	else
	{
		madvise((void *)va, npages << 12, MADV_DONTNEED);
		mprotect((void *)va, npages << 12, PROT_NONE);
		memset(g_fake.state + ((va - (uintptr_t)g_fake.mem) >> 12), 0, npages);
		g_fake.mapped -= npages;
		g_fake.unmaps++;
	}
	pthread_mutex_unlock(&g_fake.lock);
	return (rc);
}

int	fake_pages_mapped_at(uintptr_t va)
{
	return (fake_pages_in_state(va & ~4095ull, 1, 1));
}

void	fake_pages_unmap_fail(uint32_t count, int rc)
{
	pthread_mutex_lock(&g_fake.lock);
	g_fake.unmap_fail = count;
	g_fake.unmap_rc = rc;
	pthread_mutex_unlock(&g_fake.lock);
}
