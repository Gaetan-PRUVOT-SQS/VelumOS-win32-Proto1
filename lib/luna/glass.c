#include "velum/theme.h"
#include "luna_int.h"

static t_color	glass_pixel(const t_lglass *g, const t_lgb *o, t_point p)
{
	t_color	c;
	int32_t	spec_h;
	int32_t	a;

	c = g->opaque;
	if (o != NULL)
	{
		c = lp_saturate(lp_gb_sample(o, p.x, p.y), g->sat);
		c = lp_mix(c, g->fill | 0xff000000, g->fill >> 24);
	}
	spec_h = g->r.h * VT_SPEC_END_PCT / 100;
	if (spec_h > 0 && p.y - g->r.y < spec_h)
	{
		a = (int32_t)(g->spec >> 24) * (spec_h - (p.y - g->r.y)) / spec_h;
		c = lp_mix(c, g->spec | 0xff000000, (uint32_t)a);
	}
	return (c);
}

static void	glass_row(t_surface *s, const t_lglass *g, const t_lgb *o, t_rect v)
{
	t_point	p;
	int32_t	cov;
	t_color	c;

	p.y = v.y;
	p.x = v.x - 1;
	while (++p.x < v.x + v.w)
	{
		if (lp_in(g->hole, p))
			continue ;
		cov = lp_rmask(g, p.x, p.y);
		if (cov == 0)
			continue ;
		c = glass_pixel(g, o, p);
		if (cov < 16)
			c = lp_mix(gfx_get(s, p), c, (uint32_t)(cov * 255 / 16));
		gfx_put(s, p, c | 0xff000000);
	}
}

static bool	glass_blurred(const t_surface *s, const t_lglass *g, t_lgb *o)
{
	t_rect	src;
	size_t	px;

	src = lp_isect(g->r, lp_rect(0, 0, s->w, s->h));
	px = (size_t)((src.w + LX_SCALE - 1) / LX_SCALE)
		* (size_t)((src.h + LX_SCALE - 1) / LX_SCALE);
	if (luna_transparency_reduced() || src.w <= 0 || src.h <= 0
		|| !lp_gscratch(px, &o->a, &o->b))
		return (false);
	lp_gb_down(s, src, o);
	lp_gb_blur(o, lp_max(g->blur / LX_SCALE, 1));
	return (true);
}

void	lp_glass(t_surface *s, const t_lglass *in)
{
	t_lglass	g;
	t_lgb		o;
	const t_lgb	*blur;
	t_rect		v;

	if (!lp_ok(s) || in == NULL)
		return ;
	g = *in;
	g.r = lp_clean(in->r);
	if (g.r.w <= 0 || g.r.h <= 0)
		return ;
	v = lp_visible(s, g.r);
	if (v.w <= 0 || v.h <= 0)
		return ;
	blur = NULL;
	if (glass_blurred(s, &g, &o))
		blur = &o;
	while (v.h > 0)
	{
		glass_row(s, &g, blur, lp_rect(v.x, v.y, v.w, 1));
		v.y++;
		v.h--;
	}
	lp_glass_rim(s, &g);
}
