#include "fakes.h"
#include "velum/err.h"

static void	pitch_from_virtual_width(void)
{
	t_dispi			d;
	t_dispi_geom	g;

	fake_dispi_ready(&d);
	g_fd.virt_override = 1280;
	h_eq_i64("largeur virtuelle plus grande",
		dispi_program(&d, 1024, 768, &g), E_OK);
	h_eq_u64("pas lu sur l'appareil", g.pitch, 5120);
	g_fd.virt_override = 100;
	h_eq_i64("largeur virtuelle trop petite",
		dispi_program(&d, 800, 600, &g), E_OK);
	h_eq_u64("pas minimal", g.pitch, 3200);
}

static void	virtual_width_too_big(void)
{
	t_dispi			d;
	t_dispi_geom	g;

	fake_dispi_ready(&d);
	g_fd.virt_override = 8192;
	h_eq_i64("pas qui depasse la vram", dispi_program(&d, 1024, 768, &g),
		E_NOSPC);
	h_eq_u64("mode precedent reprogramme", g_fd.regs[DISPI_INDEX_XRES], 1024);
	g_fd.virt_override = 0;
	h_eq_i64("ensuite la programmation fonctionne",
		dispi_program(&d, 800, 600, &g), E_OK);
}

static void	no_previous_mode(void)
{
	t_dispi			d;
	t_dispi_geom	g;

	fake_dispi_ready(&d);
	g_fd.regs[DISPI_INDEX_XRES] = 0;
	g_fd.regs[DISPI_INDEX_YRES] = 0;
	g_fd.reject_w = 800;
	h_eq_i64("refus sans mode precedent", dispi_program(&d, 800, 600, &g),
		E_IO);
	h_eq_u64("pas de sequence de restauration", g_fd.nlog, 5);
}

static void	mode_chain(void)
{
	t_dispi			d;
	t_dispi_geom	g;
	uint32_t		i;
	uint32_t		w[6];
	uint32_t		h[6];

	fake_dispi_ready(&d);
	i = 0;
	while (i < 6)
	{
		w[i] = (uint32_t[]){800, 800, 1920, 1280, 1024, 1280}[i];
		h[i] = (uint32_t[]){600, 600, 1080, 720, 768, 1024}[i];
		h_eq_i64("transition", dispi_program(&d, w[i], h[i], &g), E_OK);
		h_eq_u64("pas = largeur * 4", g.pitch, w[i] * 4);
		i++;
	}
	h_eq_u64("registre final largeur", g_fd.regs[DISPI_INDEX_XRES], 1280);
	h_eq_u64("registre final hauteur", g_fd.regs[DISPI_INDEX_YRES], 1024);
}

int	main(void)
{
	h_begin("a13/dispi_virt");
	h_run("virt pas lu sur la largeur virtuelle", pitch_from_virtual_width);
	h_run("virt pas trop grand", virtual_width_too_big);
	h_run("virt enchainement de modes", mode_chain);
	h_run("virt refus sans mode precedent", no_previous_mode);
	return (h_end());
}
