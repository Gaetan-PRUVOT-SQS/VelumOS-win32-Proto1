#include "harness.h"
#include "th.h"

static void	key_make_break(void)
{
	static const uint8_t	make[] = {0x15};
	static const uint8_t	brk[] = {0xf0, 0x15};
	t_inpevent				ev[8];

	th_boot_ready();
	h_eq_i64("make : 2 evenements", th_kbd_in(make, 1, ev, 8), 2);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, 'A'), "KEY_DOWN A");
	h_true(th_ev_is(&ev[1], INP_CHAR, 'a'), "CHAR a");
	h_eq_u64("scancode", ev[0].scancode, 0x15);
	h_true(ev[0].time_ns != 0, "horodatage");
	h_eq_i64("break : 1 evenement", th_kbd_in(brk, 2, ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, 'A'), "KEY_UP A");
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

static void	extended_and_pause(void)
{
	static const uint8_t	arrow[] = {0xe0, 0x75, 0xe0, 0xf0, 0x75};
	static const uint8_t	pause[] = {0xe1, 0x14, 0x77, 0xe1, 0xf0, 0x14,
		0xf0, 0x77};
	static const uint8_t	prtsc[] = {0xe0, 0x12, 0xe0, 0x7c, 0xe0, 0xf0,
		0x7c, 0xe0, 0xf0, 0x12};
	t_inpevent				ev[8];

	th_boot_ready();
	h_eq_i64("fleche : appui et relachement", th_kbd_in(arrow, 5, ev, 8), 2);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, VK_UP), "VK_UP");
	h_eq_u64("drapeau etendu", ev[0].mods & INPM_EXTENDED, INPM_EXTENDED);
	h_eq_i64("pause", th_kbd_in(pause, 8, ev, 8), 2);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, VK_PAUSE), "VK_PAUSE");
	h_true(th_ev_is(&ev[1], INP_KEY_UP, VK_PAUSE), "relachement pause");
	h_eq_i64("impr ecran", th_kbd_in(prtsc, 10, ev, 8), 2);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, VK_SNAPSHOT), "VK_SNAPSHOT");
}

static void	wheel_mouse_packet(void)
{
	static const uint8_t	pkt[] = {0x09, 0x05, 0x03, 0x01};
	t_inpevent				ev[8];

	th_boot_ready();
	h_eq_i64("paquet : 3 evenements", th_mouse_in(pkt, 4, ev, 8), 3);
	h_true(th_ev_is(&ev[0], INP_MOUSE_MOVE, 0), "mouvement");
	h_eq_i64("x", ev[0].x, 5);
	h_eq_i64("y (ecran vers le bas)", ev[0].y, -3);
	h_true(th_ev_is(&ev[1], INP_MOUSE_DOWN, BTN_LEFT), "bouton gauche");
	h_true(th_ev_is(&ev[2], INP_WHEEL, 0), "molette");
	h_eq_i64("molette : 1 cran vers le bas", ev[2].y, -1);
	h_eq_u64("Verr Num dans les mods souris", ev[0].mods, INPM_NUM);
}

static void	interleaved_sources(void)
{
	t_inpevent	ev[8];
	int			n;

	th_boot_ready();
	f8042_push(0x15, 0);
	f8042_push(0x08, 1);
	f8042_push(0x01, 1);
	f8042_push(0x00, 1);
	f8042_push(0x00, 1);
	f8042_push(0xf0, 0);
	f8042_push(0x15, 0);
	th_irq();
	n = th_pop_all(ev, 8);
	h_eq_i64("clavier (3) + souris (1)", n, 4);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, 'A'), "1er : touche");
	h_true(th_ev_is(&ev[2], INP_MOUSE_MOVE, 0), "3e : souris");
	h_true(th_ev_is(&ev[3], INP_KEY_UP, 'A'), "dernier : relachement");
}

int	main(void)
{
	h_begin("a10/ps2_irq");
	h_run("appui et relachement", key_make_break);
	h_run("etendues, pause, impr ecran", extended_and_pause);
	h_run("paquet souris a molette", wheel_mouse_packet);
	h_run("sources entrelacees", interleaved_sources);
	return (h_end());
}
