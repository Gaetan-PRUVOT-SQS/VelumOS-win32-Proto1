#include "rng_int.h"
#include "velum/random.h"

static t_rng	g_rng = {{0, 0, "random"}, &g_rng_env, {{0}, 0, {0}, 0},
	0, 0, 0, 0, 0, 0, {0}};

t_rng	*rng_global(void)
{
	return (&g_rng);
}

void	krandom(void *buf, size_t n)
{
	rng_read(&g_rng, buf, n);
}

uint64_t	krandom_u64(void)
{
	uint64_t	v;

	krandom(&v, sizeof(v));
	return (v);
}

uint64_t	krandom_below(uint64_t bound)
{
	uint64_t	threshold;
	uint64_t	v;

	if (bound < 2)
		return (0);
	threshold = (0 - bound) % bound;
	v = krandom_u64();
	while (v < threshold)
		v = krandom_u64();
	return (v % bound);
}

void	random_add_entropy(const void *data, size_t n)
{
	rng_add(&g_rng, data, n);
}
