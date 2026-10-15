#include <stdlib.h>
#include "ref.h"

void	guard_new(t_guard *g, size_t count)
{
	size_t	i;

	g->count = count;
	g->raw = malloc((count + 2 * GUARD_WORDS) * sizeof(uint32_t));
	if (g->raw == NULL)
		abort();
	g->px = g->raw + GUARD_WORDS;
	i = 0;
	while (i < GUARD_WORDS)
	{
		g->raw[i] = GUARD_CANARY + (uint32_t)i;
		g->px[count + i] = GUARD_CANARY + (uint32_t)i + 1000;
		i++;
	}
	i = 0;
	while (i < count)
	{
		g->px[i] = 0;
		i++;
	}
}

bool	guard_ok(const t_guard *g)
{
	size_t	i;

	i = 0;
	while (i < GUARD_WORDS)
	{
		if (g->raw[i] != GUARD_CANARY + (uint32_t)i)
			return (false);
		if (g->px[g->count + i] != GUARD_CANARY + (uint32_t)i + 1000)
			return (false);
		i++;
	}
	return (true);
}

void	guard_free(t_guard *g)
{
	free(g->raw);
	g->raw = NULL;
	g->px = NULL;
}

void	guard_randomize(t_guard *g)
{
	size_t	i;

	i = 0;
	while (i < g->count)
	{
		g->px[i] = (uint32_t)rng_next();
		i++;
	}
}
