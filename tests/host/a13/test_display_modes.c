#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	listing(void)
{
	t_dispmode	m[8];

	fx_display(1024, 768);
	h_eq_u64("5 modes via l'appareil", display_modes(m, 8), 5);
	h_eq_u64("mode courant marque", m[1].flags, DISP_MODE_CURRENT);
	h_eq_u64("mode courant 1024", m[1].width, 1024);
	h_eq_u64("interrogation du nombre", display_modes(NULL, 0), 5);
	h_eq_u64("sortie nulle", display_modes(NULL, 4), 0);
	h_eq_u64("taille maximale respectee", display_modes(m, 2), 2);
	h_eq_i64("une seule ouverture materielle", g_fd.open_calls, 1);
}

static void	no_device(void)
{
	t_dispmode	m[8];

	fx_display(1024, 768);
	g_fd.present = false;
	h_eq_u64("sans appareil : le mode courant seul", display_modes(m, 8), 1);
	h_eq_u64("largeur", m[0].width, 1024);
	h_eq_u64("marque courant", m[0].flags, DISP_MODE_CURRENT);
	h_eq_u64("interrogation", display_modes(NULL, 0), 1);
	h_eq_i64("autre mode : non supporte", display_set_mode(800, 600, 32),
		E_NOTSUP);
	h_eq_i64("mode courant : accepte", display_set_mode(1024, 768, 32), E_OK);
	h_eq_u64("aucune ecriture de registre", g_fd.nlog, 0);
	g_fd.present = true;
	h_eq_i64("appareil apparu plus tard", display_set_mode(800, 600, 32),
		E_OK);
}

static void	grow_mode(void)
{
	void	*old;

	fx_display(1024, 768);
	old = display_fb();
	h_eq_i64("1920x1080", display_set_mode(1920, 1080, 32), E_OK);
	h_eq_u64("largeur", display_info()->width, 1920);
	h_eq_u64("hauteur", display_info()->height, 1080);
	h_eq_u64("pas", display_info()->pitch, 7680);
	h_eq_u64("taille", display_info()->fb_size, 8294400);
	h_true(display_fb() != old, "nouveau mappage");
	h_eq_i64("une fenetre vivante", fake_vmm_live_windows(), 1);
	h_eq_i64("ancienne fenetre liberee", g_fvmm.io_unmaps, 1);
	h_true(display_surface()->px == display_fb(), "surface sur le mappage");
	h_eq_u64("generation", g_display.generation, 1);
	h_eq_i64("ecran de demarrage redessine a la nouvelle taille", g_fl.w, 1920);
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors du mappage");
}

static void	shrink_then_grow(void)
{
	fx_display(1024, 768);
	h_eq_i64("800x600", display_set_mode(800, 600, 32), E_OK);
	h_eq_i64("mappage conserve", g_fvmm.io_maps, 1);
	h_eq_i64("rien de libere", g_fvmm.io_unmaps, 0);
	h_eq_u64("pas", display_info()->pitch, 3200);
	h_eq_u64("surface : hauteur", display_surface()->h, 600);
	h_eq_i64("1280x1024 : agrandissement", display_set_mode(1280, 1024, 32),
		E_OK);
	h_eq_i64("nouveau mappage", g_fvmm.io_maps, 2);
	h_eq_i64("ancien libere", g_fvmm.io_unmaps, 1);
	h_eq_u64("generation", g_display.generation, 2);
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors du mappage");
}

int	main(void)
{
	h_begin("a13/display_modes");
	h_run("modes liste", listing);
	h_run("modes sans appareil", no_device);
	h_run("modes agrandissement", grow_mode);
	h_run("modes reduction puis agrandissement", shrink_then_grow);
	return (h_end());
}
