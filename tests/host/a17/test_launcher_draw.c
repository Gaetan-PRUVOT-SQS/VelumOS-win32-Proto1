#include "harness.h"
#include "velum/theme.h"
#include "velum/luna_launcher.h"
#include "lt.h"
#include "luna_int.h"

static const t_launchertile	g_tiles[] = {
{"Bonjour", ICON_PROGRAM}, {"Ex\xc3\xa9" "cuter...", ICON_RUN}};

static t_lt					g_t;

static void	draw_guard(void)
{
	t_launcher	l;

	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = 2;
	l.hot = 1;
	l.focus = LUNA_LAUNCHER_POWER;
	lt_clear(&g_t, LC_BLACK);
	luna_launcher(&g_t.s, &l, lp_rect(-100, -50, 540, 289));
	luna_launcher(&g_t.s, &l, lp_rect(500, 400, 540, 289));
	luna_launcher(&g_t.s, &l, lp_rect(12, 150, 540, 289));
	h_true(lt_guard_ok(&g_t), "aucune ecriture hors de la surface");
}

static void	draw_small(void)
{
	t_launcher	l;
	uint64_t	before;

	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = 2;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_NONE;
	lt_clear(&g_t, LC_BLACK);
	before = lt_hash(&g_t.s);
	luna_launcher(&g_t.s, &l, lp_rect(12, 150, 100, 100));
	luna_launcher(&g_t.s, &l, lp_rect(12, 150, -540, -289));
	h_true(lt_hash(&g_t.s) == before, "panneau trop petit : rien dessine");
}

static void	draw_focus(void)
{
	t_launcher	l;
	int32_t		y;
	int32_t		x;

	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = 2;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = 0;
	lt_clear(&g_t, LC_BLACK);
	luna_launcher(&g_t.s, &l, lp_rect(12, 150, 540, 289));
	x = 30 - 4;
	y = 150 + 18 + 48 + 18 + 16 + 10 + 48;
	h_true(g_t.s.px[y * g_t.s.stride + x - 1] != VT_ACCENT, "hors anneau");
	h_true(g_t.s.px[y * g_t.s.stride + x] == VT_ACCENT, "anneau, pixel 1");
	h_true(g_t.s.px[y * g_t.s.stride + x + 1] == VT_ACCENT, "anneau, pixel 2");
	h_true(g_t.s.px[y * g_t.s.stride + x + 2] != VT_ACCENT, "ecart de 2 px");
	h_true(g_t.s.px[y * g_t.s.stride + x + 3] != VT_ACCENT, "ecart, pixel 2");
	h_true(g_t.s.px[y * g_t.s.stride + x + 4] != VT_ACCENT, "bord de tuile");
	l.focus = LUNA_LAUNCHER_NONE;
	lt_clear(&g_t, LC_BLACK);
	luna_launcher(&g_t.s, &l, lp_rect(12, 150, 540, 289));
	h_true(g_t.s.px[y * g_t.s.stride + x] != VT_ACCENT, "sans focus");
}

int	main(void)
{
	if (lt_open(&g_t, 640, 480) != 0)
		return (1);
	h_begin("a17/lanceur_dessin");
	h_run("bornes : hors surface, zones de garde", draw_guard);
	h_run("panneau trop petit ou inverse", draw_small);
	h_run("anneau de focus accent de 2 px, decale de 2 px", draw_focus);
	lt_close(&g_t);
	return (h_end());
}
