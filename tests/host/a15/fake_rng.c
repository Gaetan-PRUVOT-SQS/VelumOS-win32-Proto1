#include "ref.h"

static uint64_t	g_state = 88172645463325252ull;

void	rng_seed(uint64_t seed)
{
	g_state = seed;
	if (g_state == 0)
		g_state = 88172645463325252ull;
}

uint64_t	rng_next(void)
{
	g_state ^= g_state >> 12;
	g_state ^= g_state << 25;
	g_state ^= g_state >> 27;
	return (g_state * 2685821657736338717ull);
}

int64_t	rng_range(int64_t lo, int64_t hi)
{
	return (lo + (int64_t)(rng_next() % (uint64_t)(hi - lo + 1)));
}

t_color	rng_color(void)
{
	uint64_t	k;

	k = rng_next() % 8;
	if (k == 0)
		return (0xff000000u | (uint32_t)rng_next());
	if (k == 1)
		return ((uint32_t)rng_next() & 0x00ffffffu);
	if (k == 2)
		return (0x80000000u | ((uint32_t)rng_next() & 0x00ffffffu));
	return ((uint32_t)rng_next());
}

int32_t	rng_coord(int32_t span)
{
	uint64_t	k;

	k = rng_next() % 16;
	if (k < 7)
		return ((int32_t)rng_range(-12, span + 12));
	if (k < 9)
		return ((int32_t)rng_range(-3, 3) + (int32_t)(rng_next() % 2) * span);
	if (k == 9)
		return (INT32_MIN + (int32_t)rng_range(0, 3));
	if (k == 10)
		return (INT32_MAX - (int32_t)rng_range(0, 3));
	if (k == 11)
		return ((int32_t)rng_range(-1073741824, 1073741823));
	if (k == 12)
		return ((int32_t)rng_range(-70000, 70000));
	return ((int32_t)(uint32_t)rng_next());
}
