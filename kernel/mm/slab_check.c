#include "heap_int.h"

static int	check_header(const t_class *c, const t_slab *s)
{
	const t_arena	*a;
	int				bad;

	a = &g_heap.arena[s->arena];
	bad = 0;
	bad += s->magic != slab_cookie(s);
	bad += s->nobj != c->nobj;
	bad += s->inuse > s->nobj;
	bad += s->full != (s->inuse == s->nobj);
	bad += !arena_test(a, arena_index(a, (uintptr_t)s));
	return (bad);
}

static int	check_bits(const t_class *c, const t_slab *s)
{
	const uint64_t	*bits;
	uint32_t		words;
	uint32_t		pad;
	uint32_t		live;
	uint32_t		i;

	words = (c->nobj + 63) / 64;
	pad = words * 64 - c->nobj;
	bits = slab_bits(s);
	live = 0;
	i = 0;
	while (i < words)
	{
		live += (uint32_t)__builtin_popcountll(bits[i]);
		i++;
	}
	if (pad && (bits[words - 1] >> (64 - pad)) != ((1ull << pad) - 1))
		return (1);
	return (live - pad != s->inuse);
}

static int	check_objects(const t_class *c, const t_slab *s)
{
	uint32_t	i;
	int			bad;

	bad = 0;
	i = 0;
	while (HEAP_DEBUG && i < c->nobj)
	{
		bad += heap_dbg_slot_bad(slab_obj_addr(c, s, i), c->size,
				slab_live(s, i));
		i++;
	}
	return (bad);
}

int	slab_check(const t_class *c, const t_slab *s)
{
	int	bad;

	bad = check_header(c, s);
	if (bad)
		return (bad);
	return (check_bits(c, s) + check_objects(c, s));
}
