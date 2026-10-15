#include "fake.h"

void	region_clear(t_region *rg)
{
	rg->n = 0;
}

bool	region_empty(const t_region *rg)
{
	return (rg->n == 0);
}

t_rect	region_bounds(const t_region *rg)
{
	t_rect		b;
	uint32_t	i;

	b = rect_make(0, 0, 0, 0);
	i = 0;
	while (i < rg->n)
	{
		b = rect_union(b, rg->r[i]);
		i++;
	}
	return (b);
}

static bool	covers(t_rect a, t_rect b)
{
	return (b.x >= a.x && b.y >= a.y && b.x + b.w <= a.x + a.w
		&& b.y + b.h <= a.y + a.h);
}

void	region_add(t_region *rg, t_rect r)
{
	uint32_t	i;
	uint32_t	k;

	if (rect_empty(r))
		return ;
	i = 0;
	k = 0;
	while (i < rg->n)
	{
		if (covers(rg->r[i], r))
			return ;
		if (!covers(r, rg->r[i]))
			rg->r[k++] = rg->r[i];
		i++;
	}
	rg->n = k;
	if (rg->n >= GFX_REGION_MAX)
	{
		rg->r[0] = rect_union(region_bounds(rg), r);
		rg->n = 1;
		return ;
	}
	rg->r[rg->n] = r;
	rg->n++;
}
