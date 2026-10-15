#include <stdlib.h>
#include "render_edge.h"

int	edge_open(t_edge *e)
{
	size_t	i;

	e->total = (size_t)EDGE_W * (EDGE_H + 2 * EDGE_GUARD);
	e->all = malloc(e->total * sizeof(uint32_t));
	if (!e->all)
		return (0);
	i = 0;
	while (i < e->total)
	{
		e->all[i] = EDGE_SENTINEL;
		i++;
	}
	gfx_surface_init(&e->s, e->all + (size_t)EDGE_W * EDGE_GUARD, EDGE_W,
		EDGE_H);
	gfx_fill(&e->s, rect_make(0, 0, EDGE_W, EDGE_H), EDGE_BG);
	return (1);
}

int	edge_guard_ok(const t_edge *e)
{
	size_t	i;

	i = 0;
	while (i < (size_t)EDGE_W * EDGE_GUARD)
	{
		if (e->all[i] != EDGE_SENTINEL
			|| e->all[e->total - 1 - i] != EDGE_SENTINEL)
			return (0);
		i++;
	}
	return (1);
}
