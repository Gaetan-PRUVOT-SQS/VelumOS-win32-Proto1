#include "luna_int.h"

static void	wall_row(t_surface *s, const t_lwall *c, int32_t y)
{
	int32_t	x;
	t_color	col;

	x = c->vis.x;
	while (x < c->vis.x + c->vis.w)
	{
		col = lp_wall_blobs(c, lp_min(x - c->ox, c->w - 1),
				lp_min(y - c->oy, c->h - 1));
		col = lp_wall_veils(c, col, lp_min(x - c->ox, c->w - 1),
				lp_min(y - c->oy, c->h - 1));
		s->px[(size_t)y * (size_t)s->stride + (size_t)x] = col | 0xff000000;
		x++;
	}
}

void	luna_wallpaper(t_surface *s, t_rect r)
{
	t_lwall	c;
	int32_t	y;

	r = lp_clean(r);
	if (!lp_ok(s) || r.w <= 0 || r.h <= 0)
		return ;
	lp_wall_setup(&c, r, s);
	y = c.vis.y;
	while (y < c.vis.y + c.vis.h)
	{
		wall_row(s, &c, y);
		y++;
	}
}
