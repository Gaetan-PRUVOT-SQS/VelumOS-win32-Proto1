#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	activation_order(void)
{
	t_handle	c;
	uint32_t	a;
	uint32_t	b;
	t_wmarg		m;

	hs_start(&g_s, 320, 240);
	c = hr_client(&g_s);
	a = hs_create(&g_s, c, rect_make(10, 10, 150, 100), WS_DEFAULT);
	b = hs_create(&g_s, c, rect_make(100, 80, 150, 100), WS_DEFAULT);
	h_eq_i64("derniere creee active", g_s.t.active, hs_slot(&g_s, b));
	h_eq_i64("en haut", g_s.t.z[g_s.t.nz - 1], hs_slot(&g_s, b));
	hs_click(&g_s, 40, 60);
	h_eq_i64("clic active A", g_s.t.active, hs_slot(&g_s, a));
	h_eq_i64("A remontee", g_s.t.z[g_s.t.nz - 1], hs_slot(&g_s, a));
	hr_arg(&m, WMC_SET_STATE, a, WSTATE_MIN);
	hr_send(&g_s, c, &m);
	h_eq_i64("reduction rend la main a B", g_s.t.active, hs_slot(&g_s, b));
	hr_arg(&m, WMC_ACTIVATE, a, 0);
	m.h.size = sizeof(t_wmhdr);
	hr_send(&g_s, c, &m);
	h_true(g_s.t.w[hs_slot(&g_s, a)].state == WSTATE_NORMAL
		&& g_s.t.active == hs_slot(&g_s, a), "activer restaure");
	h_eq_i64("invariants", wz_check(&g_s.t), 0);
	hs_stop(&g_s);
}

static void	special_styles(void)
{
	t_handle	c;
	uint32_t	a;
	uint32_t	t;
	uint32_t	p;

	hs_start(&g_s, 320, 240);
	c = hr_client(&g_s);
	t = hs_create(&g_s, c, rect_make(0, 0, 150, 100), WS_DEFAULT
			| WS_TOPMOST);
	a = hs_create(&g_s, c, rect_make(20, 20, 150, 100), WS_DEFAULT);
	h_eq_i64("topmost reste au-dessus", g_s.t.z[g_s.t.nz - 1],
		hs_slot(&g_s, t));
	p = hs_create(&g_s, c, rect_make(200, 150, 80, 60), WS_POPUP
			| WS_TOPMOST | WS_NOACTIVATE);
	h_eq_i64("popup sans activation", g_s.t.active, hs_slot(&g_s, a));
	hr_drain(c, 0, NULL);
	hs_click(&g_s, 220, 170);
	h_eq_i64("clic popup sans activation", g_s.t.active, hs_slot(&g_s, a));
	h_eq_i64("popup recoit le clic", hr_drain(c, WMS_MOUSE, NULL), 4);
	hs_create(&g_s, c, rect_make(0, 210, 320, 30), WS_APPBAR);
	h_eq_i64("barre non activee", g_s.t.active, hs_slot(&g_s, a));
	h_eq_i64("zone de travail reduite", g_s.t.work.h, 210);
	(void)p;
	h_eq_i64("invariants", wz_check(&g_s.t), 0);
	hs_stop(&g_s);
}

static void	activate_permission(void)
{
	t_handle	shell;
	t_handle	app;
	uint32_t	w[2];
	t_wmarg		m;

	hs_start(&g_s, 320, 240);
	shell = hr_client(&g_s);
	app = hr_client(&g_s);
	hr_arg(&m, WMC_SUBSCRIBE, 0, 1);
	hr_send(&g_s, shell, &m);
	w[0] = hs_create(&g_s, app, rect_make(10, 10, 150, 100), WS_DEFAULT);
	w[1] = hs_create(&g_s, shell, rect_make(50, 50, 150, 100), WS_DEFAULT);
	hr_activate(&g_s, app, w[0]);
	h_eq_i64("vol de focus refuse", hr_error(app), E_PERM);
	h_eq_i64("focus garde", g_s.t.active, hs_slot(&g_s, w[1]));
	hr_activate(&g_s, shell, w[0]);
	h_eq_i64("le shell active une autre appli", g_s.t.active,
		hs_slot(&g_s, w[0]));
	h_true(hr_drain(shell, WMS_WIN_UPD, NULL) >= 2, "abonne prevenu");
	h_eq_i64("pas de violation", g_s.violations, 0);
	hs_stop(&g_s);
}

static void	destroy_active(void)
{
	t_handle	c;
	uint32_t	a;
	uint32_t	b;

	hs_start(&g_s, 320, 240);
	c = hr_client(&g_s);
	a = hs_create(&g_s, c, rect_make(10, 10, 150, 100), WS_DEFAULT);
	b = hs_create(&g_s, c, rect_make(60, 60, 150, 100), WS_DEFAULT);
	hr_destroy(&g_s, c, b);
	h_eq_i64("la suivante devient active", g_s.t.active, hs_slot(&g_s, a));
	hr_destroy(&g_s, c, a);
	h_eq_i64("plus aucune active", g_s.t.active, -1);
	h_eq_i64("invariants", wz_check(&g_s.t), 0);
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_focus");
	h_run("ordre d'activation", activation_order);
	h_run("styles particuliers", special_styles);
	h_run("droit d'activer", activate_permission);
	h_run("destruction de l'active", destroy_active);
	return (h_end());
}
