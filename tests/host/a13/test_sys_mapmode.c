#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"
#include "velum/vmm.h"

static void	old_mapping_survives_mode_change(void)
{
	int64_t	va;

	fx_display(1024, 768);
	va = fx_map(0);
	h_eq_i64("changement de mode", display_set_mode(1920, 1080, 32), E_OK);
	h_eq_i64("ancien mappage toujours present", fake_vmm_live_maps(), 1);
	h_eq_u64("ancien mappage : meme longueur", fx_live_map(0)->len, 3145728);
	h_eq_u64("aucun demappage encore", g_fvmm.unmaps, 0);
	h_true(va > 0, "adresse valide");
}

static void	map_after_mode_change(void)
{
	int64_t	va;
	int64_t	again;

	fx_display(1024, 768);
	fx_map(0);
	display_set_mode(1920, 1080, 32);
	va = fx_map(0);
	h_true(va > 0, "nouveau mappage");
	h_eq_i64("l'ancien est demappe", fake_vmm_live_maps(), 1);
	h_eq_u64("un demappage", g_fvmm.unmaps, 1);
	h_eq_u64("longueur du nouveau mode", fx_live_map(0)->len, 8294400);
	again = fx_map(0);
	h_eq_i64("appel suivant : meme adresse", again, va);
	display_set_mode(800, 600, 32);
	h_true(fx_map(0) > 0, "mode reduit");
	h_eq_u64("longueur arrondie a la page", fx_live_map(0)->len, 1921024);
	h_eq_i64("toujours un seul mappage", fake_vmm_live_maps(), 1);
}

static void	same_mode_keeps_mapping(void)
{
	int64_t	va;

	fx_display(1024, 768);
	va = fx_map(0);
	h_eq_i64("meme mode", display_set_mode(1024, 768, 32), E_OK);
	h_eq_i64("meme adresse", fx_map(0), va);
	h_eq_u64("pas de second vmm_map", g_fvmm.maps, 1);
	h_eq_i64("echec du changement de mode",
		display_set_mode(641, 480, 32), E_INVAL);
	h_eq_i64("toujours la meme adresse", fx_map(0), va);
}

static void	failed_switch_keeps_mapping(void)
{
	int64_t	va;

	fx_display(1024, 768);
	va = fx_map(0);
	g_fd.reject_w = 1280;
	h_eq_i64("refus de l'appareil", display_set_mode(1280, 720, 32), E_IO);
	h_eq_i64("meme adresse apres echec", fx_map(0), va);
	h_eq_u64("aucun nouveau mappage", g_fvmm.maps, 1);
	h_eq_u64("aucun demappage", g_fvmm.unmaps, 0);
}

int	main(void)
{
	h_begin("a13/sys_mapmode");
	h_run("mapmode ancien mappage valable", old_mapping_survives_mode_change);
	h_run("mapmode map apres changement", map_after_mode_change);
	h_run("mapmode meme mode", same_mode_keeps_mapping);
	h_run("mapmode changement refuse", failed_switch_keeps_mapping);
	return (h_end());
}
