#include "gfx_int.h"

void	region_clear(t_region *rg)
{
	if (rg != NULL)
		rg->n = 0;
}

t_rect	region_bounds(const t_region *rg)
{
	t_rect		b;
	uint32_t	i;

	b = rect_make(0, 0, 0, 0);
	i = 0;
	while (rg != NULL && i < rg->n && i < GFX_REGION_MAX)
	{
		b = rect_union(b, rg->r[i]);
		i++;
	}
	return (b);
}

bool	region_empty(const t_region *rg)
{
	return (rg == NULL || rg->n == 0);
}

void	gfx_region_check(t_region *rg)
{
	if (rg->n > GFX_REGION_MAX)
		rg->n = GFX_REGION_MAX;
}
