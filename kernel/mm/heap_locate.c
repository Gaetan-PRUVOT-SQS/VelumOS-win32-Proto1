#include "heap_int.h"

int	heap_locate(uintptr_t p, uint32_t *arena)
{
	uint64_t	off;
	uint64_t	slab_bytes;

	off = p - g_heap.lay.base;
	slab_bytes = (uint64_t)HEAP_SLAB_ARENAS << g_heap.lay.slab_shift;
	if (off < slab_bytes)
	{
		*arena = (uint32_t)(off >> g_heap.lay.slab_shift);
		return (HEAP_KIND_SLAB);
	}
	if (off - slab_bytes < ((uint64_t)g_heap.lay.large_pages << PAGE_SHIFT))
	{
		*arena = HEAP_LARGE_ARENA;
		return (HEAP_KIND_LARGE);
	}
	return (HEAP_KIND_NONE);
}

void	heap_block_info(const void *p, const void *at, t_block *out)
{
	uint32_t	arena;
	int			kind;

	kind = heap_locate((uintptr_t)p, &arena);
	if (kind == HEAP_KIND_SLAB)
		slab_info(arena, p, at, out);
	else if (kind == HEAP_KIND_LARGE)
		large_info(p, at, out);
	else
		heap_fault("pointeur étranger au tas", p, at);
}

int	heap_block_fits(const t_block *b, size_t size)
{
	t_heap_req	rq;

	if (size > b->cap)
		return (0);
	heap_req_set(&rq, size, 0, NULL);
	if (b->kind == HEAP_KIND_SLAB)
		return (heap_class_of(&rq) == b->unit);
	return (heap_class_of(&rq) == HEAP_CLASSES && b->cap - size < PAGE_SIZE);
}

void	heap_block_restamp(void *p, const t_block *b, size_t size)
{
	t_large	*h;

	if (b->kind == HEAP_KIND_SLAB)
	{
		if (HEAP_DEBUG)
			heap_dbg_resize(p, g_heap.cls[b->unit].size, size);
		return ;
	}
	h = large_resolve(p, NULL);
	if (HEAP_DEBUG)
		heap_dbg_large_resize(h, size);
	h->size = size;
}
