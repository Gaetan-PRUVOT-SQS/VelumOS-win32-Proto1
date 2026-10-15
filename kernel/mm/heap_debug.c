#include "heap_int.h"
#include "velum/libk.h"

size_t	heap_dbg_scan(const void *p, size_t n, uint8_t v)
{
	const uint8_t	*b;
	size_t			i;
	size_t			bad;

	b = p;
	i = 0;
	bad = 0;
	while (i < n)
	{
		if (b[i] != v)
			bad++;
		i++;
	}
	return (bad);
}

void	heap_dbg_fresh(void *p, size_t n)
{
	memset(p, HEAP_FILL_FREE, n);
}

void	heap_dbg_zone_set(void *p, uint32_t slot, size_t size)
{
	uint64_t	*rec;

	memset((char *)p + size, HEAP_FILL_ZONE, slot - HEAP_ZONE_TAIL - size);
	rec = (uint64_t *)((char *)p + slot - HEAP_ZONE_TAIL);
	rec[0] = size;
	rec[1] = HEAP_ZONE_MAGIC ^ size;
}

size_t	heap_dbg_zone_bad(const void *p, uint32_t slot)
{
	const uint64_t	*rec;

	rec = (const uint64_t *)((const char *)p + slot - HEAP_ZONE_TAIL);
	if (rec[1] != (HEAP_ZONE_MAGIC ^ rec[0]) || rec[0] > slot - HEAP_ZONE_TAIL)
		return (1);
	return (heap_dbg_scan((const char *)p + rec[0],
			slot - HEAP_ZONE_TAIL - rec[0], HEAP_FILL_ZONE));
}

void	heap_dbg_alloc(void *p, uint32_t slot, const t_heap_req *rq)
{
	if (heap_dbg_scan(p, slot, HEAP_FILL_FREE))
		heap_fault("écriture après libération détectée dans", p, rq->caller);
	if (slot <= HEAP_ZONE_FROM)
	{
		memset(p, HEAP_FILL_ALLOC, slot);
		return ;
	}
	memset(p, HEAP_FILL_ALLOC, rq->size);
	heap_dbg_zone_set(p, slot, rq->size);
}
