#include "heap_int.h"

static t_slab	*trim_collect(t_class *c)
{
	t_slab		*chain;
	t_slab		*s;
	t_cache		*k;
	uint32_t	t;

	chain = NULL;
	t = 0;
	while (t < HEAP_TAGS)
	{
		k = &c->cache[t];
		while (k->empty > 0)
		{
			s = k->partial.tail;
			slabq_remove(&k->partial, s);
			k->empty--;
			k->slabs--;
			s->magic = 0;
			s->next = chain;
			chain = s;
		}
		t++;
	}
	return (chain);
}

void	slab_trim(t_class *c)
{
	t_slab		*chain;
	t_slab		*next;
	uint64_t	flags;

	flags = heap_lock(&c->lock);
	chain = trim_collect(c);
	heap_unlock(&c->lock, flags);
	while (chain)
	{
		next = chain->next;
		if (slab_destroy(chain) < 0)
			slab_revive(chain);
		chain = next;
	}
}

void	heap_trim(void)
{
	uint32_t	i;

	if (!g_heap.ready)
		return ;
	i = 0;
	while (i < HEAP_CLASSES)
	{
		slab_trim(&g_heap.cls[i]);
		i++;
	}
}
