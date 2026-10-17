#include <stdio.h>
#include "harness.h"
#include "lt_bbox.h"

static const t_rect	g_bar = {20, 12, 180, 8};

static t_lt	g_t;
static t_lt	g_u;

static uint64_t	bar_hash(uint32_t pct, uint32_t tick)
{
	lt_clear(&g_u, LC_WHITE);
	lp_boot_bar(&g_u.s, &g_bar, pct, tick);
	return (lt_hash(&g_u.s));
}

static void	bar_fill(void)
{
	static const uint32_t	pct[4] = {10, 25, 50, 75};
	t_color					track;
	int32_t					edge;
	char					name[80];
	int						i;

	lt_clear(&g_t, LC_WHITE);
	lp_boot_bar(&g_t.s, &g_bar, 0, 0);
	track = gfx_get(&g_t.s, lp_pt(g_bar.x + g_bar.w / 2, g_bar.y + 4));
	i = 0;
	while (i < 4)
	{
		lt_clear(&g_t, LC_WHITE);
		lp_boot_bar(&g_t.s, &g_bar, pct[i], 99);
		edge = g_bar.x + g_bar.w * (int32_t)pct[i] / 100;
		snprintf(name, sizeof(name), "avancement %u : rempli avant le bord",
			(unsigned)pct[i]);
		h_true(gfx_get(&g_t.s, lp_pt(edge - 4, g_bar.y + 4)) != track, name);
		snprintf(name, sizeof(name), "avancement %u : piste apres le bord",
			(unsigned)pct[i]);
		h_true(gfx_get(&g_t.s, lp_pt(edge + 4, g_bar.y + 4)) == track, name);
		i++;
	}
}

static void	bar_limits(void)
{
	h_eq_u64("pct 100 et pct 101 identiques", bar_hash(101, 3),
		bar_hash(100, 3));
	h_eq_u64("pct enorme borne a 100", bar_hash(4000000000u, 3),
		bar_hash(100, 3));
	h_eq_u64("tick sans effet quand le pourcentage est connu",
		bar_hash(40, 0), bar_hash(40, 12345));
	h_true(bar_hash(0, 0) != bar_hash(0, 10), "indetermine : le segment avance");
	h_true(bar_hash(100, 0) != bar_hash(0, 0), "100 % differe de la piste");
}

static void	bar_odd(int32_t x, int32_t y, int32_t w, int32_t h)
{
	t_rect	r;

	r = lp_rect(x, y, w, h);
	lp_boot_bar(&g_t.s, &r, 50, 0);
}

static void	bar_bounds(void)
{
	static const uint32_t	tick[5] = {0, 1, 37, 4000000000u, 4294967295u};
	t_rect					ink;
	int						i;

	i = 0;
	while (i < 5)
	{
		lt_clear(&g_t, LC_WHITE);
		lp_boot_bar(&g_t.s, &g_bar, 0, tick[i]);
		ink = ltb_bbox(&g_t.s, NULL, LC_WHITE);
		h_true(ink.w > 0 && ltb_inside(ink, g_bar),
			"segment indetermine dans le rectangle de la barre");
		i++;
	}
	lt_clear(&g_t, LC_WHITE);
	lp_boot_bar(&g_t.s, NULL, 50, 0);
	bar_odd(0, 0, 1, 8);
	bar_odd(0, 0, 40, 1);
	bar_odd(-30, -3, 50, 8);
	bar_odd(690, 250, 100, 30);
	h_true(lt_guard_ok(&g_t), "rectangles hors cadre sans debordement");
}

int	main(void)
{
	if (lt_open(&g_t, 700, 260) != 0 || lt_open(&g_u, 700, 260) != 0)
		return (1);
	luna_theme_set(LUNA_THEME_LIGHT);
	h_begin("a17/boot_bar");
	h_run("P8 barre : bord de remplissage par pourcentage", bar_fill);
	h_run("P8 barre : valeurs limites de pct et de tick", bar_limits);
	h_run("P8 barre : segment borne, rectangles degeneres", bar_bounds);
	lt_close(&g_u);
	lt_close(&g_t);
	return (h_end());
}
