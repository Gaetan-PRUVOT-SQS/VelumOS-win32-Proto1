#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	edges_vertical(void)
{
	t_scene		sc;
	t_gradient	g;
	int			x;

	scene_open(&sc, 7, 5, 0);
	g = grad_make(rect_make(0, 0, 7, 5), 0xff102030, 0xffa0b0c0, false);
	gfx_gradient(&sc.s, &g);
	x = 0;
	while (x < 7)
	{
		h_eq_u64("bord haut = from", scene_px(&sc, x, 0), 0xff102030);
		h_eq_u64("bord bas = to", scene_px(&sc, x, 4), 0xffa0b0c0);
		x++;
	}
	scene_adopt(&sc, rect_make(0, 0, 7, 5));
	scene_close(&sc, "bords exacts verticaux");
}

static void	edges_horizontal(void)
{
	t_scene		sc;
	t_gradient	g;
	int			y;

	scene_open(&sc, 7, 5, 0);
	g = grad_make(rect_make(0, 0, 7, 5), 0xff102030, 0xffa0b0c0, true);
	gfx_gradient(&sc.s, &g);
	y = 0;
	while (y < 5)
	{
		h_eq_u64("bord gauche = from", scene_px(&sc, 0, y), 0xff102030);
		h_eq_u64("bord droit = to", scene_px(&sc, 6, y), 0xffa0b0c0);
		y++;
	}
	scene_adopt(&sc, rect_make(0, 0, 7, 5));
	scene_close(&sc, "bords exacts horizontaux");
}

static void	monotone_gray(void)
{
	t_scene		sc;
	t_gradient	g;
	int			y;

	scene_open(&sc, 8, 768, 0);
	g = grad_make(rect_make(0, 0, 8, 768), 0xff000000, 0xffffffff, false);
	gfx_gradient(&sc.s, &g);
	y = 0;
	while (y < 767)
	{
		h_true((scene_px(&sc, 0, y) & 255) <= (scene_px(&sc, 0, y + 1) & 255),
			"gris croissant");
		y++;
	}
	h_eq_u64("haut noir", scene_px(&sc, 7, 0), 0xff000000);
	h_eq_u64("bas blanc", scene_px(&sc, 7, 767), 0xffffffff);
	scene_adopt(&sc, rect_make(0, 0, 8, 768));
	scene_close(&sc, "rampe de gris");
}

int	main(void)
{
	h_begin("a15/gradient_edges");
	h_run("gradient vertical: bords exacts from puis to", edges_vertical);
	h_run("gradient horizontal: bords exacts from puis to", edges_horizontal);
	h_run("gradient: rampe de gris monotone sur 768", monotone_gray);
	return (h_end());
}
