#include "luna_ctrl.h"

static void	ring_row(t_surface *s, const t_lrfill *f, const t_lrfill *in,
				int32_t y)
{
	t_point	p;
	t_rect	vis;
	int32_t	cov;

	vis = lp_visible(s, f->r);
	p.y = y;
	p.x = vis.x;
	while (p.x < vis.x + vis.w)
	{
		cov = lp_rcov(f, p) - lp_rcov(in, p);
		if (cov > 0)
			lp_plot(s, p, f->top, (uint32_t)cov);
		p.x++;
	}
}

void	lp_rring(t_surface *s, const t_lrfill *f, int32_t w)
{
	t_lrfill	in;
	t_rect		vis;
	int32_t		y;

	if (!lp_ok(s) || f == NULL || w <= 0)
		return ;
	w = lp_min(w, LP_RMAX);
	in = lp_rmake(lp_inset(f->r, w), f->rad - w, 0, 0);
	vis = lp_visible(s, f->r);
	y = vis.y;
	while (y < vis.y + vis.h)
	{
		ring_row(s, f, &in, y);
		y++;
	}
}

void	lp_rtop(t_surface *s, const t_lrfill *f)
{
	t_rect	vis;
	t_point	p;

	if (!lp_ok(s) || f == NULL || f->r.h <= 0)
		return ;
	vis = lp_visible(s, lp_rect(f->r.x, f->r.y, f->r.w, 1));
	p.y = vis.y;
	p.x = vis.x;
	while (p.x < vis.x + vis.w)
	{
		lp_plot(s, p, f->top, (uint32_t)lp_rcov(f, p));
		p.x++;
	}
}

void	lp_rfocus(t_surface *s, t_rect r, int32_t rad, t_color c)
{
	t_lrfill	f;
	int32_t		m;

	m = LCT_FOCUS_GAP + LCT_FOCUS_W;
	f = lp_rmake(lp_inset(lp_clean(r), -m), rad + m, c, c);
	lp_rring(s, &f, LCT_FOCUS_W);
}
