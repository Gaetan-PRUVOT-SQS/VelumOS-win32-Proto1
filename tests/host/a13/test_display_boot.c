#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	no_framebuffer(void)
{
	fake_reset();
	h_eq_i64("init sans framebuffer : 0", display_boot_init(), 0);
	h_true(!display_ready(), "affichage absent");
	h_eq_u64("largeur nulle", display_info()->width, 0);
	h_true(display_fb() == NULL, "pas de mappage");
	h_eq_u64("adresse physique nulle", display_fb_phys(), 0);
	h_eq_u64("aucun mappage demande", g_fvmm.io_calls, 0);
	h_eq_i64("pas de crochet de panique", g_fk.screens, 0);
	h_eq_i64("un message d'information", g_fk.info, 1);
	h_true(g_fp.sys[SYS_DISPLAY_INFO] && g_fp.sys[SYS_DISPLAY_MAP]
		&& g_fp.sys[SYS_DISPLAY_SET_MODE] && g_fp.sys[SYS_DISPLAY_MODES]
		&& g_fp.sys[SYS_KCON], "appels systeme enregistres quand meme");
	h_true(!kcon_enabled(), "console inactive");
}

static void	invalid_framebuffer(void)
{
	fake_reset();
	fake_fb_set(1024, 768);
	boot_info_rw()->fb.bpp = 24;
	h_eq_i64("init framebuffer refuse : 0", display_boot_init(), 0);
	h_true(!display_ready(), "affichage absent");
	h_eq_u64("aucun mappage demande", g_fvmm.io_calls, 0);
	h_eq_i64("un avertissement", g_fk.warn, 1);
	h_eq_i64("pas de crochet de panique", g_fk.screens, 0);
	h_eq_i64("appel info", fake_sys(SYS_DISPLAY_INFO, 1, 0, 0), E_NODEV);
}

static void	valid_quiet(void)
{
	fake_reset();
	fake_fb_set(1024, 768);
	h_eq_i64("init", display_boot_init(), 0);
	h_true(display_ready(), "affichage pret");
	h_eq_u64("largeur", display_info()->width, 1024);
	h_eq_u64("pas", display_info()->pitch, 4096);
	h_eq_u64("taille", display_info()->fb_size, 1024ull * 768 * 4);
	h_eq_u64("mappage en ecriture combinee", g_fvmm.last_io_flags, VM_WC);
	h_eq_u64("drapeau WC", display_info()->flags & DISP_FLAG_WC, DISP_FLAG_WC);
	h_eq_u64("adresse physique", display_fb_phys(), FAKE_FB_PHYS);
	h_eq_u64("longueur mappee", g_fvmm.win[0].len, 1024ull * 768 * 4);
	h_eq_i64("etat : ecran de demarrage", display_state(), DSP_SPLASH);
	h_eq_u64("ecran de demarrage dessine une fois", g_fl.calls, 1);
	h_true(g_fk.screen == display_panic_screen, "crochet de panique");
	h_eq_i64("pas d'abonnement au journal", g_fk.sinks, 0);
}

static void	valid_verbose(void)
{
	fake_reset();
	fake_fb_set(1024, 768);
	fake_cmdline("quiet verbose");
	display_boot_init();
	h_eq_i64("etat : console", display_state(), DSP_CONSOLE);
	h_true(kcon_enabled(), "console active");
	h_eq_u64("pas d'ecran de demarrage", g_fl.calls, 0);
	h_eq_i64("abonne au journal", g_fk.sinks, 1);
	h_true(g_fk.sink == kcon_write, "le puits est kcon_write");
	g_fk.sink("hello\n", 6);
	h_eq_u64("cinq glyphes", g_ffont.glyphs, 5);
}

int	main(void)
{
	h_begin("a13/display_boot");
	h_run("boot sans framebuffer", no_framebuffer);
	h_run("boot framebuffer invalide", invalid_framebuffer);
	h_run("boot silencieux (par defaut)", valid_quiet);
	h_run("boot verbeux", valid_verbose);
	return (h_end());
}
