#include "harness.h"
#include "th.h"

static void	nominal_state(void)
{
	th_ps2_reset();
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("double canal", g_ps2.dual, 1);
	h_eq_u64("clavier pret", g_ps2.kbd_ok, 1);
	h_eq_u64("souris prete", g_ps2.mouse_ok, 1);
	h_eq_u64("souris a molette (id 3)", g_ps2.mouse_id, 3);
	h_eq_u64("paquets de 4 octets", g_ps2.mdec.wheel, 1);
	h_eq_u64("jeu de codes 2 selectionne", g_f8042.scanset, 2);
	h_eq_u64("clavier : balayage actif", g_f8042.kbd_scanning, 1);
	h_eq_u64("souris : flux actif", g_f8042.mouse_stream, 1);
	h_eq_u64("souris : derniere cadence 100", g_f8042.rates[2], 100);
	h_eq_u64("octet de config final", g_f8042.cfg, 0x07);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

static void	command_order(void)
{
	int	dis1;
	int	selftest;
	int	test1;
	int	en1;

	th_ps2_reset();
	ps2_boot();
	dis1 = th_cmd_index(0xad, 0);
	selftest = th_cmd_index(0xaa, 0);
	test1 = th_cmd_index(0xab, 0);
	en1 = th_cmd_index(0xae, 0);
	h_eq_i64("1ere commande : desactiver le port 1", g_f8042.cmds[0], 0xad);
	h_eq_i64("2e commande : desactiver le port 2", g_f8042.cmds[1], 0xa7);
	h_true(dis1 < selftest && selftest < test1 && test1 < en1,
		"ordre : desactivation, auto-test, test des ports, activation");
	h_true(th_cmd_index(0xa9, 0) > selftest, "test du port 2 apres auto-test");
}

static void	irq_registrations(void)
{
	th_ps2_reset();
	ps2_boot();
	h_eq_i64("deux IRQ demandees", g_firq.count, 2);
	h_eq_u64("IRQ clavier (GSI 1)", g_firq.reg[0].gsi, 1);
	h_eq_u64("IRQ souris (GSI 12)", g_firq.reg[1].gsi, 12);
	h_true(g_firq.reg[0].fn == ps2_irq_handler, "gestionnaire clavier");
	h_true(g_firq.reg[1].ctx == &g_ps2, "contexte souris");
	h_eq_u64("front montant par defaut", g_firq.reg[0].flags, 0);
	th_ps2_reset();
	g_firq.gsi_of_isa[1] = 9;
	g_firq.gsi_of_isa[12] = 20;
	ps2_boot();
	h_eq_u64("surcharge ACPI clavier", g_firq.reg[0].gsi, 9);
	h_eq_u64("surcharge ACPI souris", g_firq.reg[1].gsi, 20);
}

static void	led_numlock_at_boot(void)
{
	uint8_t	last;

	th_ps2_reset();
	ps2_boot();
	last = g_f8042.kbd_log[g_f8042.nkbd - 1];
	h_eq_u64("sequence LED engagee (ED envoye)", last, 0xed);
	th_irq();
	h_eq_u64("Verr Num allumee apres les ACK", g_f8042.leds, INPUT_LED_NUM);
	h_eq_u64("machine LED au repos", g_ps2.led.state, LED_IDLE);
}

int	main(void)
{
	h_begin("a10/ps2_boot");
	h_run("etat nominal apres ps2_boot", nominal_state);
	h_run("ordre des commandes 8042", command_order);
	h_run("demandes d'IRQ", irq_registrations);
	h_run("Verr Num au demarrage", led_numlock_at_boot);
	return (h_end());
}
