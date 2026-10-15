#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	keys_to_active(void)
{
	t_handle	a;
	t_handle	b;
	t_wmkey		k;

	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	b = hr_client(&g_s);
	hs_create(&g_s, a, rect_make(10, 10, 150, 100), WS_DEFAULT);
	hs_create(&g_s, b, rect_make(60, 60, 150, 100), WS_DEFAULT);
	hr_drain(a, 0, NULL);
	hs_key(&g_s, 'Q', 0);
	h_eq_i64("touches a l'active", hr_drain(b, WMS_KEY, &k), 2);
	h_true(k.ev.type == INP_KEY_UP && k.ev.code == 'Q', "evenement intact");
	h_eq_i64("rien pour l'arriere-plan", hr_drain(a, WMS_KEY, NULL), 0);
	hs_key(&g_s, VK_F1 + 3, INPM_ALT);
	h_eq_i64("Alt+F4 ferme l'active", hr_drain(b, WMS_CLOSE_REQ, NULL), 1);
	h_eq_i64("Alt+F4 avale", hr_drain(b, WMS_KEY, NULL), 0);
	h_eq_i64("autre client intact", hr_drain(a, WMS_CLOSE_REQ, NULL), 0);
	hs_stop(&g_s);
}

static void	alt_tab(void)
{
	t_handle	c;
	uint32_t	w[3];
	t_wmarg		m;

	hs_start(&g_s, 320, 240);
	c = hr_client(&g_s);
	w[0] = hs_create(&g_s, c, rect_make(0, 0, 150, 100), WS_DEFAULT);
	w[1] = hs_create(&g_s, c, rect_make(30, 30, 150, 100), WS_DEFAULT);
	w[2] = hs_create(&g_s, c, rect_make(60, 60, 150, 100), WS_DEFAULT);
	hr_arg(&m, WMC_SET_STATE, w[0], WSTATE_MIN);
	hr_send(&g_s, c, &m);
	hs_key(&g_s, VK_TAB, INPM_ALT);
	h_eq_i64("premier Alt+Tab", g_s.t.active, hs_slot(&g_s, w[1]));
	hs_key(&g_s, VK_TAB, INPM_ALT);
	h_eq_i64("second Alt+Tab, reduite restauree", g_s.t.active,
		hs_slot(&g_s, w[0]));
	hs_key(&g_s, VK_TAB, INPM_ALT | INPM_SHIFT);
	h_eq_i64("Maj+Alt+Tab recule", g_s.t.active, hs_slot(&g_s, w[1]));
	hs_key(&g_s, VK_MENU, 0);
	hs_key(&g_s, VK_TAB, INPM_ALT);
	h_eq_i64("nouvelle session", g_s.t.active, hs_slot(&g_s, w[0]));
	h_eq_i64("tabulation avalee", hr_drain(c, WMS_KEY, NULL), 2);
	h_eq_i64("invariants", wz_check(&g_s.t), 0);
	hs_stop(&g_s);
}

static void	win_key(void)
{
	t_handle	shell;
	t_handle	app;
	uint32_t	bar;
	t_wmkey		k;
	t_wmarg		m;

	hs_start(&g_s, 320, 240);
	shell = hr_client(&g_s);
	app = hr_client(&g_s);
	bar = hs_create(&g_s, shell, rect_make(0, 210, 320, 30), WS_APPBAR);
	hs_create(&g_s, app, rect_make(10, 10, 150, 100), WS_DEFAULT);
	hr_drain(shell, 0, NULL);
	hs_key(&g_s, VK_LWIN, 0);
	h_eq_i64("touche Windows au shell", hr_drain(shell, WMS_KEY, &k), 2);
	h_eq_i64("vers la barre", k.h.window, bar);
	fk_push_key(INP_CHAR, VK_LWIN, 0);
	hs_input(&g_s);
	h_eq_i64("caractere [ a l'appli", hr_drain(app, WMS_KEY, NULL), 1);
	hr_arg(&m, WMC_SUBSCRIBE, 0, 1);
	hr_send(&g_s, app, &m);
	h_eq_i64("second shell refuse", hr_error(app), E_PERM);
	hs_stop(&g_s);
}

static void	explicit_capture(void)
{
	t_handle	shell;
	t_handle	app;
	uint32_t	pop;
	t_wmmouse	mm;

	hs_start(&g_s, 320, 240);
	shell = hr_client(&g_s);
	app = hr_client(&g_s);
	hr_value(&g_s, shell, WMC_SUBSCRIBE, (uint32_t [2]){0, 1});
	hs_create(&g_s, app, rect_make(10, 10, 150, 100), WS_DEFAULT);
	pop = hs_create(&g_s, shell, rect_make(0, 100, 100, 110), WS_POPUP
			| WS_TOPMOST | WS_NOACTIVATE);
	hr_value(&g_s, app, WMC_CAPTURE, (uint32_t [2]){g_s.t.w[0].id, 0});
	hr_value(&g_s, shell, WMC_CAPTURE, (uint32_t [2]){pop, 1});
	hr_drain(shell, 0, NULL);
	hs_key(&g_s, VK_DOWN, 0);
	h_eq_i64("clavier au menu", hr_drain(shell, WMS_KEY, NULL), 2);
	hs_click(&g_s, 300, 20);
	hr_drain(shell, WMS_MOUSE, &mm);
	h_true(mm.type == INP_MOUSE_UP && mm.hit == HT_NOWHERE, "clic exterieur");
	hr_value(&g_s, shell, WMC_CAPTURE, (uint32_t [2]){pop, 0});
	hr_drain(app, 0, NULL);
	hs_key(&g_s, VK_DOWN, 0);
	h_eq_i64("clavier rendu", hr_drain(app, WMS_KEY, NULL), 2);
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_keys");
	h_run("touches et Alt+F4", keys_to_active);
	h_run("Alt+Tab", alt_tab);
	h_run("touche Windows", win_key);
	h_run("capture explicite", explicit_capture);
	return (h_end());
}
