#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	splash_progress(void)
{
	fx_display(160, 96);
	h_eq_u64("tick initial", g_fl.tick, 1);
	display_boot_progress(50, "pmm");
	h_eq_u64("pourcentage", g_fl.pct, 50);
	h_eq_u64("tick incremente", g_fl.tick, 2);
	display_boot_progress(150, "x");
	h_eq_u64("pourcentage plafonne a 100", g_fl.pct, 100);
	display_boot_progress(101, NULL);
	h_eq_u64("101 plafonne a 100", g_fl.pct, 100);
	display_boot_progress(0, NULL);
	h_eq_u64("tick 5", g_fl.tick, 5);
	h_eq_u64("cinq dessins", g_fl.calls, 5);
	h_true(fake_count(display_surface(), rect_make(0, 0, 160, 96), 0xff0058ee)
		== 0, "pourcentage 0 : barre vide");
}

static void	done_stops_drawing(void)
{
	uint32_t	n;

	fx_display(160, 96);
	display_boot_done();
	h_eq_i64("etat arrete", display_state(), DSP_OFF);
	n = g_fl.calls;
	display_boot_progress(80, "x");
	h_eq_u64("plus de dessin", g_fl.calls, n);
	display_boot_done();
	h_eq_i64("arret idempotent", display_state(), DSP_OFF);
	memset(&g_display, 0, sizeof(g_display));
	display_boot_progress(10, "cpu");
	display_boot_done();
	h_eq_u64("avant l'init : aucun dessin", g_fl.calls, n);
}

static void	enable_disable_cycle(void)
{
	t_surface	*s;

	fx_display(160, 96);
	display_boot_progress(50, "x");
	kcon_enable(true);
	s = display_surface();
	h_eq_i64("etat : console", display_state(), DSP_CONSOLE);
	h_eq_u64("ecran efface",
		fake_count(s, rect_make(0, 0, 160, 96), 0xff0058ee), 0);
	kcon_write("ab\n", 3);
	h_eq_u64("deux glyphes", g_ffont.glyphs, 2);
	kcon_enable(false);
	h_eq_i64("etat : arret", display_state(), DSP_OFF);
	kcon_write("zz", 2);
	h_eq_u64("plus d'ecriture apres arret", g_ffont.glyphs, 2);
	s->px[0] = 0xff123456;
	kcon_clear();
	h_eq_u64("effacement ignore apres arret", s->px[0], 0xff123456);
	kcon_enable(true);
	h_eq_u64("reactivation : ecran efface", s->px[0], 0xff000000);
	h_eq_i64("curseur au debut", g_kcon.row * 100 + g_kcon.col, 0);
}

static void	invalid_transitions(void)
{
	fx_display(160, 96);
	kcon_enable(true);
	g_fl.calls = 0;
	display_boot_progress(10, "a");
	h_eq_u64("progression ignoree en console", g_fl.calls, 0);
	display_boot_done();
	h_eq_i64("done sans effet en console", display_state(), DSP_CONSOLE);
	kcon_write("hi", 2);
	kcon_enable(true);
	h_true(fake_count(display_surface(), rect_make(0, 0, 8, 16), KCON_FG)
		> 0, "texte conserve");
	kcon_enable(false);
	kcon_enable(false);
	h_eq_i64("double arret", display_state(), DSP_OFF);
	memset(&g_display, 0, sizeof(g_display));
	kcon_enable(true);
	h_eq_i64("activation avant l'init : sans effet", display_state(), DSP_OFF);
}

int	main(void)
{
	h_begin("a13/display_state");
	h_run("etat progression du splash", splash_progress);
	h_run("etat fin du splash", done_stops_drawing);
	h_run("etat console active puis coupee", enable_disable_cycle);
	h_run("etat transitions invalides", invalid_transitions);
	return (h_end());
}
