#include "heap_int.h"

static int	arena_bit(const t_arena *a, uint64_t idx)
{
	return ((int)((a->bits[idx >> 6] >> (idx & 63)) & 1));
}

static uint64_t	arena_next(const t_arena *a, uint64_t idx)
{
	idx++;
	while ((idx & 63) == 0 && idx < a->slots && a->bits[idx >> 6] == ~0ull)
		idx += 64;
	return (idx);
}

uint64_t	arena_scan(const t_arena *a, uint64_t n)
{
	uint64_t	idx;
	uint64_t	start;
	uint64_t	run;

	idx = a->hint << 6;
	start = 0;
	run = 0;
	while (n > 0 && idx < a->slots)
	{
		if (arena_bit(a, idx))
		{
			run = 0;
			idx = arena_next(a, idx);
			continue ;
		}
		if (run == 0)
			start = idx;
		run++;
		if (run == n)
			return (start);
		idx++;
	}
	return (ARENA_NONE);
}

int	arena_held(const t_arena *a, uint64_t idx, uint64_t n)
{
	uint64_t	i;

	if (n == 0 || idx >= a->slots || n > a->slots - idx)
		return (0);
	i = 0;
	while (i < n)
	{
		if (!arena_bit(a, idx + i))
			return (0);
		i++;
	}
	return (1);
}
