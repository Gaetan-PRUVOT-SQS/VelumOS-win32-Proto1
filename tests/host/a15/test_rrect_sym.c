#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	render(t_scene *sc, int32_t rad)
{
	t_rrect	rr;

	scene_open(sc, 64, 64, 0);
	scene_clear(sc, 0xff000000);
	rr = (t_rrect){rect_make(4, 4, 56, 56), 0xffffffff, rad, rad, rad, rad};
	gfx_rrect_fill(&sc->s, &rr);
}

static void	sym_pixel(const t_scene *sc, int32_t x, int32_t y)
{
	t_color	c;

	c = scene_px(sc, 4 + x, 4 + y);
	h_eq_u64("miroir horizontal", c, scene_px(sc, 59 - x, 4 + y));
	h_eq_u64("miroir vertical", c, scene_px(sc, 4 + x, 59 - y));
	h_eq_u64("transposition", c, scene_px(sc, 4 + y, 4 + x));
}

static void	symmetric(void)
{
	static const int32_t	radii[5] = {2, 4, 6, 9, 16};
	t_scene					sc;
	int32_t					x;
	int32_t					y;
	int						i;

	i = 0;
	while (i < 5)
	{
		render(&sc, radii[i]);
		y = 0;
		while (y < 56)
		{
			x = 0;
			while (x < 56)
			{
				sym_pixel(&sc, x, y);
				x++;
			}
			y++;
		}
		scene_adopt(&sc, rect_make(0, 0, 64, 64));
		scene_close(&sc, "formes symetriques");
		i++;
	}
}

static void	monotone(void)
{
	t_scene	sc;
	int32_t	u;
	int32_t	v;
	t_color	c;

	render(&sc, 16);
	v = 0;
	while (v < 15)
	{
		u = 0;
		while (u < 15)
		{
			c = scene_px(&sc, 4 + u, 4 + v) & 255;
			h_true(c <= (scene_px(&sc, 5 + u, 4 + v) & 255),
				"couverture croissante vers l'interieur (x)");
			h_true(c <= (scene_px(&sc, 4 + u, 5 + v) & 255),
				"couverture croissante vers l'interieur (y)");
			u++;
		}
		v++;
	}
	h_eq_u64("coin extreme vide", scene_px(&sc, 4, 4) & 255, 0);
	h_eq_u64("centre plein", scene_px(&sc, 32, 32), 0xffffffff);
	scene_adopt(&sc, rect_make(0, 0, 64, 64));
	scene_close(&sc, "formes monotones");
}

int	main(void)
{
	h_begin("a15/rrect_sym");
	h_run("rrect lisse: symetries", symmetric);
	h_run("rrect lisse: monotonie", monotone);
	return (h_end());
}
