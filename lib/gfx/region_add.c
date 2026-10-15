#include "gfx_int.h"

static bool	region_append(t_region *rg, const t_region *w)
{
	uint32_t	i;

	i = 0;
	while (i < w->n)
	{
		if (rg->n >= GFX_REGION_MAX)
			gfx_region_merge(rg);
		if (rg->n >= GFX_REGION_MAX)
			return (false);
		rg->r[rg->n] = w->r[i];
		rg->n++;
		i++;
	}
	gfx_region_merge(rg);
	return (true);
}

static bool	region_pieces(t_region *w, const t_region *rg)
{
	uint32_t	i;

	i = 0;
	while (i < rg->n && w->n > 0)
	{
		if (!gfx_region_cut(w, rg->r[i]))
			return (false);
		i++;
	}
	return (true);
}

void	region_add(t_region *rg, t_rect r)
{
	t_region	w;
	t_rect		all;

	if (rg == NULL)
		return ;
	gfx_region_check(rg);
	r = rect_intersect(r, rect_make(-GFX_REGION_LIMIT, -GFX_REGION_LIMIT,
				2 * GFX_REGION_LIMIT, 2 * GFX_REGION_LIMIT));
	if (rect_empty(r))
		return ;
	all = rect_union(region_bounds(rg), r);
	w.n = 1;
	w.r[0] = r;
	if (region_pieces(&w, rg) && region_append(rg, &w))
		return ;
	rg->r[0] = all;
	rg->n = 1;
}
