#include "luna_int.h"

static t_rect	lg_sane(t_rect r)
{
	if (r.x < -LG_POS_MAX || r.x > LG_POS_MAX || r.y < -LG_POS_MAX
		|| r.y > LG_POS_MAX)
		return (lp_rect(0, 0, 0, 0));
	r.w = lp_clamp(r.w, 0, LG_DIM_MAX);
	r.h = lp_clamp(r.h, 0, LG_DIM_MAX);
	return (r);
}

static uint32_t	lg_flags(const t_lunawin *w)
{
	uint32_t	f;

	if ((w->style & WS_CAPTION) == 0 || (w->style & LG_FRAMELESS_STYLES))
		return (0);
	f = LG_FRAMED;
	if (w->style & WS_TOOLWINDOW)
		f |= LG_TOOL;
	else if (w->maximized)
		f |= LG_MAX;
	if ((w->style & WS_SIZEBOX) && !(f & LG_MAX))
		f |= LG_SIZABLE;
	if (w->style & WS_SYSMENU)
		f |= LG_SYSMENU;
	return (f);
}

static void	lg_dims(t_lgeo *g)
{
	t_cmet	m;

	m = lm_metrics();
	g->border = 0;
	g->top = 0;
	g->bottom = 0;
	g->cap_end = 0;
	if (!(g->flags & LG_FRAMED))
		return ;
	g->cap_end = m->caption_h;
	if (g->flags & LG_TOOL)
		g->cap_end = lm_detail()->tool_caption_h;
	if (g->flags & LG_MAX)
	{
		g->cap_end = m->caption_h - m->frame_w;
		return ;
	}
	g->border = m->frame_w;
	g->top = m->frame_w;
	g->bottom = m->frame_bottom;
}

static void	lg_caption(t_lgeo *g)
{
	t_rect	nominal;

	g->caption = lp_rect(g->outer.x, g->outer.y, 0, 0);
	if (!(g->flags & LG_FRAMED))
		return ;
	nominal = lp_rect(g->outer.x + g->border, g->outer.y + g->top,
			g->outer.w - 2 * g->border, g->cap_end - g->top);
	g->caption = lp_isect(nominal, g->outer);
}

void	lg_make(const t_lunawin *w, t_lgeo *g)
{
	g->outer = lg_sane(w->outer);
	g->flags = lg_flags(w);
	lg_dims(g);
	lg_caption(g);
	lg_buttons(w, g);
	lg_zones(g);
}
