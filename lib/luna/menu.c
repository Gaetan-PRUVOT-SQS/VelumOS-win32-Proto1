#include "luna_ctrl.h"

static void	panel_paint(t_surface *s, t_rect r, int32_t rad)
{
	t_cpal		pal;
	t_lrfill	f;
	t_color		edge;

	pal = lp_pal();
	edge = lp_mix(pal->opaque_panel, pal->ink, 46);
	f = lp_rmake(r, rad, edge, edge);
	lp_rfill(s, &f);
	f = lp_rmake(lp_inset(f.r, 1), rad - 1, pal->opaque_panel,
			pal->opaque_panel);
	lp_rfill(s, &f);
}

void	luna_menu_panel(t_surface *s, t_rect r)
{
	if (!lp_ok(s))
		return ;
	panel_paint(s, r, LCT_RAD_MENU);
}

void	luna_menu_item(t_surface *s, t_rect r, t_lunastate st)
{
	t_lrfill	f;
	t_color		c;

	if (!lp_ok(s))
		return ;
	if (st != LS_HOT && st != LS_PRESSED && st != LS_FOCUSED)
		return ;
	c = lp_pal()->sel_bg;
	if (st == LS_PRESSED)
		c = lp_pal()->sel_ring;
	f = lp_rmake(r, LCT_RAD_ITEM, c, c);
	lp_rfill(s, &f);
	if (st != LS_FOCUSED)
		return ;
	f.top = lp_pal()->sel_ring;
	lp_rring(s, &f, 1);
}

void	luna_tooltip(t_surface *s, t_rect r)
{
	if (!lp_ok(s))
		return ;
	panel_paint(s, r, LCT_RAD_ITEM);
}
