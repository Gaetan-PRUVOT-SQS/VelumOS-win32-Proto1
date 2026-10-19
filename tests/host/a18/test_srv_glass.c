#include <stdlib.h>
#include "harness.h"
#include "help.h"
#include "velum/luna_launcher.h"

static t_wsrv	g_s;

static int	start_failing(int64_t count)
{
	int	r;

	fk_reset();
	fk_set_display(640, 480);
	g_fk.fail_alloc = count;
	r = ws_open(&g_s);
	g_fk.pump = hs_pump;
	g_fk.pump_arg = &g_s;
	return (r);
}

static void	verre_alloue(void)
{
	uint64_t	need;

	h_eq_i64("ouverture", hs_start(&g_s, 640, 480), 0);
	need = (uint64_t)luna_glass_need(rect_make(0, 0, LUNA_LAUNCHER_WIDTH,
				LUNA_LAUNCHER_HEIGHT)) * 4;
	h_true(need >= (uint64_t)luna_glass_need(rect_make(0, 0, 640, 80)) * 4,
		"le lanceur domine la barre de 640 px");
	h_true(g_s.glass != NULL, "tampon de verre alloue");
	h_eq_u64("taille luna_glass_need(panneau du lanceur) * 4", g_s.glass_bytes, need);
	hs_stop(&g_s);
	h_true(g_s.glass == NULL && g_s.glass_bytes == 0, "tampon libere");
	h_true(g_s.back.px == NULL, "fond libere");
}

static void	verre_refuse(void)
{
	t_handle	a;
	t_surface	ref;
	uint32_t	*px;

	h_eq_i64("ouverture malgre l'echec", start_failing(1), 0);
	h_true(g_s.glass == NULL && g_s.glass_bytes == 0, "pas de tampon");
	h_true(g_s.back.px != NULL, "fond conserve");
	a = hr_client(&g_s);
	h_true(hs_create(&g_s, a, rect_make(40, 40, 300, 200), WS_DEFAULT) != 0,
		"fenetre creee sans verre");
	ws_mark(&g_s, g_s.dirty.screen);
	ws_frame(&g_s);
	px = calloc(640 * 480, 4);
	gfx_surface_init(&ref, px, 640, 480);
	h_eq_i64("repli opaque egal a la composition complete",
		hx_compare(&g_s, &ref), 0);
	free(px);
	fk_close(a);
	hs_stop(&g_s);
}

static void	fond_refuse(void)
{
	h_eq_i64("echec du fond", start_failing(0), E_NOMEM);
	h_true(g_s.back.px == NULL && g_s.glass == NULL, "rien alloue");
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_glass");
	h_run("tampon de verre alloue et libere", verre_alloue);
	h_run("echec d'allocation du verre, repli opaque", verre_refuse);
	h_run("echec d'allocation du fond", fond_refuse);
	return (h_end());
}
