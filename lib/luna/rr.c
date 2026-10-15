#include "luna_int.h"

static void	rr_clamp(const t_lrr *rr, t_lrr *c)
{
	int32_t	lim;
	int32_t	k;

	*c = *rr;
	lim = lp_min(lp_min(rr->r.w, rr->r.h) / 2, LP_RMAX);
	k = 0;
	while (k < 4)
	{
		c->rad[k] = lp_clamp(rr->rad[k], 0, lim);
		k++;
	}
}

static int32_t	rr_rad(int32_t top, int32_t bot, int32_t dt, int32_t db)
{
	if (dt < top)
		return (top);
	if (db < bot)
		return (bot);
	return (0);
}

static int32_t	rr_side(t_surface *s, const t_lrr *c, int32_t y, int32_t right)
{
	t_ledge	e;
	int32_t	dt;
	int32_t	db;

	dt = y - c->r.y;
	db = c->r.y + c->r.h - 1 - y;
	e.rad = rr_rad(c->rad[right], c->rad[3 - right], dt, db);
	e.x = c->r.x + right * (c->r.w - 1);
	e.y = y;
	e.dir = 1 - 2 * right;
	e.iy = lp_min(dt, db);
	e.c = lp_grad_at(c->g, y - c->y0);
	lp_rr_edge(s, c, &e);
	return (e.rad);
}

void	lp_rr_paint(t_surface *s, const t_lrr *rr)
{
	t_lrr	c;
	t_lspan	sp;

	if (s == NULL || rr == NULL || rr->g == NULL)
		return ;
	rr_clamp(rr, &c);
	c.vis = lp_visible(s, rr->r);
	sp.y = c.vis.y;
	while (sp.y < c.vis.y + c.vis.h)
	{
		sp.c = lp_grad_at(c.g, sp.y - c.y0);
		sp.x0 = c.r.x + rr_side(s, &c, sp.y, 0);
		sp.x1 = c.r.x + c.r.w - rr_side(s, &c, sp.y, 1);
		lp_rr_span(s, &c, &sp);
		sp.y++;
	}
}
