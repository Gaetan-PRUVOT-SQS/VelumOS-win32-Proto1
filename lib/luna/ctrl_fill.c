#include "luna_ctrl.h"

t_lrfill	lp_rmake(t_rect r, int32_t rad, t_color top, t_color bottom)
{
	t_lrfill	f;

	f.r = lp_clean(r);
	f.rad = lp_clamp(rad, 0, lp_max(lp_min(f.r.w, f.r.h) / 2, 0));
	f.top = top;
	f.bottom = bottom;
	return (f);
}

int32_t	lp_rcov(const t_lrfill *f, t_point p)
{
	int32_t	ix;
	int32_t	iy;

	if (!lp_in(f->r, p))
		return (0);
	ix = lp_min(p.x - f->r.x, f->r.x + f->r.w - 1 - p.x);
	iy = lp_min(p.y - f->r.y, f->r.y + f->r.h - 1 - p.y);
	if (ix >= f->rad || iy >= f->rad)
		return (16);
	return (lp_corner_cov(f->rad, ix, iy));
}

void	lp_rfill(t_surface *s, const t_lrfill *f)
{
	t_rect	vis;
	t_point	p;
	t_color	c;

	if (!lp_ok(s) || f == NULL)
		return ;
	vis = lp_visible(s, f->r);
	p.y = vis.y;
	while (p.y < vis.y + vis.h)
	{
		c = lp_mix(f->top, f->bottom, (uint32_t)((int64_t)(p.y - f->r.y)
					* 255 / lp_max(f->r.h - 1, 1)));
		p.x = vis.x;
		while (p.x < vis.x + vis.w)
		{
			lp_plot(s, p, c, (uint32_t)lp_rcov(f, p));
			p.x++;
		}
		p.y++;
	}
}
