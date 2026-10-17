#include <string.h>
#include "harness.h"
#include "help.h"
#include "velum/theme.h"

static t_wsrv	g_s;

static void	move_and_resize(void)
{
	t_handle	c;
	uint32_t	id;
	int			slot;
	t_wmcreated	r;

	slot = hs_one(&g_s, &c, &id);
	hs_drag(&g_s, (t_point){100, 30}, (t_point){130, 60});
	h_true(g_s.t.w[slot].rect.x == 50 && g_s.t.w[slot].rect.y == 50,
		"deplacee par la legende");
	h_eq_i64("pas de nouvelle section", hr_drain(c, WMS_RESIZED, NULL), 0);
	hs_drag(&g_s, (t_point){249, 199}, (t_point){279, 219});
	h_true(g_s.t.w[slot].rect.w == 230 && g_s.t.w[slot].rect.h == 170,
		"redimensionnee par le coin");
	memset(&r, 0, sizeof(r));
	h_eq_i64("une section a la fin", hr_drain(c, WMS_RESIZED, &r), 1);
	h_true(r.width == 230 && r.height == 170 - VT_CAPTION_H
		&& r.stride == r.width, "dimensions client");
	h_eq_i64("contenu suivi", g_s.t.w[slot].content.w, 230);
	h_eq_i64("curseur de coin sous le pointeur", g_s.cur.shape, CUR_SIZE_NWSE);
	hs_stop(&g_s);
}

static void	caption_buttons(void)
{
	t_handle	c;
	uint32_t	id;
	int			slot;

	slot = hs_one(&g_s, &c, &id);
	hs_click(&g_s, hs_capbtn_x(20, 200, 0), hs_capbtn_y(20));
	h_eq_i64("fermer demande", hr_drain(c, WMS_CLOSE_REQ, NULL), 1);
	hs_drag(&g_s, (t_point){hs_capbtn_x(20, 200, 0), hs_capbtn_y(20)},
		(t_point){100, 100});
	h_eq_i64("relache ailleurs", hr_drain(c, WMS_CLOSE_REQ, NULL), 0);
	h_eq_i64("bouton relache", g_s.t.w[slot].pressed, 0);
	hs_click(&g_s, hs_capbtn_x(20, 200, 1), hs_capbtn_y(20));
	h_true(g_s.t.w[slot].state == WSTATE_MAX
		&& g_s.t.w[slot].rect.w == 320, "agrandie");
	hs_click(&g_s, 100, 10);
	hs_click(&g_s, 100, 10);
	h_true(g_s.t.w[slot].state == WSTATE_NORMAL
		&& g_s.t.w[slot].rect.x == 20, "double clic restaure");
	hs_click(&g_s, hs_capbtn_x(20, 200, 2), hs_capbtn_y(20));
	h_eq_i64("reduite", g_s.t.w[slot].state, WSTATE_MIN);
	h_eq_i64("plus active", g_s.t.active, -1);
	h_true(hr_drain(c, WMS_STATE, NULL) >= 3, "etats notifies");
	hs_stop(&g_s);
}

static void	client_events(void)
{
	t_handle	c;
	uint32_t	id;
	t_wmmouse	m;

	hs_one(&g_s, &c, &id);
	hs_point(&g_s, 50, 80);
	h_eq_i64("mouvement client", hr_drain(c, WMS_MOUSE, &m), 1);
	h_true(m.x == 50 - 20 && m.y == 80 - 20 - VT_CAPTION_H
		&& m.hit == HT_CLIENT, "coordonnees client");
	hs_point(&g_s, 300, 220);
	hr_drain(c, WMS_MOUSE, &m);
	h_eq_i64("sortie signalee", m.hit, HT_NOWHERE);
	hs_point(&g_s, 50, 80);
	fk_push_input(INP_MOUSE_DOWN, BTN_RIGHT, 0, 0);
	hs_input(&g_s);
	hr_drain(c, WMS_MOUSE, &m);
	h_true(m.type == INP_MOUSE_DOWN && m.buttons == (2u | (1u << 8)),
		"bouton droit et indice");
	hs_stop(&g_s);
}

static void	client_capture(void)
{
	t_handle	c;
	uint32_t	id;
	t_wmmouse	m;

	hs_one(&g_s, &c, &id);
	hs_point(&g_s, 50, 80);
	fk_push_input(INP_MOUSE_DOWN, BTN_RIGHT, 0, 0);
	hs_input(&g_s);
	hs_point(&g_s, 300, 230);
	hr_drain(c, WMS_MOUSE, &m);
	h_true(m.x == 300 - 20 && m.hit == HT_NOWHERE,
		"capture implicite");
	fk_push_input(INP_MOUSE_UP, BTN_RIGHT, 0, 0);
	fk_push_input(INP_WHEEL, 0, 0, -3);
	hs_input(&g_s);
	h_eq_i64("capture relachee", g_s.m.capture, -1);
	hs_point(&g_s, 60, 90);
	fk_push_input(INP_WHEEL, 0, 0, -3);
	hs_input(&g_s);
	hr_drain(c, WMS_MOUSE, &m);
	h_true(m.type == INP_WHEEL && m.wheel == -3, "molette");
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_mouse");
	h_run("deplacer et redimensionner", move_and_resize);
	h_run("boutons de legende", caption_buttons);
	h_run("evenements client", client_events);
	h_run("capture implicite", client_capture);
	return (h_end());
}
