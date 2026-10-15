#include "harness.h"
#include "th.h"

static void	single_release_after_loss(void)
{
	static const uint8_t	down[] = {0x12};
	static const uint8_t	lost[] = {0x00};
	static const uint8_t	up[] = {0xf0, 0x12};
	t_inpevent				ev[8];
	int						n;

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Maj enfoncee", th_kbd_in(down, 1, ev, 8), 1);
	n = th_kbd_in(lost, 1, ev, 8);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_SHIFT), "KEY_UP synthetique");
	n += th_kbd_in(up, 2, ev, 8);
	h_eq_i64("un seul KEY_UP au total", n, 1);
	h_eq_u64("rien de tenu", g_xlate.kbd.held, 0);
	h_eq_i64("nouvel appui emis", th_kbd_in(down, 1, ev, 8), 1);
	h_eq_i64("son relachement emis", th_kbd_in(up, 2, ev, 8), 1);
}

static void	ghost_release_silent(void)
{
	static const uint8_t	lctrl_up[] = {0xf0, 0x14};
	static const uint8_t	rwin_up[] = {0xe0, 0xf0, 0x27};
	static const uint8_t	key_up[] = {0xf0, 0x1c};
	t_inpevent				ev[8];

	th_boot_ready();
	h_eq_i64("Ctrl jamais enfoncee", th_kbd_in(lctrl_up, 2, ev, 8), 0);
	h_eq_i64("Windows droite jamais enfoncee", th_kbd_in(rwin_up, 3, ev, 8),
		0);
	h_eq_i64("touche ordinaire : relachement emis", th_kbd_in(key_up, 2, ev,
			8), 1);
	h_eq_u64("rien de tenu", g_xlate.kbd.held, 0);
}

static void	sides_are_distinct(void)
{
	static const uint8_t	ldown[] = {0x12};
	static const uint8_t	rup[] = {0xf0, 0x59};
	static const uint8_t	lup[] = {0xf0, 0x12};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Maj gauche enfoncee", th_kbd_in(ldown, 1, ev, 8), 1);
	h_eq_i64("relachement de Maj droite : rien", th_kbd_in(rup, 2, ev, 8), 0);
	h_eq_u64("Maj gauche toujours tenue", g_xlate.kbd.held, KM_LSHIFT);
	h_eq_i64("relachement de Maj gauche", th_kbd_in(lup, 2, ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_SHIFT), "KEY_UP Maj");
	h_eq_u64("rien de tenu", g_xlate.kbd.held, 0);
}

int	main(void)
{
	h_begin("a10/kbd_lost5");
	h_run("Maj, perte, vrai relachement : un seul KEY_UP",
		single_release_after_loss);
	h_run("relachement d'un modificateur jamais enfonce",
		ghost_release_silent);
	h_run("gauche et droite distinctes", sides_are_distinct);
	return (h_end());
}
