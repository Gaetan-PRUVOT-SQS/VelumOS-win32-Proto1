#include "lt_bbox.h"

static void	bbox_add(t_rect *r, int32_t x, int32_t y)
{
	if (r->w == 0)
	{
		*r = lp_rect(x, y, 1, 1);
		return ;
	}
	if (x < r->x)
	{
		r->w += r->x - x;
		r->x = x;
	}
	if (y < r->y)
	{
		r->h += r->y - y;
		r->y = y;
	}
	if (x >= r->x + r->w)
		r->w = x - r->x + 1;
	if (y >= r->y + r->h)
		r->h = y - r->y + 1;
}

t_rect	ltb_bbox(const t_surface *a, const t_surface *b, t_color bg)
{
	t_rect	r;
	t_color	ref;
	t_point	p;

	r = lp_rect(0, 0, 0, 0);
	p.y = 0;
	while (p.y < a->h)
	{
		p.x = 0;
		while (p.x < a->w)
		{
			ref = bg;
			if (b != NULL)
				ref = gfx_get(b, p);
			if (gfx_get(a, p) != ref)
				bbox_add(&r, p.x, p.y);
			p.x++;
		}
		p.y++;
	}
	return (r);
}

bool	ltb_inside(t_rect in, t_rect out)
{
	return (in.x >= out.x && in.y >= out.y && in.x + in.w <= out.x + out.w
		&& in.y + in.h <= out.y + out.h);
}
