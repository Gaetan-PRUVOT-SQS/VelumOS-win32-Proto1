#include "harness.h"
#include "th.h"

static void	button_transitions(void)
{
	t_mouse		m;
	t_mousepkt	p;
	t_inpevent	ev[MOUSE_OUT_MAX];
	uint32_t	prev;
	uint32_t	now;

	prev = 0;
	while (prev < 8)
	{
		now = 0;
		while (now < 8)
		{
			mouse_init(&m);
			m.buttons = (uint8_t)prev;
			p = (t_mousepkt){0, 0, 0, (uint8_t)now};
			g_h.name = "transitions de boutons 8x8";
			h_eq_i64("evenements", mouse_events(&m, &p, 0, ev),
				th_popcount(prev ^ now));
			h_eq_u64("etat memorise", m.buttons, now);
			now++;
		}
		prev++;
	}
}

static void	button_types(void)
{
	t_mouse		m;
	t_mousepkt	p;
	t_inpevent	ev[MOUSE_OUT_MAX];

	mouse_init(&m);
	p = (t_mousepkt){0, 0, 0, 5};
	h_eq_i64("gauche + milieu enfonces", mouse_events(&m, &p, 0, ev), 2);
	h_true(th_ev_is(&ev[0], INP_MOUSE_DOWN, BTN_LEFT), "gauche en premier");
	h_true(th_ev_is(&ev[1], INP_MOUSE_DOWN, BTN_MIDDLE), "milieu ensuite");
	p.buttons = 2;
	h_eq_i64("changement", mouse_events(&m, &p, 0, ev), 3);
	h_true(th_ev_is(&ev[0], INP_MOUSE_UP, BTN_LEFT), "gauche relache");
	h_true(th_ev_is(&ev[1], INP_MOUSE_DOWN, BTN_RIGHT), "droit enfonce");
	h_true(th_ev_is(&ev[2], INP_MOUSE_UP, BTN_MIDDLE), "milieu relache");
}

static void	move_and_wheel(void)
{
	t_mouse		m;
	t_mousepkt	p;
	t_inpevent	ev[MOUSE_OUT_MAX];

	mouse_init(&m);
	p = (t_mousepkt){5, 3, 0, 0};
	h_eq_i64("deplacement", mouse_events(&m, &p, INPM_CTRL, ev), 1);
	h_eq_u64("type", ev[0].type, INP_MOUSE_MOVE);
	h_eq_i64("x", ev[0].x, 5);
	h_eq_i64("y inverse (haut positif cote materiel)", ev[0].y, -3);
	h_eq_u64("mods du clavier recopies", ev[0].mods, INPM_CTRL);
	p = (t_mousepkt){0, 0, 2, 0};
	h_eq_i64("molette", mouse_events(&m, &p, 0, ev), 1);
	h_eq_u64("type molette", ev[0].type, INP_WHEEL);
	h_eq_i64("molette vers le haut = positif", ev[0].y, -2);
	p = (t_mousepkt){0, 0, 0, 0};
	h_eq_i64("paquet immobile", mouse_events(&m, &p, 0, ev), 0);
}

static void	full_packet_order(void)
{
	t_mouse		m;
	t_mousepkt	p;
	t_inpevent	ev[MOUSE_OUT_MAX];

	mouse_init(&m);
	p = (t_mousepkt){-4, 2, -1, 7};
	h_eq_i64("mouvement + 3 boutons + molette", mouse_events(&m, &p, 0, ev),
		5);
	h_eq_u64("1er : mouvement", ev[0].type, INP_MOUSE_MOVE);
	h_eq_u64("2e : bouton", ev[1].type, INP_MOUSE_DOWN);
	h_eq_u64("4e : bouton", ev[3].type, INP_MOUSE_DOWN);
	h_eq_u64("dernier : molette", ev[4].type, INP_WHEEL);
	h_eq_i64("molette vers le bas", ev[4].y, 1);
	h_true(5 <= MOUSE_OUT_MAX, "tampon de sortie suffisant");
}

int	main(void)
{
	h_begin("a10/mouse_ev");
	h_run("transitions de boutons", button_transitions);
	h_run("types et ordre des boutons", button_types);
	h_run("deplacement et molette", move_and_wheel);
	h_run("ordre dans un paquet complet", full_packet_order);
	return (h_end());
}
