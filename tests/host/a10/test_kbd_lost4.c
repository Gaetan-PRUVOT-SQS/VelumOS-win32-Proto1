#include "harness.h"
#include "th.h"

static void	error_on_break_prefix(void)
{
	static const uint8_t	shift[] = {0x12};
	static const uint8_t	key[] = {0x1b, 0xf0, 0x1b};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Maj enfoncee", th_kbd_in(shift, 1, ev, 8), 1);
	f8042_push_err(0xf0, 0);
	th_irq();
	h_eq_i64("erreur sur F0 : 1 relachement", th_pop_all(ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_SHIFT), "KEY_UP Maj");
	h_eq_i64("reste 12 du relachement jete", th_kbd_in(shift, 1, ev, 8), 0);
	h_eq_u64("rien de tenu", g_xlate.kbd.held, 0);
	h_eq_i64("frappe suivante normale", th_kbd_in(key, 3, ev, 8), 3);
	h_true(th_ev_is(&ev[1], INP_CHAR, 's'), "minuscule");
}

static void	error_on_ext_prefix(void)
{
	static const uint8_t	rctrl[] = {0xe0, 0x14};
	static const uint8_t	rest[] = {0xf0, 0x14};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Ctrl droite enfoncee", th_kbd_in(rctrl, 2, ev, 8), 1);
	h_eq_u64("etat enfonce", g_xlate.kbd.held, KM_RCTRL);
	f8042_push_err(0xe0, 0);
	th_irq();
	h_eq_i64("erreur sur E0 : 1 relachement", th_pop_all(ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_CONTROL), "KEY_UP Ctrl");
	h_eq_i64("reste F0 14 : aucun evenement", th_kbd_in(rest, 2, ev, 8), 0);
	h_eq_u64("rien de tenu", g_xlate.kbd.held, 0);
}

static void	error_on_last_byte(void)
{
	static const uint8_t	a[] = {0x1c};
	static const uint8_t	s[] = {0x1b};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	f8042_push_err(0x1c, 0);
	th_irq();
	h_eq_i64("octet en erreur ignore", th_pop_all(ev, 8), 0);
	h_eq_i64("frappe suivante jetee (cout accepte)", th_kbd_in(a, 1, ev, 8),
		0);
	h_eq_i64("un seul octet jete", th_kbd_in(s, 1, ev, 8), 2);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, 'S'), "KEY_DOWN S");
	h_true(th_ev_is(&ev[1], INP_CHAR, 's'), "CHAR s");
}

static void	error_then_full_break(void)
{
	static const uint8_t	a[] = {0x1c};
	static const uint8_t	brk[] = {0xf0, 0x1c};
	static const uint8_t	s[] = {0x1b};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("appui", th_kbd_in(a, 1, ev, 8), 2);
	f8042_push_err(0xff, 0);
	th_irq();
	h_eq_i64("erreur sans modificateur", th_pop_all(ev, 8), 0);
	h_eq_i64("F0 1C complet decode", th_kbd_in(brk, 2, ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, 'Q'), "KEY_UP de la touche");
	h_eq_i64("drapeau consomme par le prefixe", th_kbd_in(s, 1, ev, 8), 2);
	h_eq_u64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

int	main(void)
{
	h_begin("a10/kbd_lost4");
	h_run("Maj, erreur sur F0, 12 restant : rien de tenu",
		error_on_break_prefix);
	h_run("Ctrl droite, erreur sur E0, F0 14 : rien de tenu",
		error_on_ext_prefix);
	h_run("erreur sur dernier octet, frappe jetee, frappe normale",
		error_on_last_byte);
	h_run("erreur puis F0 1C complet decode", error_then_full_break);
	return (h_end());
}
