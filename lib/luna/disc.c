#include "luna_int.h"

uint32_t	lp_isqrt(uint64_t v)
{
	uint64_t	res;
	uint64_t	bit;

	res = 0;
	bit = 1ULL << 62;
	while (bit > v)
		bit >>= 2;
	while (bit != 0)
	{
		if (v >= res + bit)
		{
			v -= res + bit;
			res = (res >> 1) + bit;
		}
		else
			res >>= 1;
		bit >>= 2;
	}
	return ((uint32_t)res);
}

static int32_t	edge_cov(int32_t r, int64_t d2)
{
	int64_t	lo;
	int64_t	hi;

	lo = (int64_t)(r - 8) * (r - 8);
	hi = (int64_t)(r + 8) * (r + 8);
	if (r >= 8 && d2 <= lo)
		return (16);
	if (d2 >= hi)
		return (0);
	return (lp_clamp(r - (int32_t)lp_isqrt((uint64_t)d2) + 8, 0, 16));
}

static int32_t	disc_cov(const t_ldisc *d, int32_t x, int32_t y)
{
	int64_t	dx;
	int64_t	dy;
	int32_t	cov;

	dx = x - d->cx;
	dy = y - d->cy;
	cov = edge_cov(d->r, dx * dx + dy * dy);
	if (d->r_in > 0 && cov > 0)
		cov = lp_min(cov, 16 - edge_cov(d->r_in, dx * dx + dy * dy));
	return (cov);
}

void	lp_disc(t_surface *s, const t_ldisc *d, const t_lgrad *g)
{
	t_rect	vis;
	t_point	p;
	int32_t	top;
	int32_t	cov;

	if (s == NULL || d == NULL || g == NULL || d->r <= 0)
		return ;
	top = lp_floor16(d->cy - d->r - 8);
	vis = lp_rect(lp_floor16(d->cx - d->r - 8), top,
			lp_floor16(2 * d->r + 16) + 2, lp_floor16(2 * d->r + 16) + 2);
	vis = lp_visible(s, vis);
	p.y = vis.y;
	while (p.y < vis.y + vis.h)
	{
		p.x = vis.x;
		while (p.x < vis.x + vis.w)
		{
			cov = disc_cov(d, p.x * 16 + 8, p.y * 16 + 8);
			if (cov > 0)
				lp_plot(s, p, lp_grad_at(g, p.y - top), cov);
			p.x++;
		}
		p.y++;
	}
}
