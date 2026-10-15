#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/heap.h"
#include "velum/pmm.h"
#include "velum/proc.h"

void	pmm_get_stats(t_pmm_stats *out)
{
	*out = g_fake.pmm;
}

void	heap_get_stats(t_heap_stats *out)
{
	*out = g_fake.heap;
}

void	*kcalloc(size_t n, size_t size)
{
	if (g_fake.alloc_fail)
		return (NULL);
	return (calloc(n, size));
}

void	kfree(void *ptr)
{
	free(ptr);
}

int	proc_list(t_procinfo *out, uint32_t max)
{
	if (g_fake.proc_list_rc < 0)
		return (g_fake.proc_list_rc);
	if (g_fake.nprocs < max)
		max = g_fake.nprocs;
	memset(out, 0, max * sizeof(*out));
	return ((int)max);
}
