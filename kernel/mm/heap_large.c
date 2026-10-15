#include "heap_int.h"

static int	large_geometry(const t_heap_req *rq, uint64_t *lead,
	uint64_t *pages)
{
	uint64_t	limit;

	limit = ((uint64_t)g_heap.lay.large_pages - 2) << PAGE_SHIFT;
	if (rq->size >= limit)
		return (-1);
	*lead = HEAP_LARGE_HDR;
	if (rq->align > HEAP_ALIGN_SLAB)
		*lead = PAGE_SIZE;
	*pages = (*lead + rq->size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	return (0);
}

static t_large	*large_reserve(uint64_t pages)
{
	t_arena		*a;
	uint64_t	slot;

	a = &g_heap.arena[HEAP_LARGE_ARENA];
	slot = arena_take(a, pages + 1);
	if (slot == ARENA_NONE)
		return (NULL);
	if (heap_pages_map(arena_addr(a, slot), pages) < 0)
	{
		arena_give(a, slot, pages + 1);
		return (NULL);
	}
	__atomic_fetch_add(&g_heap.pages_mapped, pages, __ATOMIC_RELAXED);
	return ((t_large *)arena_addr(a, slot));
}

void	large_publish(t_large *h)
{
	t_large_state	*l;
	uint64_t		flags;

	l = &g_heap.large;
	flags = heap_lock(&l->lock);
	h->prev = NULL;
	h->next = l->head;
	if (l->head)
		l->head->prev = h;
	l->head = h;
	l->allocs[h->tag]++;
	l->bytes[h->tag] += (uint64_t)h->npages << PAGE_SHIFT;
	l->calls_alloc++;
	heap_unlock(&l->lock, flags);
	large_head_set(arena_index(&g_heap.arena[HEAP_LARGE_ARENA], (uintptr_t)h),
		1);
}

void	*large_alloc(const t_heap_req *rq)
{
	t_large		*h;
	uint64_t	lead;
	uint64_t	pages;

	if (large_geometry(rq, &lead, &pages) < 0)
		return (NULL);
	h = large_reserve(pages);
	if (!h)
		return (NULL);
	h->cookie = HEAP_LARGE_MAGIC ^ (uintptr_t)h;
	h->size = rq->size;
	h->data = (uintptr_t)h + lead;
	h->npages = (uint32_t)pages;
	h->tag = rq->tag;
	if (HEAP_DEBUG)
		heap_dbg_large_fill(h);
	large_publish(h);
	return ((void *)h->data);
}
