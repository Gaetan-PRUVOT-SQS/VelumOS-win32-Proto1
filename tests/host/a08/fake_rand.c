#include "fake.h"

void	fk_seed(uint64_t seed)
{
	g_fk.rng = seed;
	if (!g_fk.rng)
		g_fk.rng = 0x9e3779b97f4a7c15ull;
}

uint64_t	fk_rand(void)
{
	uint64_t	x;

	x = g_fk.rng;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	g_fk.rng = x;
	return (x);
}
