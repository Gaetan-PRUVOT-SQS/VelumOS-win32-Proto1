#include "heap_int.h"
#include "velum/libk.h"

static void	slab_init(t_slab *s, uint32_t cls, uint32_t tag)
{
	const t_class	*c;
	uint32_t		words;
	uint32_t		pad;

	c = &g_heap.cls[cls];
	memset(s, 0, c->obj_off);
	s->magic = slab_cookie(s);
	s->nobj = (uint16_t)c->nobj;
	s->cls = (uint8_t)cls;
	s->tag = (uint8_t)tag;
	s->arena = (uint8_t)c->arena;
	words = (c->nobj + 63) / 64;
	pad = words * 64 - c->nobj;
	if (pad)
		slab_bits(s)[words - 1] = ~0ull << (64 - pad);
	if (HEAP_DEBUG)
		heap_dbg_fresh(slab_obj_addr(c, s, 0), (size_t)c->nobj * c->size);
}

t_slab	*slab_create(t_class *c, uint32_t tag)
{
	t_arena		*a;
	uint64_t	idx;
	uint64_t	pages;

	a = &g_heap.arena[c->arena];
	pages = 1ull << (a->shift - PAGE_SHIFT);
	idx = arena_take(a, 1);
	if (idx == ARENA_NONE)
		return (NULL);
	if (heap_pages_map(arena_addr(a, idx), pages) < 0)
	{
		arena_give(a, idx, 1);
		return (NULL);
	}
	__atomic_fetch_add(&g_heap.pages_mapped, pages, __ATOMIC_RELAXED);
	slab_init((t_slab *)arena_addr(a, idx), (uint32_t)(c - g_heap.cls), tag);
	return ((t_slab *)arena_addr(a, idx));
}

int	slab_destroy(t_slab *s)
{
	t_arena		*a;
	uint64_t	idx;
	uint64_t	pages;

	a = &g_heap.arena[s->arena];
	idx = arena_index(a, (uintptr_t)s);
	pages = 1ull << (a->shift - PAGE_SHIFT);
	if (heap_unmap((uintptr_t)s, pages) < 0)
		return (-1);
	__atomic_fetch_sub(&g_heap.pages_mapped, pages, __ATOMIC_RELAXED);
	arena_give(a, idx, 1);
	return (0);
}

void	slab_revive(t_slab *s)
{
	t_class		*c;
	t_cache		*k;
	uint64_t	flags;

	c = &g_heap.cls[s->cls];
	k = &c->cache[s->tag];
	flags = heap_lock(&c->lock);
	s->magic = slab_cookie(s);
	slabq_push_tail(&k->partial, s);
	k->slabs++;
	k->empty++;
	heap_unlock(&c->lock, flags);
}
