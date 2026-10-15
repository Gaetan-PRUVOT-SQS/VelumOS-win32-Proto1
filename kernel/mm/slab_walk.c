#include "heap_int.h"

int	slab_plausible(const t_class *c, const t_slab *s)
{
	const t_arena	*a;
	uintptr_t		off;

	a = &g_heap.arena[c->arena];
	off = (uintptr_t)s - a->base;
	if ((uintptr_t)s < a->base || off >= (a->slots << a->shift))
		return (0);
	return ((off & ((1ull << a->shift) - 1)) == 0
		&& arena_test(a, arena_index(a, (uintptr_t)s)));
}

static void	walk_one(const t_class *c, const t_slab *s, t_walk *w)
{
	w->bad += (s->prev != w->prev) + (s->tag != w->tag);
	w->bad += (s->full != w->want_full) + slab_check(c, s);
	w->slabs++;
	w->inuse += s->inuse;
	w->empty += (s->inuse == 0);
	w->bad += (s->inuse != 0 && w->empty != 0 && !w->want_full);
	w->prev = s;
}

void	slab_walk(const t_class *c, const t_slabq *q, uint32_t tag, t_walk *w)
{
	const t_slab	*s;
	uint64_t		limit;

	s = q->head;
	w->prev = NULL;
	w->tag = tag;
	limit = g_heap.arena[c->arena].slots;
	while (s && limit > 0)
	{
		if (!slab_plausible(c, s))
		{
			w->bad++;
			return ;
		}
		walk_one(c, s, w);
		s = s->next;
		limit--;
	}
	w->bad += (s != NULL) + (q->tail != w->prev);
}
