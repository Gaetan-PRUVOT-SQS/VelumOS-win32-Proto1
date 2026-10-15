#include "velum/theme.h"
#include "luna_int.h"

#define GK_PILL_MAX 4096

static const t_gkind	g_kind[LUNA_GLASS_KINDS] = {
{{VT_GLASS_DOCK, VT_GLASS_DOCK_D}, VT_BLUR_DOCK, VT_R_DOCK, false, false},
{{VT_GLASS_PANEL, VT_GLASS_PANEL_D}, VT_BLUR_PANEL, VT_R_PANEL, true, false},
{{VT_GLASS_PANEL, VT_GLASS_PANEL_D}, VT_BLUR_PANEL, VT_R_WINDOW_LG, true,
	false},
{{VT_GLASS_CHIP, VT_GLASS_CHIP_D}, VT_BLUR_WINDOW, VT_R_PANEL, true, false},
{{VT_GLASS_CHIP, VT_GLASS_CHIP_D}, VT_BLUR_CHIP, 0, false, true},
{{VT_GLASS_FRAME, VT_GLASS_FRAME_D}, VT_BLUR_PANEL, VT_R_WINDOW_LG, false,
	false},
{{VT_GLASS_WINDOW, VT_GLASS_WINDOW_D}, VT_BLUR_WINDOW, VT_R_WINDOW, false,
	false},
{{VT_GLASS_CONSOLE, VT_GLASS_CONSOLE}, VT_BLUR_WINDOW, VT_R_WINDOW, false,
	false},
{{0x80ffffff, 0x99282e3e}, VT_BLUR_CHIP, VT_R_DOCK, false, false}};

size_t	luna_glass_need(t_rect r)
{
	r = lp_clean(r);
	return (2 * (size_t)((r.w + LX_SCALE - 1) / LX_SCALE)
		* (size_t)((r.h + LX_SCALE - 1) / LX_SCALE));
}

static void	kind_fill(t_lglass *g, const t_gkind *k, t_cpal p, bool dark)
{
	g->fill = k->fill[dark];
	g->blur = k->blur;
	g->sat = p->sat;
	if (k->panel)
		g->sat = p->sat_panel;
	g->opaque = p->opaque_panel;
	g->spec = p->spec;
	g->rim[0] = p->rim_top;
	g->rim[1] = p->rim_in;
	g->rim[2] = p->rim_bottom;
}

bool	lp_glass_make(t_lglass *g, t_rect r, t_glasskind kind)
{
	const t_gkind	*k;

	if ((int32_t)kind < 0 || kind >= LUNA_GLASS_KINDS)
		return (false);
	k = &g_kind[kind];
	g->r = r;
	g->hole = lp_rect(0, 0, 0, 0);
	g->rad_top = k->rad;
	if (k->pill)
		g->rad_top = lp_min(r.h / 2, GK_PILL_MAX);
	g->rad_bot = g->rad_top;
	kind_fill(g, k, lp_pal(), luna_theme_get() == LUNA_THEME_DARK);
	if (kind == LUNA_GLASS_CONSOLE)
		g->opaque = 0xff0e111a;
	return (true);
}

void	luna_glass(t_surface *s, t_rect r, t_glasskind kind)
{
	t_lglass	g;

	r = lp_clean(r);
	if (!lp_ok(s) || !lp_glass_make(&g, r, kind))
		return ;
	lp_glass(s, &g);
}
