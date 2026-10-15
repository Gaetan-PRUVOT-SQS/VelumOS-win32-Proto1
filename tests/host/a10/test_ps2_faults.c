#include "harness.h"
#include "th.h"
#include "velum/err.h"

static void	absent_controller(void)
{
	th_ps2_reset();
	g_f8042.float_bus = 1;
	h_eq_i64("bus flottant : retour 0", ps2_boot(), 0);
	h_eq_i64("aucune commande emise", g_f8042.ncmds, 0);
	h_eq_i64("aucune IRQ", g_firq.count, 0);
	h_true(fklog_has("pas de contrôleur PS/2"), "journal");
	h_eq_i64("input_boot_init : retour 0", input_boot_init(), 0);
}

static void	selftest_failure(void)
{
	th_ps2_reset();
	g_f8042.selftest = 0xfc;
	h_eq_i64("auto-test rate", ps2_boot(), E_IO);
	h_eq_i64("aucune IRQ", g_firq.count, 0);
	h_eq_i64("aucun octet envoye aux peripheriques", g_f8042.nkbd, 0);
	th_ps2_reset();
	g_f8042.selftest = 0xfc;
	h_eq_i64("input_boot_init non fatal", input_boot_init(), 0);
	h_true(fklog_has("indisponible"), "avertissement journalise");
}

static void	selftest_resets_config(void)
{
	th_ps2_reset();
	g_f8042.selftest_resets = 1;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("config finale rectifiee", g_f8042.cfg, 0x07);
	h_eq_u64("traduction du controleur coupee", g_f8042.cfg & 0x40, 0);
}

static void	port_test_failures(void)
{
	th_ps2_reset();
	g_f8042.porttest[0] = 1;
	h_eq_i64("port 1 defectueux", ps2_boot(), 0);
	h_eq_u64("clavier ignore", g_ps2.kbd_ok, 0);
	h_eq_u64("souris utilisable", g_ps2.mouse_ok, 1);
	h_eq_i64("aucun octet au clavier", g_f8042.nkbd, 0);
	th_ps2_reset();
	g_f8042.porttest[0] = 2;
	g_f8042.porttest[1] = 3;
	h_eq_i64("deux ports defectueux", ps2_boot(), 0);
	h_eq_i64("aucune IRQ", g_firq.count, 0);
	h_true(fklog_has("utilisable"), "journal");
}

int	main(void)
{
	h_begin("a10/ps2_faults");
	h_run("controleur absent", absent_controller);
	h_run("auto-test en echec", selftest_failure);
	h_run("auto-test qui efface la config", selftest_resets_config);
	h_run("tests de port en echec", port_test_failures);
	return (h_end());
}
