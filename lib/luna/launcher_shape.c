#include "launcher_int.h"

static const int32_t	g_dir[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};

void	lp_launch_fill(t_surface *s, t_rect r, int32_t rad, t_color c)
{
	t_lrr	rr;
	t_lgrad	g;
	t_lstop	st;
	int32_t	k;

	r = lp_clean(r);
	if (!lp_ok(s) || r.w <= 0 || r.h <= 0 || (c >> 24) == 0)
		return ;
	lp_flat(&g, &st, c);
	rr.r = r;
	k = 0;
	while (k < 4)
		rr.rad[k++] = rad;
	rr.g = &g;
	rr.y0 = r.y;
	rr.hole = lp_rect(0, 0, 0, 0);
	rr.vis = lp_visible(s, r);
	lp_rr_paint(s, &rr);
}

static void	ring_corners(t_surface *s, t_rect r, int32_t rad, t_color c)
{
	t_lcring	k;
	int32_t		i;

	i = 0;
	while (i < 4)
	{
		k.dx = g_dir[i][0];
		k.dy = g_dir[i][1];
		k.x = r.x + (k.dx < 0) * (r.w - 1);
		k.y = r.y + (k.dy < 0) * (r.h - 1);
		k.rad = rad;
		k.c = c;
		lp_corner_ring(s, &k);
		i++;
	}
}

void	lp_launch_ring(t_surface *s, t_rect r, int32_t rad, t_color c)
{
	r = lp_clean(r);
	if (!lp_ok(s) || r.w < 2 || r.h < 2)
		return ;
	rad = lp_clamp(rad, 0, lp_min(r.w, r.h) / 2);
	ring_corners(s, r, rad, c);
	lp_fill(s, lp_rect(r.x + rad, r.y, r.w - 2 * rad, 1), c);
	lp_fill(s, lp_rect(r.x + rad, r.y + r.h - 1, r.w - 2 * rad, 1), c);
	lp_fill(s, lp_rect(r.x, r.y + rad, 1, r.h - 2 * rad), c);
	lp_fill(s, lp_rect(r.x + r.w - 1, r.y + rad, 1, r.h - 2 * rad), c);
}

void	lp_launch_focus(t_surface *s, t_rect r, int32_t rad)
{
	t_color	c;

	c = lp_pal()->accent;
	lp_launch_ring(s, lp_inset(r, -(LL_FOCUS_GAP + 2)), rad + LL_FOCUS_GAP + 2,
		c);
	lp_launch_ring(s, lp_inset(r, -(LL_FOCUS_GAP + 1)), rad + LL_FOCUS_GAP + 1,
		c);
}

t_color	lp_launch_tone(t_color light, t_color dark)
{
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (dark);
	return (light);
}
