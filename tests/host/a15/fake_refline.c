#include <stdlib.h>
#include "ref.h"

t_grid	*grid_new(int32_t w, int32_t h)
{
	t_grid	*g;

	g = malloc(sizeof(*g));
	if (g == NULL)
		abort();
	g->w = w;
	g->h = h;
	g->on = calloc((size_t)w * (size_t)h, 1);
	if (g->on == NULL)
		abort();
	return (g);
}

void	grid_free(t_grid *g)
{
	free(g->on);
	free(g);
}

bool	grid_get(const t_grid *g, int32_t x, int32_t y)
{
	if (x < 0 || y < 0 || x >= g->w || y >= g->h)
		return (false);
	return (g->on[(size_t)y * (size_t)g->w + (size_t)x] != 0);
}

void	grid_set(t_grid *g, int32_t x, int32_t y, bool v)
{
	if (x < 0 || y < 0 || x >= g->w || y >= g->h)
		return ;
	g->on[(size_t)y * (size_t)g->w + (size_t)x] = v;
}
