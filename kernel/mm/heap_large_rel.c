#include "heap_int.h"

static void	large_unlink(t_large *h)
{
	t_large_state	*l;
	uint64_t		flags;

	l = &g_heap.large;
	flags = heap_lock(&l->lock);
	if (h->prev)
		h->prev->next = h->next;
	else
		l->head = h->next;
	if (h->next)
		h->next->prev = h->prev;
	l->allocs[h->tag]--;
	l->bytes[h->tag] -= (uint64_t)h->npages << PAGE_SHIFT;
	l->calls_free++;
	heap_unlock(&l->lock, flags);
}

int	large_release(t_large *h)
{
	t_arena		*a;
	uint64_t	slot;
	uint64_t	pages;

	a = &g_heap.arena[HEAP_LARGE_ARENA];
	slot = arena_index(a, (uintptr_t)h);
	pages = h->npages;
	large_head_set(slot, 0);
	h->cookie = 0;
	if (heap_unmap((uintptr_t)h, pages) < 0)
		return (-1);
	__atomic_fetch_sub(&g_heap.pages_mapped, pages, __ATOMIC_RELAXED);
	arena_give(a, slot, pages + 1);
	return (0);
}

void	large_revive(t_large *h)
{
	h->cookie = HEAP_LARGE_MAGIC ^ (uintptr_t)h;
	large_publish(h);
}

void	large_free(void *p, const void *at)
{
	t_large	*h;

	h = large_resolve(p, at);
	if (HEAP_DEBUG)
		heap_dbg_large_verify(h, at);
	large_unlink(h);
	if (large_release(h) < 0)
		large_revive(h);
}
