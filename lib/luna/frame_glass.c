#include "luna_int.h"

int32_t	lp_frame_radius(const t_lgeo *g)
{
	if (g->flags & LG_MAX)
		return (0);
	return (lm_detail()->title_radius);
}

int32_t	lp_frame_bands(const t_lgeo *g, t_rect *out)
{
	int32_t	rad;

	rad = lp_frame_radius(g);
	if (g->outer.h <= g->cap_end + rad)
	{
		out[0] = g->outer;
		return (1);
	}
	out[0] = lp_rect(g->outer.x, g->outer.y, g->outer.w, g->cap_end);
	if (rad == 0)
		return (1);
	out[1] = lp_rect(g->outer.x, g->outer.y + g->outer.h - rad, g->outer.w,
			rad);
	return (2);
}

static void	band_paint(t_surface *s, t_lglass *b, t_rect r, bool active)
{
	b->r = r;
	if (!active)
		b->spec = (b->spec & 0x00ffffff) | ((b->spec >> 25) << 24);
	lp_glass(s, b);
}

void	lp_frame_glass(t_surface *s, const t_lgeo *g, bool active)
{
	t_lglass	b;
	t_rect		r[2];
	int32_t		n;

	if (!lp_glass_make(&b, g->outer, LUNA_GLASS_WINDOW))
		return ;
	n = lp_frame_bands(g, r);
	b.rad_top = lp_frame_radius(g);
	b.rad_bot = b.rad_top;
	if (n == 2)
	{
		b.rad_bot = 0;
		b.rim[2] = 0;
	}
	band_paint(s, &b, r[0], active);
	if (n < 2)
		return ;
	b.rad_top = 0;
	b.rad_bot = lp_frame_radius(g);
	b.rim[0] = 0;
	b.rim[2] = lp_pal()->rim_bottom;
	b.spec = 0;
	band_paint(s, &b, r[1], active);
}

int32_t	luna_window_glass_bands(const t_lunawin *w, t_rect *out)
{
	t_lgeo	g;

	if (w == NULL || out == NULL)
		return (0);
	lg_make(w, &g);
	if (!(g.flags & LG_FRAMED) || g.outer.w <= 0 || g.outer.h <= 0)
		return (0);
	return (lp_frame_bands(&g, out));
}
