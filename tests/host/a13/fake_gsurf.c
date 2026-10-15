#include <stdlib.h>
#include "fakes.h"

bool	fake_gsurf_new(t_gsurf *g, int32_t w, int32_t h, int32_t stride)
{
	size_t	i;

	g->count = (size_t)stride * (size_t)h;
	g->base = malloc((g->count + 2 * FAKE_GUARD) * sizeof(uint32_t));
	if (!g->base)
		return (false);
	i = 0;
	while (i < g->count + 2 * FAKE_GUARD)
		g->base[i++] = FAKE_GUARD_WORD;
	g->px = g->base + FAKE_GUARD;
	memset(g->px, 0, g->count * sizeof(uint32_t));
	g->s.px = g->px;
	g->s.w = w;
	g->s.h = h;
	g->s.stride = stride;
	g->s.clip = rect_make(0, 0, w, h);
	return (true);
}

bool	fake_gsurf_ok(const t_gsurf *g)
{
	size_t	i;

	i = 0;
	while (i < FAKE_GUARD)
	{
		if (g->base[i] != FAKE_GUARD_WORD)
			return (false);
		if (g->px[g->count + i] != FAKE_GUARD_WORD)
			return (false);
		i++;
	}
	return (true);
}

void	fake_gsurf_free(t_gsurf *g)
{
	free(g->base);
	g->base = NULL;
}
