#include <stdint.h>
#include "harness.h"
#include "lt.h"
#include "luna_int.h"

#define BACK 0xff3366ccu
#define SCR 4096
#define NLG 7

static uint32_t			g_scr[SCR];

static const int32_t	g_l[NLG][5] = {
{-7, -7, 0, 0, 0}, {INT32_MAX, INT32_MAX, 1, 0, 0},
{INT32_MIN, 0, -5, 0, 0}, {0, INT32_MIN, 8, 0, 0}, {1000, 3, 16, 0, 0},
{4, 4, 16, 1, 0}, {4, 4, 16, 0, 1}};

static void	lglass_set(t_lglass *g, const int32_t *c)
{
	lp_glass_make(g, lp_rect(10, 10, 60, 30), LUNA_GLASS_PANEL);
	g->rad_top = c[0];
	g->rad_bot = c[1];
	g->blur = c[2];
	if (c[3] != 0)
		g->hole = lp_rect(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX);
	if (c[4] != 0)
		g->r = lp_rect(INT32_MAX - 5, 5, 100, 10);
}

static void	lglass_hostile(void)
{
	t_lt		t;
	t_lglass	g;
	uint64_t	ref;
	int32_t		i;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	luna_glass_scratch(g_scr, SCR);
	i = -1;
	while (++i < NLG)
	{
		lt_clear(&t, BACK);
		ref = lt_hash(&t.s);
		lglass_set(&g, g_l[i]);
		lp_glass(&t.s, &g);
		h_true(lt_guard_ok(&t), "gardes intactes");
		if (g_l[i][4] != 0)
			h_true(lt_hash(&t.s) == ref, "rectangle lointain, rien de peint");
	}
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_robuste_interne");
	h_run("t_lglass interne hostile, rayons, trou, rectangle", lglass_hostile);
	return (h_end());
}
