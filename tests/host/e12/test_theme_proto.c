#include <string.h>
#include "../a18/help.h"
#include "../harness.h"
#include "velum/luna.h"
#include "wmc_int.h"

static t_wsrv	g_s;

static void	reset_theme(void)
{
	ws_theme_reset();
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
}

static void	theme_msg(t_wmtheme *m, uint32_t window, uint32_t dark,
	uint32_t reduced)
{
	memset(m, 0, sizeof(*m));
	m->h.magic = WM_MAGIC;
	m->h.type = WMC_SET_THEME;
	m->h.size = sizeof(*m);
	m->h.window = window;
	m->dark = dark;
	m->reduced = reduced;
}

static void	broadcast_to_all(void)
{
	t_handle	a;
	t_handle	b;
	uint32_t	id;
	t_wmtheme	m;
	t_wmtheme	got;

	reset_theme();
	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	b = hr_client(&g_s);
	id = hs_create(&g_s, a, rect_make(20, 20, 200, 150), WS_DEFAULT);
	hr_drain(a, 0, NULL);
	hr_drain(b, 0, NULL);
	theme_msg(&m, id, 1, 1);
	hr_send(&g_s, a, &m);
	memset(&got, 0, sizeof(got));
	h_eq_i64("l'emetteur recoit WMS_THEME", hr_drain(a, WMS_THEME, &got), 1);
	h_eq_i64("sombre diffuse", got.dark, 1);
	h_eq_i64("transparence reduite diffusee", got.reduced, 1);
	h_eq_i64("fenetre 0 dans la diffusion", got.h.window, 0);
	memset(&got, 0, sizeof(got));
	h_eq_i64("l'autre client recoit aussi", hr_drain(b, WMS_THEME, &got), 1);
	h_eq_i64("meme contenu", got.dark, 1);
	h_eq_i64("le serveur est sombre", luna_theme_get(), LUNA_THEME_DARK);
	h_true(luna_transparency_reduced(), "transparence reduite cote serveur");
	hr_send(&g_s, a, &m);
	h_eq_i64("meme valeur : pas de rediffusion", hr_drain(b, WMS_THEME, NULL), 0);
	fk_close(a);
	fk_close(b);
	hs_stop(&g_s);
	reset_theme();
}

static void	late_client(void)
{
	t_handle	a;
	t_handle	b;
	uint32_t	id;
	t_wmtheme	m;
	t_wmtheme	got;

	reset_theme();
	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	id = hs_create(&g_s, a, rect_make(20, 20, 200, 150), WS_DEFAULT);
	theme_msg(&m, id, 1, 0);
	hr_send(&g_s, a, &m);
	b = hr_client(&g_s);
	memset(&got, 0, sizeof(got));
	h_eq_i64("nouveau client informe", hr_drain(b, WMS_THEME, &got), 1);
	h_eq_i64("sombre annonce", got.dark, 1);
	fk_close(a);
	fk_close(b);
	hs_stop(&g_s);
	reset_theme();
}

static void	refused_cases(void)
{
	t_handle	a;
	t_handle	b;
	uint32_t	ia;
	uint32_t	ib;
	t_wmtheme	m;

	reset_theme();
	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	b = hr_client(&g_s);
	ia = hs_create(&g_s, a, rect_make(20, 20, 150, 100), WS_DEFAULT);
	ib = hs_create(&g_s, b, rect_make(60, 60, 150, 100), WS_DEFAULT);
	hr_drain(a, 0, NULL);
	hr_drain(b, 0, NULL);
	theme_msg(&m, ia, 1, 0);
	hr_send(&g_s, a, &m);
	h_eq_i64("fenetre inactive : E_PERM", hr_error(a), E_PERM);
	h_eq_i64("theme inchange", luna_theme_get(), LUNA_THEME_LIGHT);
	theme_msg(&m, ib, 2, 0);
	hr_send(&g_s, b, &m);
	h_eq_i64("dark = 2 : E_INVAL", hr_error(b), E_INVAL);
	theme_msg(&m, ib, 0, 3);
	hr_send(&g_s, b, &m);
	h_eq_i64("reduced = 3 : E_INVAL", hr_error(b), E_INVAL);
	theme_msg(&m, ib, 1, 0);
	m.reserved = 7;
	hr_send(&g_s, b, &m);
	h_eq_i64("reserved != 0 : E_INVAL", hr_error(b), E_INVAL);
	h_eq_i64("toujours clair", luna_theme_get(), LUNA_THEME_LIGHT);
	fk_close(a);
	fk_close(b);
	hs_stop(&g_s);
	reset_theme();
}

