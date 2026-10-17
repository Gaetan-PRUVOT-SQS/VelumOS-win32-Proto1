#include "harness.h"
#include "help.h"
#include "velum/theme.h"

static t_wtable	g_t;

static void	clamp_and_min(void)
{
	int32_t	w;
	int32_t	h;

	h_eq_i64("sous le min", wg_clamp(-5, 0, 10), 0);
	h_eq_i64("au-dessus du max", wg_clamp(15, 0, 10), 10);
	h_eq_i64("dans l'intervalle", wg_clamp(7, 0, 10), 7);
	h_eq_i64("borne basse prioritaire", wg_clamp(5, 10, 0), 10);
	h_eq_i64("extremes", wg_clamp(INT32_MIN, -3, 3), -3);
	wg_min_size(WS_DEFAULT, &w, &h);
	h_eq_i64("largeur mini decoree", w, WS_MINTRACK_W);
	h_eq_i64("hauteur mini decoree", h, VT_CAPTION_H);
	wg_min_size(WS_POPUP, &w, &h);
	h_true(w == 1 && h == 1, "popup sans minimum");
	wg_min_size(WS_DEFAULT | WS_DESKTOP, &w, &h);
	h_true(w == 1 && h == 1, "bureau sans cadre");
	h_true(wh_decorated(WS_CAPTION) && !wh_decorated(WS_SYSMENU),
		"decoration suit WS_CAPTION");
	h_true(!wh_decorated(WS_DEFAULT | WS_APPBAR)
		&& !wh_decorated(WS_DEFAULT | WS_FULLSCREEN)
		&& !wh_decorated(WS_DEFAULT | WS_POPUP), "styles sans cadre");
}

static void	normalize_size(void)
{
	t_rect	r;

	hz_screen(&g_t);
	r = wg_normalize(&g_t, WS_DESKTOP, rect_make(5, 5, 10, 10));
	h_true(r.x == 0 && r.w == 1024 && r.h == 768, "bureau plein ecran");
	r = wg_normalize(&g_t, WS_FULLSCREEN, rect_make(5, 5, 10, 10));
	h_true(r.w == 1024 && r.h == 768, "plein ecran");
	r = wg_normalize(&g_t, WS_DEFAULT, rect_make(100, 100, 1, 1));
	h_true(r.w == WS_MINTRACK_W && r.h == VT_CAPTION_H,
		"taille mini imposee");
	r = wg_normalize(&g_t, WS_DEFAULT, rect_make(0, 0, 5000, 5000));
	h_true(r.w == 1024 && r.h == 768, "taille plafonnee a l'ecran");
}

static void	normalize_pos(void)
{
	t_rect	r;

	hz_screen(&g_t);
	r = wg_normalize(&g_t, WS_DEFAULT, rect_make(-9000, -50, 300, 200));
	h_true(r.x == -(300 - WS_DRAG_KEEP) && r.y == 0, "gauche et haut");
	r = wg_normalize(&g_t, WS_DEFAULT, rect_make(9000, 9000, 300, 200));
	h_true(r.x == 1024 - WS_DRAG_KEEP && r.y == 738 - VT_CAPTION_H,
		"droite et bas");
	r = wg_normalize(&g_t, WS_DEFAULT, rect_make(1024 - 40,
				738 - VT_CAPTION_H, 300, 200));
	h_true(r.x == 984 && r.y == 738 - VT_CAPTION_H,
		"limite exacte acceptee");
	r = wg_normalize(&g_t, WS_DEFAULT, rect_make(1024 - 39,
				738 - VT_CAPTION_H + 1, 300, 200));
	h_true(r.x == 984 && r.y == 738 - VT_CAPTION_H,
		"limite + 1 ramenee");
	r = wg_normalize(&g_t, WS_POPUP, rect_make(900, 700, 300, 200));
	h_true(r.x == 724 && r.y == 568, "popup entierement visible");
	r = wg_normalize(&g_t, WS_APPBAR, rect_make(0, 738, 1024, 30));
	h_true(r.x == 0 && r.y == 738 && r.h == 30, "barre des taches intacte");
	r = wg_maximized(&g_t);
	h_true(r.h == 738 && r.w == 1024, "agrandi = zone de travail");
}

static void	rect_helpers(void)
{
	h_true(wr_inside(rect_make(0, 0, 10, 10), rect_make(0, 0, 10, 10)),
		"egal inclus");
	h_true(!wr_inside(rect_make(0, 0, 10, 10), rect_make(1, 0, 10, 10)),
		"debord droite");
	h_true(!wr_inside(rect_make(0, 0, 10, 10), rect_make(0, -1, 5, 5)),
		"debord haut");
	h_true(wr_inside(rect_make(INT32_MAX - 5, 0, 5, 5),
			rect_make(INT32_MAX - 5, 0, 5, 5)), "pas de debordement");
	h_eq_i64("decalage", wr_offset(rect_make(1, 2, 3, 4), 10, -5).y, -3);
}

int	main(void)
{
	h_begin("a18/core_geom");
	h_run("clamp et tailles mini", clamp_and_min);
	h_run("normalisation taille", normalize_size);
	h_run("normalisation position", normalize_pos);
	h_run("aides rectangle", rect_helpers);
	return (h_end());
}
