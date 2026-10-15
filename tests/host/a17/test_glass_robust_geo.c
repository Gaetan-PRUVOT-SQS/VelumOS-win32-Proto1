#include <stdint.h>
#include "harness.h"
#include "lt.h"
#include "luna_int.h"

#define BACK 0xff3366ccu
#define SCR 4096
#define NREC 7
#define NBLUR 5

static uint32_t			g_scr[SCR];

static const int32_t	g_r[NREC][4] = {
{0, 10, 50, INT32_MAX}, {10, 0, INT32_MAX, 40}, {0, 0, INT32_MAX, INT32_MAX},
{-5, -5, INT32_MAX, INT32_MAX}, {INT32_MIN, 0, INT32_MAX, 10},
{20, 20, 16777217, 16777217}, {-16777216, 5, 16777217, 30}};

static const int32_t	g_b[NBLUR] = {0, INT32_MIN, INT32_MAX, 1 << 30, 40000};

static void	paint(t_lt *t, const t_lglass *g, uint64_t *hash)
{
	lt_clear(t, BACK);
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	luna_glass_scratch(g_scr, SCR);
	lp_glass(&t->s, g);
	*hash = lt_hash(&t->s);
}

static void	one_rect(t_lt *t, const int32_t *r)
{
	t_lglass	g;
	t_lglass	c;
	uint64_t	got;
	uint64_t	want;

	lp_glass_make(&g, lp_rect(r[0], r[1], r[2], r[3]), LUNA_GLASS_PANEL);
	c = g;
	c.r = lp_clean(g.r);
	paint(t, &g, &got);
	h_true(lt_guard_ok(t), "gardes intactes, rectangle hostile");
	paint(t, &c, &want);
	h_eq_u64("rectangle hostile egal au rectangle nettoye", got, want);
}

static void	rect_equals_clean(void)
{
	t_lt	t;
	int32_t	i;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	i = -1;
	while (++i < NREC)
		one_rect(&t, g_r[i]);
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

static void	blur_bounded(void)
{
	t_lt		t;
	t_lglass	g;
	uint64_t	got;
	uint64_t	want;
	int32_t		i;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	lp_glass_make(&g, lp_rect(10, 10, 60, 30), LUNA_GLASS_PANEL);
	g.blur = 16384;
	paint(&t, &g, &want);
	i = -1;
	while (++i < NBLUR)
	{
		g.blur = g_b[i];
		paint(&t, &g, &got);
		h_true(lt_guard_ok(&t), "gardes intactes, flou hostile");
		if (g_b[i] > 16384)
			h_eq_u64("flou au dela du plafond egal au plafond", got, want);
	}
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_robuste_geometrie");
	h_run("t_lglass direct, rectangle hostile, modele lp_clean",
		rect_equals_clean);
	h_run("t_lglass direct, rayon de flou borne", blur_bounded);
	return (h_end());
}
