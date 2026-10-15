#include "luna_int.h"

t_color	lp_mix(t_color a, t_color b, uint32_t t)
{
	uint32_t	out;
	uint32_t	sh;
	uint32_t	ca;
	uint32_t	cb;

	if (t > 255)
		t = 255;
	out = 0;
	sh = 0;
	while (sh < 32)
	{
		ca = (a >> sh) & 0xff;
		cb = (b >> sh) & 0xff;
		out |= (((ca * (255 - t) + cb * t + 127) / 255) & 0xff) << sh;
		sh += 8;
	}
	return (out);
}

void	lp_plot(t_surface *s, t_point p, t_color c, uint32_t cov)
{
	uint32_t	a;
	t_color		dst;

	a = ((c >> 24) * cov) / 16;
	if (a == 0)
		return ;
	if (a >= 255)
	{
		gfx_put(s, p, c | 0xff000000);
		return ;
	}
	dst = gfx_get(s, p);
	gfx_put(s, p, lp_mix(dst, c, a) | 0xff000000);
}

void	lp_fill(t_surface *s, t_rect r, t_color c)
{
	t_rect	vis;
	t_point	p;

	if (r.w <= 0 || r.h <= 0 || (c >> 24) == 0)
		return ;
	if ((c >> 24) == 255)
	{
		gfx_fill(s, r, c);
		return ;
	}
	vis = lp_visible(s, r);
	p.y = vis.y;
	while (p.y < vis.y + vis.h)
	{
		p.x = vis.x;
		while (p.x < vis.x + vis.w)
		{
			lp_plot(s, p, c, 16);
			p.x++;
		}
		p.y++;
	}
}

void	lp_flat(t_lgrad *g, t_lstop *st, t_color c)
{
	st->at = 0;
	st->c = c;
	g->st = st;
	g->n = 1;
	g->tint = 0;
	g->tint_t = 0;
}

t_rect	lp_visible(const t_surface *s, t_rect r)
{
	t_rect	bounds;

	bounds = lp_rect(0, 0, s->w, s->h);
	return (lp_isect(lp_isect(r, bounds), s->clip));
}
