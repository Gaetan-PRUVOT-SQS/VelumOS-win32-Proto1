#include "heap_int.h"
#include "velum/libk.h"
#include "velum/panic.h"

static void	arena_mark(t_arena *a, uint64_t idx, uint64_t n, int set)
{
	uint64_t	i;
	uint64_t	*w;
	uint64_t	bit;

	i = 0;
	while (i < n)
	{
		w = &a->bits[(idx + i) >> 6];
		bit = 1ull << ((idx + i) & 63);
		if (set)
			__atomic_store_n(w, *w | bit, __ATOMIC_RELEASE);
		else
			__atomic_store_n(w, *w & ~bit, __ATOMIC_RELEASE);
		i++;
	}
}

void	arena_init(t_arena *a, const t_arena_cfg *cfg)
{
	heap_lock_init(&a->lock, cfg->name);
	a->bits = cfg->bits;
	a->base = cfg->base;
	a->slots = cfg->slots;
	a->shift = cfg->shift;
	a->taken = 0;
	a->hint = 0;
	memset(a->bits, 0, ((cfg->slots + 63) / 64) * sizeof(uint64_t));
}

uint64_t	arena_take(t_arena *a, uint64_t n)
{
	uint64_t	flags;
	uint64_t	idx;

	flags = heap_lock(&a->lock);
	idx = arena_scan(a, n);
	if (idx != ARENA_NONE)
	{
		arena_mark(a, idx, n, 1);
		a->taken += n;
		while (a->hint < (a->slots + 63) / 64 && a->bits[a->hint] == ~0ull)
			a->hint++;
	}
	heap_unlock(&a->lock, flags);
	return (idx);
}

void	arena_give(t_arena *a, uint64_t idx, uint64_t n)
{
	uint64_t	flags;
	int			held;

	flags = heap_lock(&a->lock);
	held = arena_held(a, idx, n);
	if (held)
	{
		arena_mark(a, idx, n, 0);
		a->taken -= n;
		if ((idx >> 6) < a->hint)
			a->hint = idx >> 6;
	}
	heap_unlock(&a->lock, flags);
	kassert_check(held, "heap: plage non réservée rendue");
}
