#include <stdio.h>
#include <stdlib.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	start(t_handle *cl)
{
	hs_start(&g_s, 320, 240);
	cl[0] = hr_client(&g_s);
	cl[1] = hr_client(&g_s);
	cl[2] = hr_client(&g_s);
	hr_value(&g_s, cl[0], WMC_SUBSCRIBE, (uint32_t [2]){0, 1});
	hs_create(&g_s, cl[0], rect_make(0, 0, 320, 240), WS_DESKTOP);
	hs_create(&g_s, cl[0], rect_make(0, 214, 320, 26), WS_APPBAR);
}

static int	step_checks(const t_handle *cl, t_surface *ref, int i,
	int *bad_inv)
{
	int	bad;

	hr_drain(cl[0], 0, NULL);
	hr_drain(cl[1], 0, NULL);
	hr_drain(cl[2], 0, NULL);
	g_fk.now += WS_FRAME_NS;
	ws_step(&g_s);
	if (wz_check(&g_s.t) != 0)
		(*bad_inv)++;
	bad = hx_compare(&g_s, ref);
	if (bad != 0)
		fprintf(stderr, "operation %d : %d pixels differents\n", i, bad);
	return (bad != 0);
}

static void	finish(t_surface *ref)
{
	h_eq_i64("aucune violation", g_s.violations, 0);
	free(ref->px);
	hs_stop(&g_s);
	h_eq_i64("rien ne fuit", fk_live(), 0);
}

static void	random_ops(void)
{
	t_handle	cl[3];
	uint64_t	st;
	t_surface	ref;
	int			bad[2];
	int			i;

	st = hg_seed("a18/srv_random");
	start(cl);
	gfx_surface_init(&ref, calloc(320 * 240, 4), 320, 240);
	bad[0] = 0;
	bad[1] = 0;
	i = -1;
	while (++i < 1000)
	{
		if (hg_below(&st, 2))
			hx_req(&g_s, cl, &st);
		else
			hx_input(&g_s, &st, hg_below(&st, 5));
		bad[0] += step_checks(cl, &ref, i, &bad[1]);
	}
	h_eq_i64("invariants Z et focus apres chaque pas", bad[1], 0);
	h_eq_i64("incremental = complet apres chaque pas", bad[0], 0);
	finish(&ref);
}

int	main(void)
{
	h_begin("a18/srv_random");
	h_run("1000 operations aleatoires", random_ops);
	return (h_end());
}
