#include <string.h>
#include "harness.h"
#include "help.h"
#include "velum/theme.h"

#define W 800
#define H 600
#define SIGMA 10.0
#define DESK_B 152.0
#define SHADOW_B 80.0
#define SHADOW_A 71.0
#define OFFSET_Y 12
#define HALF 20

static t_wtable	g_t;
static uint32_t	g_px[W * H];
static uint32_t	g_inc[W * H];
static uint32_t	g_scratch[4 * W * H];

static double	erf_series(double x)
{
	double	term;
	double	sum;
	int		n;

	term = x;
	sum = x;
	n = 1;
	while (n < 60)
	{
		term = -term * x * x / n;
		sum += term / (2 * n + 1);
		n++;
	}
	return (sum * 1.1283791670955126);
}

static double	coverage(double signed_dist)
{
	return (0.5 * (1.0 - erf_series(signed_dist / (SIGMA * 1.4142135623730951))));
}

static void	paint(uint32_t *px, t_rect d)
{
	t_surface	s;
	t_wcursor	c;

	gfx_surface_init(&s, px, W, H);
	c.pos = (t_point){0, 0};
	c.shape = CUR_ARROW;
	c.visible = false;
	wc_compose(&g_t, &s, c, d);
}

static void	scene(uint32_t style)
{
	wt_init(&g_t, rect_make(0, 0, W, H));
	g_t.work = g_t.screen;
	hz_add(&g_t, style);
	g_t.w[0].rect = rect_make(200, 100, 400, 300);
	g_t.active = 0;
	luna_theme_set(LUNA_THEME_LIGHT);
	paint(g_px, g_t.screen);
}

static double	measured(int x, int y)
{
	double	b;

	b = (double)(g_px[y * W + x] & 0xffu);
	return ((DESK_B - b) / (DESK_B - SHADOW_B));
}

static void	probe(const char *what, int x, int y, double dist)
{
	double	want;
	double	got;
	double	gap;

	want = coverage(dist) * SHADOW_A / 255.0;
	got = measured(x, y);
	gap = got - want;
	if (gap < 0)
		gap = -gap;
	h_true(gap < 0.025, what);
}

static void	profile_bas(void)
{
	int	k;

	scene(WS_DEFAULT);
	k = 0;
	while (k < 34)
	{
		probe("bord bas, milieu", 400, 400 + k, k + 0.5 - OFFSET_Y);
		k++;
	}
}

static void	profile_cotes_haut(void)
{
	int	j;

	scene(WS_DEFAULT);
	j = 0;
	while (j < 22)
	{
		probe("bord gauche, milieu", 199 - j, 250, j + 0.5);
		probe("bord droit, milieu", 600 + j, 250, j + 0.5);
		probe("bord haut, milieu", 400, 99 - j, j + 0.5 + OFFSET_Y);
		j++;
	}
}

static void	hors_marge(void)
{
	int	x;
	int	y;
	int	bad;

	scene(WS_DEFAULT);
	bad = 0;
	y = 0;
	while (y < H)
	{
		x = 0;
		while (x < W)
		{
			if (x < 200 - HALF || x >= 600 + HALF || y < 100 - 8
				|| y >= 400 + 32)
				bad += (g_px[y * W + x] != WS_DESK_COLOR);
			x++;
		}
		y++;
	}
	h_eq_i64("pixels hors de la marge : bureau intact", bad, 0);
}

static void	etendue(void)
{
	t_rect	e;

	scene(WS_DEFAULT);
	e = wsh_extent(&g_t, 0);
	h_eq_i64("etendue x", e.x, 180);
	h_eq_i64("etendue y", e.y, 92);
	h_eq_i64("etendue largeur", e.w, 440);
	h_eq_i64("etendue hauteur", e.h, 340);
	h_true(wsh_has(&g_t, 0), "fenetre a cadre : ombre");
}

static void	sans_ombre(void)
{
	scene(0);
	h_true(!wsh_has(&g_t, 0), "sans cadre : pas d'ombre");
	h_true(wsh_extent(&g_t, 0).w == 400, "sans cadre : etendue = rectangle");
	scene(WS_DEFAULT);
	g_t.w[0].state = WSTATE_MAX;
	h_true(!wsh_has(&g_t, 0), "maximisee : pas d'ombre");
	scene(WS_DEFAULT);
	g_t.w[0].state = WSTATE_MIN;
	h_true(!wsh_has(&g_t, 0), "minimisee : pas d'ombre");
}

static void	sombre(void)
{
	scene(WS_DEFAULT);
	luna_theme_set(LUNA_THEME_DARK);
	paint(g_px, g_t.screen);
	h_true((g_px[(400 + 4) * W + 400] & 0xffu) < 0x60,
		"theme sombre : ombre plus dense");
	luna_theme_set(LUNA_THEME_LIGHT);
}

static void	deplacement(void)
{
	t_rect	old;
	t_rect	d;

	scene(WS_DEFAULT);
	memcpy(g_inc, g_px, sizeof(g_px));
	old = wsh_extent(&g_t, 0);
	g_t.w[0].rect = rect_make(230, 140, 400, 300);
	d = rect_union(old, wsh_extent(&g_t, 0));
	paint(g_inc, wgl_extend(&g_t, d));
	paint(g_px, g_t.screen);
	h_true(memcmp(g_px, g_inc, sizeof(g_px)) == 0,
		"deplacement : incremental = complet avec ombre");
	memcpy(g_inc, g_px, sizeof(g_px));
	g_t.w[0].rect = rect_make(230, 140, 400, 300);
	g_t.w[0].state = WSTATE_MIN;
	d = wsh_extent(&g_t, 0);
	paint(g_inc, wgl_extend(&g_t, d));
	paint(g_px, g_t.screen);
	h_true(memcmp(g_px, g_inc, sizeof(g_px)) == 0,
		"masquage : l'ombre disparait comme au complet");
}

static void	deux_fenetres(void)
{
	scene(WS_DEFAULT);
	hz_add(&g_t, WS_DEFAULT);
	g_t.w[1].rect = rect_make(380, 330, 300, 200);
	paint(g_px, g_t.screen);
	memset(g_inc, 0, sizeof(g_inc));
	paint(g_inc, g_t.screen);
	h_true(memcmp(g_px, g_inc, sizeof(g_px)) == 0, "composition deterministe");
	h_true((g_px[380 * W + 370] & 0xffu) < (g_px[380 * W + 300] & 0xffu),
		"ombre de la fenetre du dessus visible sur celle du dessous");
}

int	main(void)
{
	luna_transparency_set(false);
	luna_glass_scratch(g_scratch, 4 * W * H);
	h_begin("a18/core_shadow");
	h_run("profil du bord bas contre gaussienne sigma 10", profile_bas);
	h_run("profils gauche, droit et haut", profile_cotes_haut);
	h_run("hors marge : bureau intact", hors_marge);
	h_run("etendue de l'ombre", etendue);
	h_run("fenetres sans ombre", sans_ombre);
	h_run("theme sombre", sombre);
	h_run("deplacement : incremental = complet", deplacement);
	h_run("deux fenetres", deux_fenetres);
	return (h_end());
}
