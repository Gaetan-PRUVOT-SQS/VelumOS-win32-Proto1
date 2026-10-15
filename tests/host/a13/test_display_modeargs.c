#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	same_mode_is_free(void)
{
	fx_display(1024, 768);
	h_eq_i64("meme mode", display_set_mode(1024, 768, 32), E_OK);
	h_eq_u64("aucune ecriture de registre", g_fd.nlog, 0);
	h_eq_i64("pas de sonde de l'appareil", g_fd.open_calls, 0);
	h_eq_u64("generation inchangee", g_display.generation, 0);
}

static void	invalid_arguments(void)
{
	fx_display(1024, 768);
	h_eq_i64("bpp 24", display_set_mode(800, 600, 24), E_INVAL);
	h_eq_i64("bpp 0", display_set_mode(800, 600, 0), E_INVAL);
	h_eq_i64("bpp 33", display_set_mode(800, 600, 33), E_INVAL);
	h_eq_i64("bpp max", display_set_mode(800, 600, UINT32_MAX), E_INVAL);
	h_eq_i64("mode hors liste", display_set_mode(640, 480, 32), E_INVAL);
	h_eq_i64("mode nul", display_set_mode(0, 0, 32), E_INVAL);
	h_eq_i64("valeurs maximales", display_set_mode(UINT32_MAX, UINT32_MAX, 32),
		E_INVAL);
	h_eq_u64("seule ecriture : l'identifiant de detection", g_fd.nlog, 1);
	h_true(fd_wrote(0, DISPI_INDEX_ID, DISPI_ID_WRITE), "ecriture de l'ID");
	h_eq_u64("mode inchange", display_info()->width, 1024);
}

static void	no_display(void)
{
	t_dispmode	m[4];

	fake_reset();
	display_boot_init();
	h_eq_i64("set_mode", display_set_mode(800, 600, 32), E_NODEV);
	h_eq_u64("modes sans affichage", display_modes(m, 4), 0);
	h_eq_i64("bpp nul", display_set_mode(1, 1, 0), E_NODEV);
}

int	main(void)
{
	h_begin("a13/display_modeargs");
	h_run("set_mode meme mode sans effet", same_mode_is_free);
	h_run("set_mode arguments invalides", invalid_arguments);
	h_run("set_mode sans affichage", no_display);
	return (h_end());
}
