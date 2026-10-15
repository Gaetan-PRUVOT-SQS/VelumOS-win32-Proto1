#include "alloc_int.h"
#include "velum/err.h"
#include "velum/vmem.h"

void	alloc_recent_forget(uint64_t lo, uint64_t hi)
{
	uint32_t	i;
	uint64_t	at;

	i = 0;
	while (i < ALLOC_RECENT)
	{
		at = (uint64_t)(uintptr_t)g_alloc.recent[i];
		if (at >= lo && at < hi)
			g_alloc.recent[i] = NULL;
		i++;
	}
}

void	*alloc_large_get(size_t size)
{
	size_t		pages;
	int64_t		va;
	t_ablock	*b;

	pages = (size + ALLOC_HDR + PAGE_SIZE - 1) / PAGE_SIZE;
	va = v_valloc(0, pages * PAGE_SIZE, PROT_R | PROT_W);
	if (va < 0)
		return (NULL);
	alloc_recent_forget((uint64_t)va, (uint64_t)va + pages * PAGE_SIZE);
	b = (t_ablock *)(uintptr_t)va;
	alloc_seal(b, ALLOC_CLS_LARGE, (uint32_t)pages, ALLOC_TAG_USED);
	g_alloc.stats.large_bytes += pages * PAGE_SIZE;
	g_alloc.stats.live_blocks++;
	g_alloc.stats.live_bytes += pages * PAGE_SIZE - ALLOC_HDR;
	return (b + 1);
}

void	alloc_large_put(t_ablock *b)
{
	uint64_t	len;

	len = (uint64_t)b->pages * PAGE_SIZE;
	g_alloc.recent[g_alloc.recent_pos % ALLOC_RECENT] = b;
	g_alloc.recent_pos++;
	g_alloc.stats.large_bytes -= len;
	g_alloc.stats.live_blocks--;
	g_alloc.stats.live_bytes -= len - ALLOC_HDR;
	v_vfree((uint64_t)(uintptr_t)b, len);
}

int	alloc_large_recent(const void *p)
{
	uint32_t	i;

	i = 0;
	while (i < ALLOC_RECENT)
	{
		if (g_alloc.recent[i] && (const void *)(g_alloc.recent[i] + 1) == p)
			return (1);
		i++;
	}
	return (0);
}
