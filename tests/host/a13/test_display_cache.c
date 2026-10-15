#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	without_pat(void)
{
	fake_reset();
	fake_fb_set(256, 128);
	g_fk.cpu.pat = false;
	display_boot_init();
	h_true(display_ready(), "affichage pret sans PAT");
	h_eq_u64("mappage sans cache", g_fvmm.last_io_flags, VM_NOCACHE);
	h_eq_u64("un seul essai", g_fvmm.io_calls, 1);
	h_eq_u64("pas de drapeau WC", display_info()->flags & DISP_FLAG_WC, 0);
	h_eq_u64("memoire des mappages", g_display.map_flags, VM_NOCACHE);
}

static void	wc_fails_nocache_works(void)
{
	fake_reset();
	fake_fb_set(256, 128);
	g_fvmm.fail_io_mask = 1;
	display_boot_init();
	h_true(display_ready(), "repli sur NOCACHE");
	h_eq_u64("deux essais", g_fvmm.io_calls, 2);
	h_eq_u64("dernier essai sans cache", g_fvmm.last_io_flags, VM_NOCACHE);
	h_eq_u64("pas de drapeau WC", display_info()->flags & DISP_FLAG_WC, 0);
	h_eq_i64("une seule fenetre", fake_vmm_live_windows(), 1);
}

static void	both_mappings_fail(void)
{
	fake_reset();
	fake_fb_set(256, 128);
	g_fvmm.fail_io_mask = 3;
	h_eq_i64("init : 0 meme sans mappage", display_boot_init(), 0);
	h_true(!display_ready(), "affichage absent");
	h_eq_i64("un avertissement", g_fk.warn, 1);
	h_eq_i64("aucune fenetre", fake_vmm_live_windows(), 0);
	h_eq_i64("pas de crochet de panique", g_fk.screens, 0);
	h_true(display_fb() == NULL, "pas de mappage");
	h_eq_i64("appel MODES", fake_sys(SYS_DISPLAY_MODES, 1, 4, 0), E_NODEV);
}

static void	font_missing(void)
{
	fake_reset();
	fake_fb_set(256, 128);
	g_ffont.null_font = true;
	fake_cmdline("verbose");
	display_boot_init();
	h_true(display_ready(), "affichage pret sans police");
	h_true(!kcon_enabled(), "console refusee");
	h_eq_i64("repli sur l'ecran de demarrage", display_state(), DSP_SPLASH);
	h_eq_i64("un avertissement de la console", g_fk.warn, 1);
	kcon_blue_screen("VelumOS", "x");
	h_eq_u64("ecran d'arret : fond seul", g_ffont.calls, 0);
}

int	main(void)
{
	h_begin("a13/display_cache");
	h_run("cache sans PAT", without_pat);
	h_run("cache WC refuse puis NOCACHE", wc_fails_nocache_works);
	h_run("cache les deux mappages echouent", both_mappings_fail);
	h_run("cache police absente", font_missing);
	return (h_end());
}
