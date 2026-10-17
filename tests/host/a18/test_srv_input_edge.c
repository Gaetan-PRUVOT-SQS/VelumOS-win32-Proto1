#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	send(uint32_t type, uint32_t code, int32_t x, int32_t y)
{
	t_inpevent	ev;

	memset(&ev, 0, sizeof(ev));
	ev.type = type;
	ev.code = code;
	ev.x = x;
	ev.y = y;
	ws_input(&g_s, &ev);
}

static void	at(const char *what, int32_t x, int32_t y)
{
	h_true(g_s.m.pos.x == x && g_s.m.pos.y == y, what);
	h_true(g_s.cur.pos.x == x && g_s.cur.pos.y == y,
		"le curseur suit la position");
}

static void	deplacements_extremes(void)
{
	hs_start(&g_s, 640, 480);
	send(INP_MOUSE_MOVE, 0, INT32_MAX, INT32_MAX);
	at("delta maximal : coin bas droit", 639, 479);
	send(INP_MOUSE_MOVE, 0, INT32_MAX, INT32_MAX);
	at("encore au maximum : inchange", 639, 479);
	send(INP_MOUSE_MOVE, 0, INT32_MIN, INT32_MIN);
	at("delta minimal : coin haut gauche", 0, 0);
	send(INP_MOUSE_MOVE, 0, -1, -1);
	at("un pixel au dela du bord haut gauche", 0, 0);
	send(INP_MOUSE_MOVE, 0, 639, 479);
	at("exactement le coin bas droit", 639, 479);
	send(INP_MOUSE_MOVE, 0, 1, 1);
	at("un pixel au dela du bord bas droit", 639, 479);
	send(INP_MOUSE_MOVE, 0, -16385, 16385);
	at("delta au dela de la borne de 16384", 0, 479);
	hs_stop(&g_s);
}

static void	boutons_et_types_invalides(void)
{
	hs_start(&g_s, 640, 480);
	send(INP_MOUSE_DOWN, BTN_MIDDLE + 1, 0, 0);
	send(INP_MOUSE_DOWN, 31, 0, 0);
	send(INP_MOUSE_DOWN, 0xffffffffu, 0, 0);
	h_eq_u64("bouton hors table ignore", g_s.m.buttons, 0);
	send(INP_MOUSE_DOWN, BTN_MIDDLE, 0, 0);
	h_eq_u64("bouton du milieu", g_s.m.buttons, 4);
	send(INP_MOUSE_UP, 0xffffffffu, 0, 0);
	h_eq_u64("relachement hors table ignore", g_s.m.buttons, 4);
	send(INP_MOUSE_UP, BTN_MIDDLE, 0, 0);
	h_eq_u64("relachement du milieu", g_s.m.buttons, 0);
	send(0, 0, 100, 100);
	send(0xffffffffu, 0, 100, 100);
	send(INP_WHEEL, 0, 0, INT32_MIN);
	send(INP_WHEEL, 0, 0, INT32_MAX);
	at("types inconnus et molette sans fenetre : position inchangee",
		320, 240);
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_input_edge");
	h_run("deplacements aux valeurs limites des entiers",
		deplacements_extremes);
	h_run("boutons et types hors table", boutons_et_types_invalides);
	return (h_end());
}
