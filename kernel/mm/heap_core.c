#include "heap_int.h"

void	heap_req_set(t_heap_req *rq, size_t size, uint32_t tag, const void *at)
{
	rq->size = size;
	rq->align = HEAP_ALIGN_MIN;
	rq->tag = tag;
	rq->caller = at;
}

size_t	heap_need(size_t size)
{
	if (HEAP_DEBUG && size > HEAP_ZONE_FROM && size <= HEAP_SMALL_MAX)
		return (size + HEAP_ZONE_TAIL);
	return (size);
}

uint32_t	heap_class_of(const t_heap_req *rq)
{
	if (rq->align > HEAP_ALIGN_SLAB || rq->size > HEAP_SMALL_MAX)
		return (HEAP_CLASSES);
	return (slab_class_find(heap_need(rq->size), rq->align));
}

void	*heap_alloc(const t_heap_req *rq)
{
	uint32_t	cls;
	void		*p;

	if (!g_heap.ready)
		heap_fault("allocation avant heap_boot_init", NULL, rq->caller);
	if (rq->tag >= HEAP_TAGS)
		heap_fault("étiquette de propriétaire inconnue", NULL, rq->caller);
	if (rq->align == 0 || (rq->align & (rq->align - 1)) != 0
		|| rq->align > HEAP_ALIGN_MAX || heap_inject_fail())
		return (heap_fail());
	cls = heap_class_of(rq);
	if (cls < HEAP_CLASSES)
		p = slab_alloc(&g_heap.cls[cls], rq);
	else
		p = large_alloc(rq);
	if (!p)
		return (heap_fail());
	return (p);
}

void	heap_free(void *p, const void *at)
{
	uint32_t	arena;
	int			kind;

	if (!p)
		return ;
	if (!g_heap.ready)
		heap_fault("libération avant heap_boot_init", p, at);
	kind = heap_locate((uintptr_t)p, &arena);
	if (kind == HEAP_KIND_SLAB)
		slab_free(arena, p, at);
	else if (kind == HEAP_KIND_LARGE)
		large_free(p, at);
	else
		heap_fault("pointeur étranger au tas", p, at);
}
