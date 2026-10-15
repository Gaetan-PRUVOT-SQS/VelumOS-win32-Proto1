#include "gfx_int.h"

void	region_subtract(t_region *rg, t_rect r)
{
	t_rect	all;

	if (rg == NULL)
		return ;
	gfx_region_check(rg);
	if (rect_empty(r) || rg->n == 0)
		return ;
	all = region_bounds(rg);
	if (!gfx_region_cut(rg, r))
	{
		rg->r[0] = all;
		rg->n = 1;
		return ;
	}
	gfx_region_merge(rg);
}

void	region_intersect(t_region *rg, t_rect r)
{
	uint32_t	i;
	uint32_t	kept;
	t_rect		c;

	if (rg == NULL)
		return ;
	gfx_region_check(rg);
	i = 0;
	kept = 0;
	while (i < rg->n)
	{
		c = rect_intersect(rg->r[i], r);
		if (!rect_empty(c))
		{
			rg->r[kept] = c;
			kept++;
		}
		i++;
	}
	rg->n = kept;
	gfx_region_merge(rg);
}
