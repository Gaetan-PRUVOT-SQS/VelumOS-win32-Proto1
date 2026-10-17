#include "harness.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static uint32_t	mix(uint32_t dst, uint32_t src, uint32_t a)
{
	uint32_t	out;
	uint32_t	shift;
	uint32_t	d;
	uint32_t	c;

	out = 0;
	shift = 0;
	while (shift < 24)
	{
		d = (dst >> shift) & 0xffu;
		c = (src >> shift) & 0xffu;
		out |= ((d * (255 - a) + c * a + 127) / 255) << shift;
		shift += 8;
	}
	return (out);
}

static void	selection_fond_et_contour(void)
{
	t_shell		sh;
	t_rect		c;
	uint32_t	acc;
	uint32_t	fill;
	uint32_t	wall;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	c = sh.cells[0];
	acc = luna_color_accent();
	drv_click(&sh, sh.desk.id, rect_center(c));
	wall = sh.wall_px[(c.y + 40) * SCREEN_W + c.x + 3];
	h_true(px_near(&sh.desk.surface, (t_point){c.x + 3, c.y + 40},
			mix(wall, acc, 31), 2), "interieur: accent a 12 %");
	wall = sh.wall_px[(c.y + 40) * SCREEN_W + c.x];
	fill = mix(wall, acc, 31);
	h_true(px_near(&sh.desk.surface, (t_point){c.x, c.y + 40},
			mix(fill, acc, 115), 2), "bord: contour de 1,5 px");
	sh_shutdown(&sh);
}

static void	selection_bornee_et_levee(void)
{
	t_shell		sh;
	t_rect		c;
	uint32_t	wall;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	c = sh.cells[0];
	drv_click(&sh, sh.desk.id, rect_center(c));
	wall = sh.wall_px[(c.y - 3) * SCREEN_W + c.x + 45];
	h_true(px_near(&sh.desk.surface, (t_point){c.x + 45, c.y - 3}, wall, 0),
		"hors cellule: fond intact");
	drv_click(&sh, sh.desk.id, (t_point){600, 400});
	wall = sh.wall_px[(c.y + 40) * SCREEN_W + c.x];
	h_true(px_near(&sh.desk.surface, (t_point){c.x, c.y + 40}, wall, 0),
		"deselection: fond restaure");
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-desksel");
	h_run("selection: fond et contour", selection_fond_et_contour);
	h_run("selection: bornee et levee", selection_bornee_et_levee);
	return (h_end());
}
