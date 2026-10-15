#include "heap_int.h"
#include "velum/libk.h"

static uint8_t	*large_zone(const t_large *h, size_t *len)
{
	uintptr_t	end;

	end = (uintptr_t)h + ((size_t)h->npages << PAGE_SHIFT);
	*len = end - (h->data + h->size);
	return ((uint8_t *)(h->data + h->size));
}

void	heap_dbg_large_fill(const t_large *h)
{
	size_t	len;
	uint8_t	*zone;

	memset((void *)h->data, HEAP_FILL_ALLOC, h->size);
	zone = large_zone(h, &len);
	memset(zone, HEAP_FILL_ZONE, len);
}

int	heap_dbg_large_bad(const t_large *h)
{
	size_t	len;
	uint8_t	*zone;

	zone = large_zone(h, &len);
	return (heap_dbg_scan(zone, len, HEAP_FILL_ZONE) != 0);
}

void	heap_dbg_large_verify(const t_large *h, const void *at)
{
	if (heap_dbg_large_bad(h))
		heap_fault("débordement de la zone rouge du bloc", (void *)h->data,
			at);
}

void	heap_dbg_large_resize(t_large *h, size_t to)
{
	size_t	from;
	size_t	len;
	uint8_t	*zone;

	from = h->size;
	if (to > from)
		memset((void *)(h->data + from), HEAP_FILL_ALLOC, to - from);
	h->size = to;
	zone = large_zone(h, &len);
	memset(zone, HEAP_FILL_ZONE, len);
}
