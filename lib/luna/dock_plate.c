#include "dock_int.h"

int32_t	ld_cov(const t_ldplate *p, int32_t x, int32_t y)
{
	int32_t	dx;
	int32_t	dy;
	int32_t	rad;

	if (x < p->r.x || y < p->r.y || x >= p->r.x + p->r.w
		|| y >= p->r.y + p->r.h)
		return (0);
	rad = lp_min(p->rad, lp_min(p->r.w, p->r.h) / 2);
	dx = lp_min(x - p->r.x, p->r.x + p->r.w - 1 - x);
	dy = lp_min(y - p->r.y, p->r.y + p->r.h - 1 - y);
	if (rad <= 0 || dx >= rad || dy >= rad)
		return (16);
	return (lp_corner_cov(rad, dx, dy));
}

static t_ldplate	ld_bound(const t_ldplate *p)
{
	t_ldplate	c;

	c = *p;
	c.r = lp_clean(p->r);
	c.rad = lp_clamp(p->rad, 0, LP_COORD_MAX);
	c.thick = lp_clamp(p->thick, 0, lp_min(c.r.w, c.r.h) / 2);
	return (c);
}

void	ld_paint(t_surface *s, const t_ldplate *p, t_ldcov fn)
{
	t_ldplate	c;
	t_rect		v;
	t_point		q;

	c = ld_bound(p);
	if (!lp_ok(s) || c.r.w <= 0 || c.r.h <= 0)
		return ;
	v = lp_visible(s, c.r);
	q.y = v.y;
	while (q.y < v.y + v.h)
	{
		q.x = v.x;
		while (q.x < v.x + v.w)
		{
			lp_plot(s, q, c.c, (uint32_t)lp_clamp(fn(&c, q.x, q.y), 0, 16));
			q.x++;
		}
		q.y++;
	}
}

void	ld_plate(t_surface *s, const t_ldplate *p)
{
	ld_paint(s, p, ld_cov);
}
