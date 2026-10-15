#include "rand_int.h"
#include "stdlib.h"
#include "string.h"
#include "velum/vmisc.h"
#include "velum/vtime.h"

static t_rand	g_rand;

static uint32_t	rand_xorshift(void)
{
	uint64_t	x;

	x = g_rand.state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	g_rand.state = x;
	return ((uint32_t)((x * 0x2545f4914f6cdd1dull) >> 32));
}

static void	rand_refill(void)
{
	uint32_t	i;
	uint32_t	v;

	g_rand.pos = 0;
	if (v_getrandom(g_rand.pool, RAND_POOL, 0) == RAND_POOL)
		return ;
	g_rand.state = (uint64_t)v_time_mono() ^ 0x9e3779b97f4a7c15ull;
	i = 0;
	while (i < RAND_POOL)
	{
		v = rand_xorshift();
		memcpy(g_rand.pool + i, &v, sizeof(v));
		i += sizeof(v);
	}
}

int	rand(void)
{
	uint32_t	v;

	v_spin_lock(&g_rand.lock);
	if (g_rand.seeded)
		v = rand_xorshift();
	else
	{
		if (g_rand.pos + sizeof(v) > RAND_POOL || !g_rand.state)
			rand_refill();
		memcpy(&v, g_rand.pool + g_rand.pos, sizeof(v));
		g_rand.pos += sizeof(v);
		g_rand.state |= 1;
	}
	v_spin_unlock(&g_rand.lock);
	return ((int)(v >> 1));
}

void	srand(unsigned int seed)
{
	v_spin_lock(&g_rand.lock);
	g_rand.seeded = 1;
	g_rand.state = ((uint64_t)seed * 0x9e3779b97f4a7c15ull) | 1;
	v_spin_unlock(&g_rand.lock);
}
