#include "luna_int.h"

static void	rim_corner(t_surface *s, t_rect r, int32_t rad, t_color c)
{
	t_lcring	k;

	k = (t_lcring){r.x, r.y, 1, 1, rad, c};
	lp_corner_ring(s, &k);
	k.x = r.x + r.w - 1;
	k.dx = -1;
	lp_corner_ring(s, &k);
}

static void	rim_bottom(t_surface *s, t_rect r, int32_t rad, t_color c)
{
	t_lcring	k;

	k = (t_lcring){r.x, r.y + r.h - 1, 1, -1, rad, c};
	lp_corner_ring(s, &k);
	k.x = r.x + r.w - 1;
	k.dx = -1;
	lp_corner_ring(s, &k);
}

static void	rim_row(t_surface *s, t_rect span, t_color base, t_color extra)
{
	lp_fill(s, span, base);
	lp_fill(s, span, extra);
}

void	lp_glass_rim(t_surface *s, const t_lglass *g)
{
	t_rect	r;
	int32_t	rt;
	int32_t	rb;

	r = g->r;
	rt = lp_clamp(g->rad_top, 0, lp_min(r.w, r.h) / 2);
	rb = lp_clamp(g->rad_bot, 0, lp_min(r.w, r.h) / 2);
	rim_corner(s, r, rt, g->rim[1]);
	rim_bottom(s, r, rb, g->rim[1]);
	rim_row(s, lp_rect(r.x + rt, r.y, r.w - 2 * rt, 1), g->rim[1], g->rim[0]);
	rim_row(s, lp_rect(r.x + rb, r.y + r.h - 1, r.w - 2 * rb, 1), g->rim[1],
		g->rim[2]);
	lp_fill(s, lp_rect(r.x, r.y + rt, 1, r.h - rt - rb), g->rim[1]);
	lp_fill(s, lp_rect(r.x + r.w - 1, r.y + rt, 1, r.h - rt - rb), g->rim[1]);
}
