#include "harness.h"
#include "th.h"

static void	no_mouse(void)
{
	th_ps2_reset();
	g_f8042.mouse_present = 0;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("clavier seul : clavier pret", g_ps2.kbd_ok, 1);
	h_eq_u64("clavier seul : pas de souris", g_ps2.mouse_ok, 0);
	h_eq_u64("config : IRQ1 + horloge 2 coupee", g_f8042.cfg, 0x25);
	h_eq_i64("une seule IRQ", g_firq.count, 1);
	h_eq_u64("IRQ clavier", g_firq.reg[0].gsi, 1);
}

static void	no_keyboard(void)
{
	th_ps2_reset();
	g_f8042.kbd_present = 0;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("souris seule : pas de clavier", g_ps2.kbd_ok, 0);
	h_eq_u64("souris seule : souris prete", g_ps2.mouse_ok, 1);
	h_eq_u64("config : IRQ12 + horloge 1 coupee", g_f8042.cfg, 0x16);
	h_eq_i64("une seule IRQ", g_firq.count, 1);
	h_eq_u64("IRQ souris", g_firq.reg[0].gsi, 12);
}

static void	single_channel(void)
{
	th_ps2_reset();
	g_f8042.dual = 0;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("canal unique detecte", g_ps2.dual, 0);
	h_eq_u64("pas de souris", g_ps2.mouse_ok, 0);
	h_eq_i64("test du port 2 non emis", th_cmd_index(0xa9, 0), -1);
	h_eq_i64("aucune ecriture vers la souris", g_f8042.nmouse, 0);
	h_eq_u64("config : IRQ1 + horloge 2 coupee", g_f8042.cfg, 0x25);
}

static void	no_device_at_all(void)
{
	th_ps2_reset();
	g_f8042.kbd_present = 0;
	g_f8042.mouse_present = 0;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("pas de clavier", g_ps2.kbd_ok, 0);
	h_eq_u64("pas de souris", g_ps2.mouse_ok, 0);
	h_eq_i64("aucune IRQ", g_firq.count, 0);
	h_eq_u64("config : les deux horloges coupees", g_f8042.cfg, 0x34);
}

int	main(void)
{
	h_begin("a10/ps2_topology");
	h_run("clavier sans souris", no_mouse);
	h_run("souris sans clavier", no_keyboard);
	h_run("controleur a canal unique", single_channel);
	h_run("aucun peripherique", no_device_at_all);
	return (h_end());
}
