#include "luna_int.h"

static void	frame_title(t_surface *s, const t_lunawin *w, const t_lgeo *g)
{
	t_ltext	t;

	if (w->title == NULL || g->title.w <= 0)
		return ;
	t.font = FONT_TITLE;
	if (g->flags & LG_TOOL)
		t.font = FONT_UI_BOLD;
	t.color = lp_pal()->muted;
	if (w->active)
		t.color = lp_pal()->ink;
	t.shadow = 0;
	t.text = w->title;
	t.box = g->title;
	t.align = LA_LEFT;
	lp_text(s, &t);
}

void	luna_window_frame(t_surface *s, const t_lunawin *w)
{
	t_lgeo	g;

	if (!lp_ok(s) || w == NULL)
		return ;
	lg_make(w, &g);
	if (!(g.flags & LG_FRAMED) || g.outer.w <= 0 || g.outer.h <= 0)
		return ;
	lp_frame_glass(s, &g, w->active);
	if (g.icon.w > 0 && w->icon != ICON_NONE)
		luna_icon(s, lp_pt(g.icon.x, g.icon.y), w->icon, g.icon.w);
	frame_title(s, w, &g);
	lp_capsule(s, w, &g);
}
