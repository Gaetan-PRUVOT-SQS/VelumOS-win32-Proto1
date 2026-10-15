#include "heap_int.h"

static void	*cache_pick(t_class *c, t_cache *k, t_slab *s)
{
	void	*p;

	if (s->inuse == 0)
		k->empty--;
	p = slab_obj_take(c, s);
	if (s->inuse == s->nobj)
	{
		slabq_remove(&k->partial, s);
		slabq_push_head(&k->full, s);
		s->full = 1;
	}
	c->calls_alloc++;
	k->allocs++;
	k->bytes += c->size;
	return (p);
}

void	*cache_take(t_class *c, uint32_t tag)
{
	t_cache	*k;

	k = &c->cache[tag];
	if (!k->partial.head)
		return (NULL);
	return (cache_pick(c, k, k->partial.head));
}

void	*cache_adopt(t_class *c, t_slab *s)
{
	t_cache	*k;

	k = &c->cache[s->tag];
	slabq_push_head(&k->partial, s);
	k->slabs++;
	k->empty++;
	return (cache_pick(c, k, s));
}

static int	cache_retire(t_cache *k, t_slab *s)
{
	slabq_remove(&k->partial, s);
	if (k->empty >= HEAP_KEEP_EMPTY)
	{
		k->slabs--;
		s->magic = 0;
		return (1);
	}
	slabq_push_tail(&k->partial, s);
	k->empty++;
	return (0);
}

int	cache_release(t_class *c, t_slab *s)
{
	t_cache	*k;

	k = &c->cache[s->tag];
	c->calls_free++;
	k->allocs--;
	k->bytes -= c->size;
	if (s->full)
	{
		slabq_remove(&k->full, s);
		s->full = 0;
		slabq_push_head(&k->partial, s);
	}
	if (s->inuse != 0)
		return (0);
	return (cache_retire(k, s));
}
