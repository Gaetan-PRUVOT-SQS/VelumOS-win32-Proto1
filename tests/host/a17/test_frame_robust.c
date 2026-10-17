#include <stdint.h>
#include <string.h>
#include "harness.h"
#include "lt_fuzz.h"

#define BACK 0xff3366ccu
#define SW 100
#define SH 60
#define SCR 4096
#define NWIN 17
#define NVAR 96
#define LONG_TITLE 4000

static uint32_t			g_scr[SCR];
static char				g_long[LONG_TITLE + 1];

static const int32_t	g_w[NWIN][4] = {
{10, 10, 80, 40}, {0, 0, 100, 60}, {-50, -50, 80, 40}, {80, 40, 100, 100},
{10, 10, 0, 0}, {10, 10, -5, 40}, {10, 10, 1, 1},
{INT32_MAX, INT32_MAX, 100, 100}, {INT32_MIN, INT32_MIN, 100, 100},
{-10, -10, INT32_MAX, INT32_MAX}, {0, 0, 1048577, 1048577},
{268435456, 0, 50, 50}, {-268435457, 0, 50, 50}, {-100, -100, 150, 160},
{268435256, 0, 300, 60}, {268435456, 0, 1000, 60}, {0, 268435256, 100, 300}};

static const uint32_t	g_st[4] = {0, WS_CAPTION | WS_SYSMENU,
	WS_CAPTION | WS_SIZEBOX, WS_CAPTION | WS_TOOLWINDOW};

static t_rect	zone_of(t_rect r)
{
	return (ltf_zone(r, lp_rect(0, 0, SW, SH), SW, SH));
}

static void	one_variant(t_lt *t, const int32_t *r, int32_t v)
{
	t_lunawin	w;
	t_lgeo		g;
	t_rect		out[2];

	w = (t_lunawin){lp_rect(r[0], r[1], r[2], r[3]), g_st[v % 4], NULL, 0,
		(v / 8) % 2 != 0, (v / 4) % 2 != 0, 0, 0};
	if (v / 16 % 3 == 1)
		w.title = "robuste";
	if (v / 16 % 3 == 2)
		w.title = g_long;
	luna_glass_scratch(g_scr, SCR * (size_t)(v / 48));
	lt_clear(t, BACK);
	luna_window_frame(&t->s, &w);
	h_true(ltf_surface_ok(t, zone_of(w.outer), BACK),
		"cadre : rien hors du rectangle de la fenetre");
	lt_clear(t, BACK);
	luna_client_band_paint(&t->s, &w, NULL, 0xff202020u);
	lg_make(&w, &g);
	h_true(ltf_surface_ok(t, zone_of(g.client), BACK),
		"bande : rien hors du client");
	h_true(luna_window_glass_bands(&w, out) <= 2, "au plus deux bandes");
}

static void	hostile_windows(void)
{
	t_lt	t;
	int32_t	i;
	int32_t	v;

	memset(g_long, 'W', LONG_TITLE);
	h_true(lt_open(&t, SW, SH) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	i = -1;
	while (++i < NWIN)
	{
		v = -1;
		while (++v < NVAR)
			one_variant(&t, g_w[i], v);
	}
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/cadre_robuste");
	h_run("cadre et bande, fenetres hostiles, styles et titres",
		hostile_windows);
	return (h_end());
}
