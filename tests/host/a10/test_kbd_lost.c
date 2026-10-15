#include "harness.h"
#include "th.h"

static void	check_freed(const char *what)
{
	static const uint8_t	key[] = {0x1c};
	t_inpevent				ev[8];

	memset(ev, 0, sizeof(ev));
	h_eq_u64(what, g_xlate.kbd.held, 0);
	h_eq_i64("frappe suivante : 2 evenements", th_kbd_in(key, 1, ev, 8), 2);
	h_eq_u64("frappe suivante sans modificateur", ev[0].mods & (INPM_SHIFT
			| INPM_CTRL | INPM_ALT | INPM_WIN), 0);
	h_true(th_ev_is(&ev[1], INP_CHAR, 'q'), "caractere en minuscule");
}

static void	overrun_releases_shift(void)
{
	static const uint8_t	down[] = {0x12};
	static const uint8_t	lost[] = {0x00};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Maj enfoncee", th_kbd_in(down, 1, ev, 8), 1);
	h_eq_u64("etat enfonce", g_xlate.kbd.held, KM_LSHIFT);
	h_eq_i64("debordement 0x00 : 1 relachement", th_kbd_in(lost, 1, ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_SHIFT), "KEY_UP Maj emis");
	h_eq_u64("scancode du relachement", ev[0].scancode, 0x12);
	h_eq_u64("mods sans Maj", ev[0].mods & INPM_SHIFT, 0);
	h_true(ev[0].time_ns != 0, "relachement horodate");
	check_freed("Maj liberee apres debordement");
}

static void	bat_releases_ctrl_alt(void)
{
	static const uint8_t	down[] = {0x14, 0x11};
	static const uint8_t	bat[] = {0xaa};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Ctrl et Alt enfoncees", th_kbd_in(down, 2, ev, 8), 2);
	h_eq_u64("etat enfonce", g_xlate.kbd.held, KM_LCTRL | KM_LALT);
	h_eq_i64("reinitialisation 0xAA : 2 relachements", th_kbd_in(bat, 1, ev,
			8), 2);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_CONTROL), "KEY_UP Ctrl");
	h_true(th_ev_is(&ev[1], INP_KEY_UP, VK_MENU), "KEY_UP Alt");
	h_eq_u64("dernier relachement sans modificateur", ev[1].mods & (INPM_CTRL
			| INPM_ALT), 0);
	check_freed("Ctrl et Alt liberees apres 0xAA");
}

static void	parity_error_loses_break(void)
{
	static const uint8_t	down[] = {0xe0, 0x1f};
	static const uint8_t	brk[] = {0xe0, 0xf0};
	static const uint8_t	key[] = {0x1c};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Windows enfoncee", th_kbd_in(down, 2, ev, 8), 1);
	h_eq_u64("etat enfonce", g_xlate.kbd.held, KM_LWIN);
	h_eq_i64("debut du relachement", th_kbd_in(brk, 2, ev, 8), 0);
	f8042_push_err(0xff, 0);
	th_irq();
	h_eq_i64("erreur du 8042 : 1 relachement", th_pop_all(ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_LWIN), "KEY_UP Windows");
	h_eq_i64("octet suivant jete", th_kbd_in(key, 1, ev, 8), 0);
	check_freed("Windows liberee apres erreur du 8042");
}

int	main(void)
{
	h_begin("a10/kbd_lost");
	h_run("enfonce, debordement 0x00, frappe sans Maj",
		overrun_releases_shift);
	h_run("enfonce, reinitialisation 0xAA, frappe sans Ctrl ni Alt",
		bat_releases_ctrl_alt);
	h_run("enfonce, relachement perdu sur erreur 8042, frappe sans Windows",
		parity_error_loses_break);
	return (h_end());
}
