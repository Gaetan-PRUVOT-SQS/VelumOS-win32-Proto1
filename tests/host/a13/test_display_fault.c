#include "display_int.h"
#include "kfix.h"

static void	hook_returns_without_fault(void)
{
	fx_display(1024, 768);
	h_true(!g_display.hold_on_panic, "pas d'arret sans fault=panic");
	g_fk.screen("VelumOS", "essai");
	h_eq_i64("le crochet rend la main", g_fk.halts, 0);
	h_true(g_ffont.calls > 0, "ecran d'arret dessine");
}

static void	hook_holds_with_fault(void)
{
	fake_reset();
	fake_fb_set(1024, 768);
	fake_cmdline("selftest fault=panic");
	display_boot_init();
	h_true(g_display.hold_on_panic, "fault=panic demande l'arret");
	if (setjmp(g_fk.halt_env) == 0)
		g_fk.screen("VelumOS", "essai");
	h_eq_i64("la machine reste figee apres l'ecran", g_fk.halts, 1);
	h_true(fake_log_has("essai"), "ecran dessine avant l'arret");
}

static void	other_fault_values(void)
{
	fake_reset();
	fake_fb_set(256, 128);
	fake_cmdline("fault=div0");
	display_boot_init();
	h_true(!g_display.hold_on_panic, "fault=div0 ignore");
	fake_cmdline("fault=panicx");
	display_boot_init();
	h_true(!g_display.hold_on_panic, "valeur voisine ignoree");
	fake_cmdline("fault=");
	display_boot_init();
	h_true(!g_display.hold_on_panic, "valeur vide ignoree");
}

int	main(void)
{
	h_begin("a13/display_fault");
	h_run("panique sans option", hook_returns_without_fault);
	h_run("panique avec fault=panic", hook_holds_with_fault);
	h_run("panique autres valeurs de fault", other_fault_values);
	return (h_end());
}
