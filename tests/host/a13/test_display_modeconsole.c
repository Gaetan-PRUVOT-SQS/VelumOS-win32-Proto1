#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	console_lost_after_switch(void)
{
	fake_reset();
	fake_fb_set(1024, 768);
	fake_cmdline("verbose");
	display_boot_init();
	h_true(kcon_enabled(), "console active au depart");
	g_ffont.cell_w = 65;
	h_eq_i64("changement de mode", display_set_mode(800, 600, 32), E_OK);
	h_eq_i64("console perdue : arret", display_state(), DSP_OFF);
	h_true(!kcon_enabled(), "console inactive");
	h_eq_i64("avertissement de la console", g_fk.warn, 1);
	kcon_write("zz", 2);
	h_eq_u64("aucune ecriture", g_ffont.glyphs, 0);
}

static void	console_back_after_good_switch(void)
{
	fake_reset();
	fake_fb_set(1024, 768);
	fake_cmdline("verbose");
	display_boot_init();
	g_ffont.cell_w = 65;
	display_set_mode(800, 600, 32);
	g_ffont.cell_w = 8;
	kcon_enable(true);
	h_true(!kcon_enabled(), "activation refusee : console non prete");
	h_eq_i64("appel KCON", fake_sys(SYS_KCON, 1, 0, 0), E_NOTSUP);
	h_eq_i64("mode suivant", display_set_mode(1024, 768, 32), E_OK);
	kcon_enable(true);
	h_true(kcon_enabled(), "console de nouveau possible");
	kcon_write("ok", 2);
	h_eq_u64("ecriture", g_ffont.glyphs, 2);
}

int	main(void)
{
	h_begin("a13/display_modeconsole");
	h_run("console perdue apres changement", console_lost_after_switch);
	h_run("console retrouvee ensuite", console_back_after_good_switch);
	return (h_end());
}
