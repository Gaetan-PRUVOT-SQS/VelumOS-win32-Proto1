#include "meminfo.h"
#include "velum/libk.h"
#include "velum/util.h"

void	meminfo_build(t_meminfo *out, const t_pmm_stats *ps,
	uint64_t heap_window, uint64_t heap_live)
{
	uint32_t	i;

	memset(out, 0, sizeof(*out));
	out->version = MEMINFO_VERSION;
	out->page_size = PAGE_SIZE;
	out->span_bytes = (ps->total_pages + ps->reserved_pages) * PAGE_SIZE;
	out->usable_bytes = ps->total_pages * PAGE_SIZE;
	out->free_bytes = ps->free_pages * PAGE_SIZE;
	out->reserved_bytes = ps->reserved_pages * PAGE_SIZE;
	out->heap_window_bytes = heap_window;
	out->heap_live_bytes = heap_live;
	i = 0;
	while (i < MEMINFO_OWNERS)
	{
		out->owned_pages[i] = ps->owned[i];
		i++;
	}
	out->alloc_calls = ps->alloc_calls;
	out->free_calls = ps->free_calls;
	out->fail_calls = ps->fail_calls;
}
