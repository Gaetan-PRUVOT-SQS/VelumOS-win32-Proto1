#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "help.h"
#include "velum/theme.h"

#define SW 320
#define SH 240

static t_wtable	g_t;

static void	random_stack(uint64_t *st)
{
	static const uint32_t	styles[] = {WS_DEFAULT, WS_DEFAULT | WS_TOPMOST,
		WS_CAPTION | WS_SYSMENU, WS_POPUP, WS_DEFAULT | WS_TOOLWINDOW,
		WS_CAPTION, WS_APPBAR, WS_DEFAULT};
	int						i;
	int						slot;

	wt_init(&g_t, rect_make(0, 0, SW, SH));
	slot = hz_add(&g_t, WS_DESKTOP);
	g_t.w[slot].rect = g_t.screen;
	i = 0;
	while (i < 7)
	{
		slot = hz_add(&g_t, styles[hg_below(st, 8)]);
		g_t.w[slot].rect = rect_make((int32_t)hg_below(st, SW) - 40,
				(int32_t)hg_below(st, SH) - 30, 1 + (int32_t)hg_below(st, 260),
				1 + (int32_t)hg_below(st, 200));
		g_t.w[slot].state = hg_below(st, 6);
		if (g_t.w[slot].state > WSTATE_HIDDEN)
			g_t.w[slot].state = WSTATE_NORMAL;
		i++;
	}
	g_t.active = wf_next(&g_t, -1);
}

static void	cross_luna(void)
{
	uint64_t	st;
	int			*owner;
	uint32_t	*code;
	int			k;
	int			bad;

	st = hg_seed("a18/core_hit");
	owner = malloc(sizeof(int) * SW * SH);
	code = malloc(sizeof(uint32_t) * SW * SH);
	k = 0;
	bad = 0;
	while (k++ < 24 && owner != NULL && code != NULL)
	{
		random_stack(&st);
		memset(code, 0, sizeof(uint32_t) * SW * SH);
		memset(owner, 0xff, sizeof(int) * SW * SH);
		hz_paint(&g_t, owner, code);
		bad += hz_cmp(&g_t, owner, code);
	}
	h_eq_i64("points en desaccord avec le modele peint", bad, 0);
	free(owner);
	free(code);
}

static void	client_matches(void)
{
	t_lunawin	lw;
	t_point		p;
	t_rect		cl;
	int			bad;

	hz_screen(&g_t);
	hz_add(&g_t, WS_DEFAULT);
	g_t.w[0].rect = rect_make(50, 60, 300, 200);
	cl = wh_client(&g_t, 0);
	wh_lunawin(&g_t, 0, &lw);
	bad = 0;
	p.y = 59;
	while (++p.y < 260)
	{
		p.x = 49;
		while (++p.x < 350)
			bad += (luna_hit_test(&lw, p) == HT_CLIENT)
				!= rect_contains(rect_make(cl.x + VT_RESIZE_ZONE, cl.y,
						cl.w - 2 * VT_RESIZE_ZONE, cl.h - VT_RESIZE_ZONE), p);
	}
	h_eq_i64("HT_CLIENT = client moins zone de bord", bad, 0);
	h_true(wr_inside(g_t.w[0].rect, cl) && cl.w > 0, "client inclus");
	g_t.w[0].state = WSTATE_MIN;
	h_eq_i64("reduite non touchee", wh_window_at(&g_t, (t_point){60, 70}), -1);
}

int	main(void)
{
	h_begin("a18/core_hit");
	h_run("croisement avec luna_hit_test", cross_luna);
	h_run("zone client coherente", client_matches);
	return (h_end());
}
