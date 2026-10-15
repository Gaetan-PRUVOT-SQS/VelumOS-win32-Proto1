#include "luna_int.h"

int32_t	lp_rmask(const t_lglass *g, int32_t x, int32_t y)
{
	t_rect	r;
	int32_t	ix;
	int32_t	rad;
	int32_t	iy;

	r = g->r;
	if (x < r.x || y < r.y || x >= r.x + r.w || y >= r.y + r.h)
		return (0);
	ix = lp_min(x - r.x, r.x + r.w - 1 - x);
	rad = lp_min(g->rad_top, lp_min(r.w, r.h) / 2);
	iy = y - r.y;
	if (iy >= rad)
	{
		rad = lp_min(g->rad_bot, lp_min(r.w, r.h) / 2);
		iy = r.y + r.h - 1 - y;
	}
	if (ix >= rad || iy >= rad)
		return (16);
	return (lp_corner_cov(rad, ix, iy));
}