static void	protocol_violations(void)
{
	t_handle	a;
	t_handle	b;
	uint32_t	ia;
	t_wmtheme	m;
	t_wmarg		short_msg;

	reset_theme();
	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	b = hr_client(&g_s);
	ia = hs_create(&g_s, a, rect_make(20, 20, 150, 100), WS_DEFAULT);
	theme_msg(&m, ia, 1, 0);
	hr_send(&g_s, b, &m);
	h_eq_i64("fenetre etrangere : deconnecte", hr_recv(b, &m, NULL), E_PIPE);
	hr_drain(a, 0, NULL);
	hr_arg(&short_msg, WMC_SET_THEME, ia, 1);
	hr_send(&g_s, a, &short_msg);
	h_eq_i64("taille t_wmarg : deconnecte", hr_recv(a, &m, NULL), E_PIPE);
	h_eq_i64("deux violations", g_s.violations, 2);
	h_eq_i64("theme inchange", luna_theme_get(), LUNA_THEME_LIGHT);
	fk_close(a);
	fk_close(b);
	hs_stop(&g_s);
	reset_theme();
}

static void	shell_versus_other(void)
{
	t_handle	shell;
	t_handle	app;
	t_handle	other;
	uint32_t	bar;
	uint32_t	ia;
	uint32_t	io;
	t_wmtheme	m;
	t_wmtheme	got;
	int			active;

	reset_theme();
	hs_start(&g_s, 320, 240);
	shell = hr_client(&g_s);
	other = hr_client(&g_s);
	app = hr_client(&g_s);
	bar = hs_create(&g_s, shell, rect_make(0, 210, 320, 30), WS_APPBAR);
	io = hs_create(&g_s, other, rect_make(10, 10, 80, 60), WS_DEFAULT);
	ia = hs_create(&g_s, app, rect_make(40, 40, 120, 80), WS_DEFAULT);
	hr_drain(shell, 0, NULL);
	hr_drain(other, 0, NULL);
	hr_drain(app, 0, NULL);
	active = g_s.t.active;
	h_true(bar != 0 && io != 0 && ia != 0, "trois fenetres");
	h_true(g_s.shell >= 0, "shell enregistre");
	h_true(active >= 0 && g_s.t.w[active].owner != g_s.shell,
		"la fenetre active n'est pas celle du shell");
	h_eq_i64("l'active est l'appli", active, hs_slot(&g_s, ia));
	theme_msg(&m, io, 1, 0);
	hr_send(&g_s, other, &m);
	h_eq_i64("ni active ni shell : E_PERM", hr_error(other), E_PERM);
	h_eq_i64("refus : theme inchange", luna_theme_get(), LUNA_THEME_LIGHT);
	theme_msg(&m, ia, 2, 0);
	hr_send(&g_s, app, &m);
	h_eq_i64("dark 2 : E_INVAL", hr_error(app), E_INVAL);
	theme_msg(&m, ia, 0, 2);
	hr_send(&g_s, app, &m);
	h_eq_i64("reduced 2 : E_INVAL", hr_error(app), E_INVAL);
	theme_msg(&m, ia, 1, 0);
	m.reserved = 1;
	hr_send(&g_s, app, &m);
	h_eq_i64("reserved 1 : E_INVAL", hr_error(app), E_INVAL);
	theme_msg(&m, ia, 0xffffffffu, 0);
	hr_send(&g_s, app, &m);
	h_eq_i64("dark max : E_INVAL", hr_error(app), E_INVAL);
	h_eq_i64("hors domaine : toujours clair", luna_theme_get(), LUNA_THEME_LIGHT);
	theme_msg(&m, bar, 1, 1);
	hr_send(&g_s, shell, &m);
	memset(&got, 0, sizeof(got));
	h_eq_i64("shell inactif accepte", hr_drain(app, WMS_THEME, &got), 1);
	h_eq_i64("sombre diffuse par le shell", got.dark, 1);
	h_eq_i64("transparence diffusee par le shell", got.reduced, 1);
	h_eq_i64("serveur sombre", luna_theme_get(), LUNA_THEME_DARK);
	hr_drain(other, 0, NULL);
	hr_drain(shell, 0, NULL);
	theme_msg(&m, ia, 0, 0);
	hr_send(&g_s, app, &m);
	memset(&got, 0, sizeof(got));
	h_eq_i64("fenetre active acceptee", hr_drain(other, WMS_THEME, &got), 1);
	h_eq_i64("retour clair", got.dark, 0);
	h_eq_i64("serveur clair", luna_theme_get(), LUNA_THEME_LIGHT);
	theme_msg(&m, io, 1, 0);
	hr_send(&g_s, other, &m);
	h_eq_i64("toujours E_PERM pour l'autre", hr_error(other), E_PERM);
	h_eq_i64("le refus ne rallume pas le sombre", luna_theme_get(),
		LUNA_THEME_LIGHT);
	fk_close(shell);
	fk_close(app);
	fk_close(other);
	hs_stop(&g_s);
	reset_theme();
}

