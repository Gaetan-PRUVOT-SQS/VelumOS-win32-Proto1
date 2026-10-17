#include "harness.h"
#include "help.h"
#include "velum/theme.h"

static t_wtable	g_t;

static bool	same(t_rect a, t_rect b)
{
	return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

static void	scene(uint32_t style_b)
{
	wt_init(&g_t, rect_make(0, 0, 1024, 768));
	g_t.work = g_t.screen;
	hz_add(&g_t, WS_DEFAULT);
	hz_add(&g_t, style_b);
	g_t.w[0].rect = rect_make(20, 20, 200, 150);
	g_t.w[1].rect = rect_make(400, 200, 200, 150);
}

static void	extension(void)
{
	t_rect	band;

	scene(WS_DEFAULT);
	band = rect_make(20, 20, 200, VT_CAPTION_H);
	h_true(same(wgl_extend(&g_t, rect_make(30, 25, 10, 10)), band),
		"classe dans la bande : bande entiere");
	h_true(same(wgl_extend(&g_t, rect_make(150, 60, 100, 40)),
			rect_make(20, 20, 230, 80)), "chevauche le bord : union");
	h_true(same(wgl_extend(&g_t, rect_make(50, 100, 10, 10)),
			rect_make(50, 100, 10, 10)), "client seul : inchange");
	h_true(same(wgl_extend(&g_t, rect_make(-10, -10, 40, 40)),
			rect_make(0, 0, 220, 70)), "debord ecran : borne puis union");
	h_true(same(wgl_extend(&g_t, g_t.w[0].rect), g_t.w[0].rect),
		"fenetre entiere : inchange");
	h_eq_i64("bande de fenetre invalide", wgl_bands(&g_t, -1, &band), 0);
	h_eq_i64("bande hors table", wgl_bands(&g_t, WS_WIN_MAX, &band), 0);
}

static void	exclusions(void)
{
	t_rect	r;

	scene(0);
	r = rect_make(410, 205, 10, 10);
	h_true(same(wgl_extend(&g_t, r), r), "sans cadre : pas de bande");
	h_eq_i64("sans cadre : zero bande", wgl_bands(&g_t, 1, &r), 0);
	scene(WS_DEFAULT);
	g_t.w[0].state = WSTATE_MIN;
	r = rect_make(30, 25, 10, 10);
	h_true(same(wgl_extend(&g_t, r), r), "reduite : pas de bande");
	g_t.w[0].state = WSTATE_HIDDEN;
	h_true(same(wgl_extend(&g_t, r), r), "masquee : pas de bande");
	h_true(rect_empty(wgl_extend(&g_t, rect_make(0, 0, 0, 0))),
		"rectangle vide : vide");
	h_true(rect_empty(wgl_extend(&g_t, rect_make(-100, -100, 50, 50))),
		"hors ecran : vide");
}

int	main(void)
{
	h_begin("a18/core_glass");
	h_run("extension : partition dans, bord, client, debord", extension);
	h_run("exclusions : sans cadre, reduite, masquee, vide", exclusions);
	return (h_end());
}
