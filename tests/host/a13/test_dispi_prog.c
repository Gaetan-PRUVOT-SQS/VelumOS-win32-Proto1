#include "fakes.h"
#include "velum/err.h"

static void	nominal_sequence(void)
{
	t_dispi			d;
	t_dispi_geom	g;

	fake_dispi_ready(&d);
	h_eq_i64("programmation 800x600", dispi_program(&d, 800, 600, &g), E_OK);
	h_eq_u64("largeur", g.width, 800);
	h_eq_u64("hauteur", g.height, 600);
	h_eq_u64("pas", g.pitch, 3200);
	h_eq_u64("cinq ecritures", g_fd.nlog, 5);
	h_true(fd_wrote(0, DISPI_INDEX_ENABLE, DISPI_OFF), "1 : desactiver");
	h_true(fd_wrote(1, DISPI_INDEX_XRES, 800), "2 : largeur");
	h_true(fd_wrote(2, DISPI_INDEX_YRES, 600), "3 : hauteur");
	h_true(fd_wrote(3, DISPI_INDEX_BPP, 32), "4 : 32 bits");
	h_true(fd_wrote(4, DISPI_INDEX_ENABLE, DISPI_ON), "5 : activer avec LFB");
}

static void	rollback_on_refusal(void)
{
	t_dispi			d;
	t_dispi_geom	g;

	fake_dispi_ready(&d);
	g_fd.reject_w = 1280;
	h_eq_i64("refus de l'appareil", dispi_program(&d, 1280, 720, &g), E_IO);
	h_eq_u64("largeur restauree", g_fd.regs[DISPI_INDEX_XRES], 1024);
	h_eq_u64("hauteur restauree", g_fd.regs[DISPI_INDEX_YRES], 768);
	h_eq_u64("mode reactive", g_fd.regs[DISPI_INDEX_ENABLE], DISPI_ON);
	h_eq_u64("deux sequences ecrites", g_fd.nlog, 10);
	h_true(fd_wrote(9, DISPI_INDEX_ENABLE, DISPI_ON), "derniere ecriture");
}

static void	refused_without_write(void)
{
	t_dispi			d;
	t_dispi_geom	g;

	fake_dispi_ready(&d);
	h_eq_i64("mode hors liste", dispi_program(&d, 640, 480, &g), E_INVAL);
	h_eq_i64("mode nul", dispi_program(&d, 0, 0, &g), E_INVAL);
	h_eq_u64("aucune ecriture de registre", g_fd.nlog, 0);
	d.ready = false;
	h_eq_i64("appareil non pret", dispi_program(&d, 800, 600, &g), E_NODEV);
	h_eq_u64("toujours aucune ecriture", g_fd.nlog, 0);
}

static void	device_clamps_height(void)
{
	t_dispi			d;
	t_dispi_geom	g;

	fake_dispi_ready(&d);
	g_fd.clamp_height = true;
	h_eq_i64("hauteur modifiee par l'appareil",
		dispi_program(&d, 1280, 720, &g), E_IO);
	g_fd.clamp_height = false;
	g_fd.reject_enable = true;
	h_eq_i64("activation ignoree", dispi_program(&d, 800, 600, &g), E_IO);
}

int	main(void)
{
	h_begin("a13/dispi_prog");
	h_run("prog sequence nominale", nominal_sequence);
	h_run("prog retour arriere apres refus", rollback_on_refusal);
	h_run("prog refus sans ecriture", refused_without_write);
	h_run("prog appareil qui corrige la valeur", device_clamps_height);
	return (h_end());
}
