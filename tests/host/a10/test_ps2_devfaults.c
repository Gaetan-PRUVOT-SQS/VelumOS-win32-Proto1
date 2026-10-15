#include "harness.h"
#include "th.h"

static void	silent_keyboard(void)
{
	uint64_t	t0;

	th_ps2_reset();
	g_f8042.kbd_silent = 1;
	t0 = g_ftime.now;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("clavier muet : ignore", g_ps2.kbd_ok, 0);
	h_eq_u64("souris toujours prete", g_ps2.mouse_ok, 1);
	h_true(g_ftime.now - t0 < 600000000ull, "delai global borne");
	h_eq_i64("une seule IRQ", g_firq.count, 1);
}

static void	silent_mouse(void)
{
	th_ps2_reset();
	g_f8042.mouse_silent = 1;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("souris muette : ignoree", g_ps2.mouse_ok, 0);
	h_eq_u64("clavier toujours pret", g_ps2.kbd_ok, 1);
	h_eq_u64("config : IRQ1 + horloge 2 coupee", g_f8042.cfg, 0x25);
}

static void	resend_requests(void)
{
	th_ps2_reset();
	g_f8042.nak_kbd = 1;
	h_eq_i64("un renvoi demande", ps2_boot(), 0);
	h_eq_u64("clavier pret apres renvoi", g_ps2.kbd_ok, 1);
	h_eq_u64("commande de reinitialisation envoyee 2 fois", g_f8042.kbd_log[0],
		0xff);
	h_eq_u64("2e envoi", g_f8042.kbd_log[1], 0xff);
	th_ps2_reset();
	g_f8042.nak_kbd = 9;
	h_eq_i64("renvois sans fin", ps2_boot(), 0);
	h_eq_u64("abandon apres 3 essais", g_ps2.kbd_ok, 0);
	h_eq_u64("3 envois exactement", g_f8042.nkbd, 3);
}

static void	irq_refused(void)
{
	th_ps2_reset();
	g_firq.fail_gsi = 12;
	h_eq_i64("IRQ souris refusee", ps2_boot(), 0);
	h_eq_u64("souris desactivee", g_ps2.mouse_ok, 0);
	h_eq_u64("clavier actif", g_ps2.kbd_ok, 1);
	h_eq_u64("config : IRQ12 non armee, horloge 2 coupee", g_f8042.cfg & 0x22,
		0x20);
	h_true(fklog_has("refusée"), "journal");
}

int	main(void)
{
	h_begin("a10/ps2_devfaults");
	h_run("clavier muet", silent_keyboard);
	h_run("souris muette", silent_mouse);
	h_run("demandes de renvoi", resend_requests);
	h_run("IRQ refusee", irq_refused);
	return (h_end());
}
