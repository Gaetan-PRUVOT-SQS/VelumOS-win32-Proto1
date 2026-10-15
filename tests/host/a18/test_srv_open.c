#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	open_ok(void)
{
	h_eq_i64("ouverture", hs_start(&g_s, 320, 240), 0);
	h_eq_i64("console noyau coupee", g_fk.kcon, 1);
	h_true(g_fk.listener >= 0 && g_s.input != 0, "port et entree ouverts");
	h_eq_i64("stride du framebuffer", g_s.fb.stride, 336);
	ws_step(&g_s);
	h_eq_i64("premiere trame composee", g_s.frames, 1);
	h_eq_i64("pixel du bureau", g_fk.fb[200 * 336 + 10] & 0xffffff,
		WS_DESK_COLOR & 0xffffff);
	h_eq_i64("marge du pitch intacte", g_fk.fb[10 * 336 + 330], 0);
	h_eq_i64("rien a recomposer ensuite", g_s.dirty.n, 0);
	hs_stop(&g_s);
	h_eq_i64("aucun objet restant", fk_live(), 0);
}

static void	open_failures(void)
{
	fk_reset();
	h_eq_i64("pas d'ecran", ws_open(&g_s), E_NODEV);
	fk_set_display(320, 240);
	g_fk.fail_alloc = 0;
	h_eq_i64("tampon arriere refuse", ws_open(&g_s), E_NOMEM);
	h_eq_i64("console intacte si echec", g_fk.kcon, 0);
	h_eq_i64("rien ne fuit", fk_live(), 0);
	ws_sys_listen("wm");
	h_eq_i64("port deja pris", ws_open(&g_s), E_EXIST);
	h_true(g_s.back.px == NULL && g_s.input == 0, "nettoyage complet");
	fk_reset();
	fk_set_display(320, 240);
	g_fk.fb_pitch = 4;
	h_eq_i64("pitch incoherent refuse", ws_open(&g_s), E_NODEV);
	fk_reset();
}

static void	hello_rules(void)
{
	t_handle	c;
	t_wmhello	m;

	hs_start(&g_s, 320, 240);
	c = hr_connect(&g_s);
	h_eq_i64("bonjour", hr_hello(&g_s, c), 0);
	hv_hello(&m);
	hr_send(&g_s, c, &m);
	h_eq_i64("second bonjour = deconnexion", hr_drain(c, 0, NULL), 0);
	h_eq_i64("canal ferme", hr_recv(c, &m, NULL), E_PIPE);
	fk_close(c);
	c = hr_connect(&g_s);
	h_eq_i64("creation avant bonjour", hr_create(&g_s, c, rect_make(0, 0,
				100, 100), WS_DEFAULT), 0);
	h_eq_i64("client rejete", hr_recv(c, &m, NULL), E_PIPE);
	h_eq_i64("aucune fenetre", g_s.t.nused, 0);
	h_eq_i64("violations comptees", g_s.violations, 2);
	fk_close(c);
	hs_stop(&g_s);
}

static void	create_window(void)
{
	t_handle	c;
	uint32_t	id;
	int			slot;
	t_rect		cl;

	hs_start(&g_s, 320, 240);
	c = hr_connect(&g_s);
	hr_hello(&g_s, c);
	id = hr_create(&g_s, c, rect_make(20, 20, 200, 150), WS_DEFAULT);
	slot = wt_find(&g_s.t, id);
	h_true(id != 0 && slot >= 0, "fenetre creee");
	cl = wh_client(&g_s.t, slot);
	h_true(g_s.t.w[slot].content.w == cl.w
		&& g_s.t.w[slot].content.h == cl.h, "section aux dimensions client");
	h_eq_i64("fenetre active", g_s.t.active, slot);
	h_eq_i64("octets comptes", g_s.c[0].sec_bytes, g_s.t.w[slot].sec_bytes);
	h_eq_i64("activation recue", hr_drain(c, WMS_ACTIVATE, NULL), 1);
	fk_close(c);
	hs_settle(&g_s, c);
	h_eq_i64("fenetre detruite a la deconnexion", g_s.t.nused, 0);
	h_eq_i64("sections liberees", g_s.sec_total, 0);
	hs_stop(&g_s);
	h_eq_i64("aucun objet restant", fk_live(), 0);
}

int	main(void)
{
	h_begin("a18/srv_open");
	h_run("ouverture", open_ok);
	h_run("echecs d'ouverture", open_failures);
	h_run("regles du bonjour", hello_rules);
	h_run("creation de fenetre", create_window);
	return (h_end());
}
