#include "luna_int.h"

typedef struct s_cspan
{
	int32_t	lo;
	int32_t	hi;
}	t_cspan;

static t_cspan	ring_span(int32_t origin, int32_t dir, int32_t extent,
	int32_t rad)
{
	int64_t	lo;
	int64_t	hi;
	t_cspan	sp;

	lo = -(int64_t)origin;
	hi = (int64_t)extent - 1 - origin;
	if (dir < 0)
	{
		lo = (int64_t)origin - extent + 1;
		hi = origin;
	}
	if (lo < 0)
		lo = 0;
	if (hi > (int64_t)rad - 1)
		hi = (int64_t)rad - 1;
	sp.lo = (int32_t)lo;
	sp.hi = (int32_t)hi;
	if (lo > hi)
	{
		sp.lo = 0;
		sp.hi = -1;
	}
	return (sp);
}

static int32_t	ring_cov(const t_lcring *k, int32_t ix, int32_t iy)
{
	int32_t	cov;

	cov = lp_corner_cov(k->rad, ix, iy);
	if (ix > 0 && iy > 0)
		cov -= lp_corner_cov(k->rad - 1, ix - 1, iy - 1);
	return (lp_clamp(cov, 0, 16));
}

void	lp_corner_ring(t_surface *s, const t_lcring *k)
{
	t_cspan	sx;
	t_cspan	sy;
	int32_t	ix;
	int32_t	iy;

	if (!lp_ok(s) || k == NULL)
		return ;
	sx = ring_span(k->x, k->dx, s->w, k->rad);
	sy = ring_span(k->y, k->dy, s->h, k->rad);
	iy = sy.lo;
	while (iy <= sy.hi)
	{
		ix = sx.lo;
		while (ix <= sx.hi)
		{
			lp_plot(s, lp_pt(k->x + k->dx * ix, k->y + k->dy * iy), k->c,
				(uint32_t)ring_cov(k, ix, iy));
			ix++;
		}
		iy++;
	}
}
