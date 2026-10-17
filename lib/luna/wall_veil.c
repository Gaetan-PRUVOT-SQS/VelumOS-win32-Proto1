#include "luna_int.h"

int32_t	lp_smooth256(int32_t t)
{
	t = lp_clamp(t, 0, 256);
	return ((t * t * (768 - 2 * t)) >> 16);
}

static int32_t	veil_mask(const t_lvc *v, int32_t u, int32_t w)
{
	int64_t	nu;
	int64_t	nv;
	int64_t	d;
	int64_t	m;
	int64_t	sd;

	nu = ((int64_t)u * 1024) / v->a;
	nv = ((int64_t)w * 1024) / v->b;
	d = lp_isqrt((uint64_t)(nu * nu + nv * nv));
	m = (nu * nu * v->a + nv * nv * v->b) / (nu * nu + nv * nv + 1);
	sd = (d - 1024) * m / 1024;
	return (lp_smooth256(128 - (int32_t)(sd * 256 / (3 * v->sigma))));
}

static int32_t	veil_alpha(const t_lvc *v, int32_t x, int32_t y)
{
	int32_t	dx;
	int32_t	dy;
	int32_t	u;
	int32_t	w;
	int32_t	g;

	dx = x - v->cx;
	dy = y - v->cy;
	u = (dx * v->co + dy * v->si) / 1024;
	w = (dy * v->co - dx * v->si) / 1024;
	if (lp_max(u, -u) > v->a + 2 * v->sigma
		|| lp_max(w, -w) > v->b + 2 * v->sigma)
		return (0);
	g = (v->b + w) * 256 / (2 * v->b);
	if (v->down)
		g = 256 - g;
	g = lp_clamp(g, 0, 256);
	return (veil_mask(v, u, w) * g / 256 * v->peak / 256);
}

t_color	lp_wall_veils(const t_lwall *c, t_color col, int32_t x, int32_t y)
{
	int32_t	i;
	int32_t	a;

	i = 0;
	while (i < LP_VEILS)
	{
		a = veil_alpha(&c->vc[i], x, y);
		if (a > 0)
			col = lp_mix(col, LC_WHITE, (uint32_t)a);
		i++;
	}
	return (col);
}
