#include "luna_int.h"

static void	poly_prep(const t_lpoly *p, t_lpe *e, int64_t sgn)
{
	int32_t	i;
	int32_t	j;
	int64_t	dx;
	int64_t	dy;
	int64_t	len;

	i = 0;
	while (i < p->n)
	{
		j = (i + 1) % p->n;
		dx = p->p[j].x - p->p[i].x;
		dy = p->p[j].y - p->p[i].y;
		len = lp_isqrt((uint64_t)(dx * dx + dy * dy));
		e[i].a = -dy * sgn;
		e[i].b = dx * sgn;
		e[i].c = (dy * p->p[i].x - dx * p->p[i].y) * sgn;
		e[i].inv = 0;
		if (len > 0)
			e[i].inv = 4294967296LL / len;
		i++;
	}
}

static int32_t	poly_cov(const t_lpe *e, int32_t n, int32_t x, int32_t y)
{
	int64_t	dist;
	int64_t	best;
	int32_t	i;

	best = 8;
	i = 0;
	while (i < n)
	{
		if (e[i].inv != 0)
		{
			dist = e[i].a * x + e[i].b * y + e[i].c;
			dist = (dist * e[i].inv) / 4294967296LL;
			if (dist < best)
				best = dist;
			if (best <= -8)
				return (0);
		}
		i++;
	}
	return ((int32_t)best + 8);
}

static void	poly_row(t_surface *s, const t_lpctx *c, int32_t y)
{
	t_point	p;
	int32_t	cov;

	p.y = y;
	p.x = c->vis.x;
	while (p.x < c->vis.x + c->vis.w)
	{
		cov = poly_cov(c->e, c->n, p.x * 16 + 8, y * 16 + 8);
		if (cov > 0)
			lp_plot(s, p, lp_grad_at(c->g, y - c->top), cov);
		p.x++;
	}
}

void	lp_poly(t_surface *s, const t_lpoly *poly, const t_lgrad *g)
{
	t_lpctx	c;
	int32_t	y;

	if (s == NULL || poly == NULL || g == NULL || poly->n < 3
		|| poly->n > LP_PMAX || lp_poly_area2(poly) == 0)
		return ;
	poly_prep(poly, c.e, 1 - 2 * (lp_poly_area2(poly) < 0));
	c.vis = lp_poly_box(poly);
	c.top = c.vis.y;
	c.vis = lp_visible(s, c.vis);
	c.n = poly->n;
	c.g = g;
	y = c.vis.y;
	while (y < c.vis.y + c.vis.h)
	{
		poly_row(s, &c, y);
		y++;
	}
}
