#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	remap_failure(void)
{
	void	*old;

	fx_display(1024, 768);
	old = display_fb();
	g_fvmm.fail_io_mask = 1u << 1;
	h_eq_i64("agrandissement sans memoire", display_set_mode(1920, 1080, 32),
		E_NOMEM);
	h_eq_u64("largeur inchangee", display_info()->width, 1024);
	h_true(display_fb() == old, "mappage inchange");
	h_eq_u64("registre largeur restaure", g_fd.regs[DISPI_INDEX_XRES], 1024);
	h_eq_u64("registre hauteur restaure", g_fd.regs[DISPI_INDEX_YRES], 768);
	h_eq_u64("mode reactive", g_fd.regs[DISPI_INDEX_ENABLE], DISPI_ON);
	h_eq_u64("generation inchangee", g_display.generation, 0);
	h_eq_i64("une fenetre", fake_vmm_live_windows(), 1);
	h_eq_i64("ensuite le changement fonctionne",
		display_set_mode(1920, 1080, 32), E_OK);
}

static void	device_refusal(void)
{
	fx_display(1024, 768);
	g_fd.reject_w = 1280;
	h_eq_i64("refus de l'appareil", display_set_mode(1280, 720, 32), E_IO);
	h_eq_u64("largeur inchangee", display_info()->width, 1024);
	h_eq_u64("registre largeur restaure", g_fd.regs[DISPI_INDEX_XRES], 1024);
	h_eq_i64("une fenetre", fake_vmm_live_windows(), 1);
	h_eq_i64("aucun mappage demande", g_fvmm.io_calls, 1);
	h_eq_u64("generation inchangee", g_display.generation, 0);
}

static void	no_leak_on_cycles(void)
{
	int	i;

	fx_display(1024, 768);
	i = 0;
	while (i < 20)
	{
		h_eq_i64("cycle 1920", display_set_mode(1920, 1080, 32), E_OK);
		h_eq_i64("cycle 800", display_set_mode(800, 600, 32), E_OK);
		h_eq_i64("cycle 1280", display_set_mode(1280, 1024, 32), E_OK);
		h_eq_i64("cycle 1024", display_set_mode(1024, 768, 32), E_OK);
		i++;
	}
	h_eq_i64("une seule fenetre vivante", fake_vmm_live_windows(), 1);
	h_eq_i64("aucune liberation inconnue", g_fvmm.bad_unmaps, 0);
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors du mappage");
}

static void	console_follows_mode(void)
{
	fake_reset();
	fake_fb_set(1024, 768);
	fake_cmdline("verbose");
	display_boot_init();
	kcon_write("abc", 3);
	h_eq_i64("800x600", display_set_mode(800, 600, 32), E_OK);
	h_eq_i64("colonnes", g_kcon.cols, 100);
	h_eq_i64("lignes", g_kcon.rows, 37);
	h_eq_i64("curseur au debut", g_kcon.row * 1000 + g_kcon.col, 0);
	h_eq_u64("ecran efface", display_surface()->px[0], 0xff000000);
	h_eq_i64("toujours en console", display_state(), DSP_CONSOLE);
	kcon_write("x", 1);
	h_eq_u64("ecriture sur la nouvelle surface", g_ffont.glyphs, 4);
}

int	main(void)
{
	h_begin("a13/display_modefail");
	h_run("echec repli mappage", remap_failure);
	h_run("echec refus de l'appareil", device_refusal);
	h_run("echec aucune fuite sur 80 changements", no_leak_on_cycles);
	h_run("etat console suit le mode", console_follows_mode);
	return (h_end());
}
