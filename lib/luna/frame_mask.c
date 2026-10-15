#include "luna_int.h"

int32_t	luna_client_band(const t_lunawin *w)
{
	t_lgeo	g;

	if (w == NULL)
		return (0);
	lg_make(w, &g);
	if (!(g.flags & LG_FRAMED))
		return (0);
	return (lp_min(lp_frame_radius(&g), g.client.h));
}

static t_color	band_color(const t_surface *src, t_point p, t_rect cl,
					t_color fb)
{
	int32_t	u;
	int32_t	v;

	u = p.x - cl.x;
	v = p.y - cl.y;
	if (src == NULL || src->px == NULL || src->stride < src->w)
		return (fb);
	if (u >= src->w || v >= src->h)
		return (fb);
	return (src->px[(size_t)v * (size_t)src->stride + (size_t)u]);
}

static void	band_pixel(t_surface *s, const t_lglass *m, t_point p, t_color c)
{
	int32_t	cov;

	cov = lp_rmask(m, p.x, p.y);
	if (cov >= 16)
		gfx_put(s, p, c | 0xff000000);
	else if (cov > 0)
		gfx_put(s, p, lp_mix(gfx_get(s, p), c, (uint32_t)(cov * 255 / 16))
			| 0xff000000);
}

static void	band_mask(const t_lgeo *g, t_lglass *m)
{
	m->r = g->outer;
	m->hole = lp_rect(0, 0, 0, 0);
	m->rad_top = 0;
	m->rad_bot = lp_frame_radius(g);
}

void	luna_client_band_paint(t_surface *s, const t_lunawin *w,
			const t_surface *src, t_color fb)
{
	t_lgeo		g;
	t_lglass	m;
	t_point		p;
	t_rect		v;

	if (!lp_ok(s) || w == NULL)
		return ;
	lg_make(w, &g);
	band_mask(&g, &m);
	v = lp_visible(s, g.client);
	p.y = lp_max(v.y, g.client.y + g.client.h - luna_client_band(w)) - 1;
	while (++p.y < v.y + v.h)
	{
		p.x = v.x - 1;
		while (++p.x < v.x + v.w)
			band_pixel(s, &m, p, band_color(src, p, g.client, fb));
	}
}
