#include <string.h>
#include <sys/mman.h>
#include "a04_fake.h"
#include "velum/err.h"

t_fake_pages	g_fake;

int	heap_pages_layout(t_heap_layout *out)
{
	if (g_fake.busy)
		return (E_BUSY);
	out->base = (uintptr_t)g_fake.mem;
	out->slab_shift = g_fake.slab_shift;
	out->large_pages = g_fake.large_pages;
	return (0);
}

int	fake_pages_in_state(uintptr_t va, size_t n, uint8_t want)
{
	size_t	first;
	size_t	i;

	if (va < (uintptr_t)g_fake.mem || (va & 4095))
		return (0);
	first = (va - (uintptr_t)g_fake.mem) >> 12;
	if (first >= g_fake.npages || n > g_fake.npages - first)
		return (0);
	i = 0;
	while (i < n)
	{
		if (g_fake.state[first + i] != want)
			return (0);
		i++;
	}
	return (1);
}

static void	pages_commit(uintptr_t va, size_t npages)
{
	if (g_fake.fail_after > 0)
		g_fake.fail_after--;
	mprotect((void *)va, npages << 12, PROT_READ | PROT_WRITE);
	memset((void *)va, A04_DIRTY, npages << 12);
	memset(g_fake.state + ((va - (uintptr_t)g_fake.mem) >> 12), 1, npages);
	g_fake.mapped += npages;
	g_fake.maps++;
}

int	heap_pages_map(uintptr_t va, size_t npages)
{
	int	rc;

	pthread_mutex_lock(&g_fake.lock);
	rc = E_NOMEM;
	if (g_fake.fail_after == 0)
		rc = E_NOMEM;
	else if (!fake_pages_in_state(va, npages, 0))
	{
		g_fake.violations++;
		rc = E_EXIST;
	}
	else
	{
		pages_commit(va, npages);
		rc = 0;
	}
	pthread_mutex_unlock(&g_fake.lock);
	return (rc);
}
