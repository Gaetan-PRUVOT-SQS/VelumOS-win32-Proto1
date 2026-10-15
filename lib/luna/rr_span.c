#include "luna_int.h"

void	lp_rr_span(t_surface *s, const t_lrr *rr, t_lspan *sp)
{
	int32_t	cut;

	sp->x0 = lp_max(sp->x0, rr->vis.x);
	sp->x1 = lp_min(sp->x1, rr->vis.x + rr->vis.w);
	if (sp->y < rr->hole.y || sp->y >= rr->hole.y + rr->hole.h
		|| rr->hole.w <= 0)
	{
		lp_fill(s, lp_rect(sp->x0, sp->y, sp->x1 - sp->x0, 1), sp->c);
		return ;
	}
	cut = lp_min(sp->x1, rr->hole.x);
	lp_fill(s, lp_rect(sp->x0, sp->y, cut - sp->x0, 1), sp->c);
	cut = lp_max(sp->x0, rr->hole.x + rr->hole.w);
	lp_fill(s, lp_rect(cut, sp->y, sp->x1 - cut, 1), sp->c);
}

void	lp_rr_edge(t_surface *s, const t_lrr *rr, const t_ledge *e)
{
	int32_t	ix;
	int32_t	px;

	ix = 0;
	while (ix < e->rad)
	{
		px = e->x + e->dir * ix;
		if (px >= rr->vis.x && px < rr->vis.x + rr->vis.w)
			lp_plot(s, lp_pt(px, e->y), e->c, lp_corner_cov(e->rad, ix, e->iy));
		ix++;
	}
}
