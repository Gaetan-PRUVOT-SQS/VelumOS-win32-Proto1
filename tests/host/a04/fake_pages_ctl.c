#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "a04_fake.h"

void	fake_pages_reset(void)
{
	if (g_fake.mem)
	{
		munmap(g_fake.mem, g_fake.npages << 12);
		free(g_fake.state);
	}
	g_fake.mem = NULL;
	g_fake.state = NULL;
	g_fake.npages = 0;
	g_fake.mapped = 0;
	g_fake.maps = 0;
	g_fake.unmaps = 0;
	g_fake.violations = 0;
	g_fake.fail_after = -1;
	g_fake.unmap_fail = 0;
	g_fake.unmap_rc = 0;
	g_fake.busy = 0;
}

void	fake_pages_setup(uint32_t slab_shift, uint32_t large_pages)
{
	size_t	bytes;

	fake_pages_reset();
	g_fake.slab_shift = slab_shift;
	g_fake.large_pages = large_pages;
	bytes = (((size_t)HEAP_ARENAS - 1) << slab_shift)
		+ ((size_t)large_pages << 12);
	g_fake.npages = bytes >> 12;
	g_fake.mem = mmap(NULL, bytes, PROT_NONE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	g_fake.state = calloc(g_fake.npages, 1);
	if (g_fake.mem == MAP_FAILED || !g_fake.state)
		abort();
}

void	fake_pages_fail_after(int64_t n)
{
	pthread_mutex_lock(&g_fake.lock);
	g_fake.fail_after = n;
	pthread_mutex_unlock(&g_fake.lock);
}

void	fake_pages_busy(int busy)
{
	g_fake.busy = busy;
}
