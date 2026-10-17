#include "velum/theme.h"
#include "launcher_int.h"

static void	tile_back(t_surface *s, t_rect r, int32_t st)
{
	if (st == 0)
		return ;
	lp_launch_fill(s, r, VT_R_TILE, lp_launch_tone(LL_HOT_L, LL_HOT_D));
	lp_fill(s, lp_rect(r.x + VT_R_TILE, r.y, r.w - 2 * VT_R_TILE, 1),
		lp_launch_tone(LL_HOT_RIM_L, LL_HOT_RIM_D));
	if (st & LL_ST_FOCUS)
		lp_launch_focus(s, r, VT_R_TILE);
}

static void	tile_label(t_surface *s, t_rect r, const char *text)
{
	t_ltext	t;

	if (text == NULL)
		return ;
	t.font = FONT_UI_BOLD;
	t.color = lp_pal()->ink;
	t.shadow = 0;
	t.text = text;
	t.box = lp_rect(r.x + 4, r.y + LL_TILE_TOP + LL_ICON + 8, r.w - 8,
			LL_LABEL_H);
	t.align = LA_CENTER;
	lp_text(s, &t);
}

void	lp_launch_tile(t_surface *s, t_rect r, const t_launchertile *t,
			int32_t st)
{
	if (t == NULL)
		return ;
	tile_back(s, r, st);
	luna_icon(s, lp_pt(r.x + (r.w - LL_ICON) / 2, r.y + LL_TILE_TOP), t->icon,
		LL_ICON);
	tile_label(s, r, t->label);
}
