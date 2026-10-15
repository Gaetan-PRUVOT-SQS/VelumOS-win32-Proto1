#include "heap_int.h"
#include "velum/libk.h"

void	heap_dbg_free(void *p, uint32_t slot, const void *at)
{
	if (slot > HEAP_ZONE_FROM && heap_dbg_zone_bad(p, slot))
		heap_fault("débordement de la zone rouge de", p, at);
	memset(p, HEAP_FILL_FREE, slot);
}

size_t	heap_dbg_used(const void *p, uint32_t slot)
{
	const uint64_t	*rec;

	rec = (const uint64_t *)((const char *)p + slot - HEAP_ZONE_TAIL);
	if (rec[1] != (HEAP_ZONE_MAGIC ^ rec[0]) || rec[0] > slot - HEAP_ZONE_TAIL)
		return (slot - HEAP_ZONE_TAIL);
	return (rec[0]);
}

void	heap_dbg_resize(void *p, uint32_t slot, size_t to)
{
	size_t	from;

	if (slot <= HEAP_ZONE_FROM)
		return ;
	from = heap_dbg_used(p, slot);
	if (to > from)
		memset((char *)p + from, HEAP_FILL_ALLOC, to - from);
	heap_dbg_zone_set(p, slot, to);
}

int	heap_dbg_slot_bad(const void *p, uint32_t slot, int live)
{
	if (!live)
		return (heap_dbg_scan(p, slot, HEAP_FILL_FREE) != 0);
	if (slot <= HEAP_ZONE_FROM)
		return (0);
	return (heap_dbg_zone_bad(p, slot) != 0);
}
