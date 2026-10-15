#include "heap_int.h"

t_slab	*slab_resolve(uint32_t arena, const void *p, const void *at)
{
	t_arena		*a;
	t_slab		*s;
	uint64_t	idx;

	a = &g_heap.arena[arena];
	idx = arena_index(a, (uintptr_t)p);
	if (!arena_test(a, idx))
		heap_fault("double libération ou pointeur étranger", p, at);
	s = (t_slab *)arena_addr(a, idx);
	if (s->magic != slab_cookie(s) || s->cls >= HEAP_CLASSES
		|| s->arena != arena)
		heap_fault("pointeur étranger (en-tête de dalle invalide)", p, at);
	return (s);
}

void	*slab_alloc(t_class *c, const t_heap_req *rq)
{
	t_slab		*s;
	void		*p;
	uint64_t	flags;

	flags = heap_lock(&c->lock);
	p = cache_take(c, rq->tag);
	heap_unlock(&c->lock, flags);
	if (!p)
	{
		s = slab_create(c, rq->tag);
		if (!s)
			return (NULL);
		flags = heap_lock(&c->lock);
		p = cache_adopt(c, s);
		heap_unlock(&c->lock, flags);
	}
	if (HEAP_DEBUG)
		heap_dbg_alloc(p, c->size, rq);
	return (p);
}

void	slab_free(uint32_t arena, void *p, const void *at)
{
	t_slab		*s;
	t_class		*c;
	uint64_t	flags;
	uint32_t	idx;
	int			rc;

	s = slab_resolve(arena, p, at);
	c = &g_heap.cls[s->cls];
	idx = slab_idx(s, p, at);
	if (!slab_live(s, idx))
		heap_fault("double libération de", p, at);
	if (HEAP_DEBUG)
		heap_dbg_free(p, c->size, at);
	flags = heap_lock(&c->lock);
	rc = slab_put(s, idx);
	if (rc == 0)
		rc = cache_release(c, s);
	heap_unlock(&c->lock, flags);
	if (rc < 0)
		heap_fault("double libération de", p, at);
	if (rc > 0 && slab_destroy(s) < 0)
		slab_revive(s);
}

void	slab_info(uint32_t arena, const void *p, const void *at, t_block *out)
{
	t_slab			*s;
	const t_class	*c;

	s = slab_resolve(arena, p, at);
	c = &g_heap.cls[s->cls];
	if (!slab_live(s, slab_idx(s, p, at)))
		heap_fault("objet déjà libéré :", p, at);
	out->cap = c->size;
	out->used = c->size;
	if (HEAP_DEBUG && c->size > HEAP_ZONE_FROM)
	{
		out->cap = c->size - HEAP_ZONE_TAIL;
		out->used = heap_dbg_used(p, c->size);
	}
	out->tag = s->tag;
	out->kind = HEAP_KIND_SLAB;
	out->unit = s->cls;
}
