#include "gfx_int.h"

bool	gfx_rect_join(t_rect a, t_rect b, t_rect *out)
{
	t_box	p;
	t_box	q;
	t_box	u;

	if (rect_empty(a) || rect_empty(b))
		return (false);
	p = gfx_box_of(a);
	q = gfx_box_of(b);
	u = gfx_box_make(gfx_min64(p.x0, q.x0), gfx_min64(p.y0, q.y0),
			gfx_max64(p.x1, q.x1), gfx_max64(p.y1, q.y1));
	if (u.x1 - u.x0 > INT32_MAX || u.y1 - u.y0 > INT32_MAX)
		return (false);
	if ((p.y0 == q.y0 && p.y1 == q.y1 && (p.x1 == q.x0 || q.x1 == p.x0))
		|| (p.x0 == q.x0 && p.x1 == q.x1 && (p.y1 == q.y0 || q.y1 == p.y0)))
	{
		*out = gfx_box_rect(u);
		return (true);
	}
	return (false);
}

static bool	merge_pair(t_region *rg)
{
	uint32_t	i;
	uint32_t	j;
	t_rect		u;

	i = 0;
	while (i < rg->n)
	{
		j = i + 1;
		while (j < rg->n)
		{
			if (gfx_rect_join(rg->r[i], rg->r[j], &u))
			{
				rg->r[i] = u;
				rg->n--;
				rg->r[j] = rg->r[rg->n];
				return (true);
			}
			j++;
		}
		i++;
	}
	return (false);
}

void	gfx_region_merge(t_region *rg)
{
	bool	again;

	again = true;
	while (again)
		again = merge_pair(rg);
}