static void	client_api(void)
{
	t_wmwin	w;

	memset(&w, 0, sizeof(w));
	h_eq_i64("fenetre nulle", wmc_set_theme(NULL, 0, 0), E_INVAL);
	h_eq_i64("id nul", wmc_set_theme(&w, 1, 0), E_INVAL);
	w.id = 4;
	h_eq_i64("dark 2 cote client", wmc_set_theme(&w, 2, 0), E_INVAL);
	h_eq_i64("reduced 2 cote client", wmc_set_theme(&w, 0, 2), E_INVAL);
	h_eq_i64("dark max cote client", wmc_set_theme(&w, 0xffffffffu, 1),
		E_INVAL);
}

static void	client_side(void)
{
	t_wmcmsg	m;
	t_wmtheme	t;

	reset_theme();
	theme_msg(&t, 0, 1, 1);
	t.h.type = WMS_THEME;
	memset(&m, 0, sizeof(m));
	memcpy(m.buf, &t, sizeof(t));
	m.len = sizeof(t);
	h_true(wmc_valid(&m), "WMS_THEME de la bonne taille valide");
	wmc_on_message(&m);
	h_eq_i64("client sombre", luna_theme_get(), LUNA_THEME_DARK);
	h_true(luna_transparency_reduced(), "client transparence reduite");
	t.dark = 0;
	t.reduced = 0;
	memcpy(m.buf, &t, sizeof(t));
	wmc_on_message(&m);
	h_eq_i64("retour au clair", luna_theme_get(), LUNA_THEME_LIGHT);
	t.dark = 5;
	memcpy(m.buf, &t, sizeof(t));
	luna_theme_set(LUNA_THEME_DARK);
	wmc_on_message(&m);
	h_eq_i64("valeur hors borne ignoree", luna_theme_get(), LUNA_THEME_DARK);
	m.len = sizeof(t) - 4;
	h_true(!wmc_valid(&m), "taille tronquee invalide");
	reset_theme();
}

int	main(void)
{
	h_begin("e12/theme_proto");
	h_run("e12-theme-diffusion", broadcast_to_all);
	h_run("e12-theme-client-tardif", late_client);
	h_run("e12-theme-refus", refused_cases);
	h_run("e12-theme-shell", shell_versus_other);
	h_run("e12-theme-api-client", client_api);
	h_run("e12-theme-violations", protocol_violations);
	h_run("e12-theme-cote-client", client_side);
	return (h_end());
}
