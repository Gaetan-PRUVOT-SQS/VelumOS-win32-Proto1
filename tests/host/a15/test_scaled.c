#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	upscale_known(void)
{
	uint32_t	src[4];
	uint32_t	dst[16];
	t_surface	s;
	t_surface	d;
	t_blit		b;

	src[0] = 0xff000001;
	src[1] = 0xff000002;
	src[2] = 0xff000003;
	src[3] = 0xff000004;
	gfx_surface_init(&s, src, 2, 2);
	gfx_surface_init(&d, dst, 4, 4);
	b = (t_blit){&d, &s, rect_make(0, 0, 4, 4), rect_make(0, 0, 2, 2)};
	gfx_blit_scaled(&b);
	h_eq_u64("2x: (0,0)", dst[0], 0xff000001);
	h_eq_u64("2x: (1,1)", dst[5], 0xff000001);
	h_eq_u64("2x: (2,1)", dst[6], 0xff000002);
	h_eq_u64("2x: (1,2)", dst[9], 0xff000003);
	h_eq_u64("2x: (3,3)", dst[15], 0xff000004);
	h_eq_u64("2x: (2,2)", dst[10], 0xff000004);
}

static void	downscale_known(void)
{
	uint32_t	src[4];
	uint32_t	dst[2];
	t_surface	s;
	t_surface	d;
	t_blit		b;

	src[0] = 10;
	src[1] = 11;
	src[2] = 12;
	src[3] = 13;
	gfx_surface_init(&s, src, 4, 1);
	gfx_surface_init(&d, dst, 2, 1);
	b = (t_blit){&d, &s, rect_make(0, 0, 2, 1), rect_make(0, 0, 4, 1)};
	gfx_blit_scaled(&b);
	h_eq_u64("4 vers 2: colonne 1", dst[0], 11);
	h_eq_u64("4 vers 2: colonne 3", dst[1], 13);
	gfx_surface_init(&d, dst, 2, 1);
	b.sr = rect_make(0, 0, 3, 1);
	gfx_blit_scaled(&b);
	h_eq_u64("3 vers 2: colonne 0", dst[0], 10);
	h_eq_u64("3 vers 2: colonne 2", dst[1], 12);
}

static void	scaled_random(void)
{
	int		i;
	t_pair	p;

	i = 0;
	while (i < 4000)
	{
		p.dr = rect_make(rng_range(-4, 10), rng_range(-4, 10),
				rng_range(-1, 12), rng_range(-1, 12));
		p.sr = rect_make(rng_range(-4, 10), rng_range(-4, 10),
				rng_range(-1, 12), rng_range(-1, 12));
		if (i % 5 == 0)
		{
			p.dr = rng_rect(7);
			p.sr = rng_rect(7);
		}
		p.mode = 0;
		scale_case(7, 7, i % CLIP_VARIANTS, p);
		i++;
	}
}

static void	scaled_identity(void)
{
	t_scene	dst;
	t_scene	src;
	t_blit	b;

	scene_open(&dst, 7, 7, 0);
	scene_open(&src, 7, 7, 0);
	b = (t_blit){&dst.s, &src.s, rect_make(0, 0, 7, 7), rect_make(0, 0, 7, 7)};
	gfx_blit_scaled(&b);
	h_eq_u64("echelle 1:1 egale la copie", img_diff(dst.g.px, src.g.px, 49),
		49);
	scene_adopt(&dst, rect_make(0, 0, 7, 7));
	scene_close(&dst, "echelle 1:1");
	scene_close(&src, "source");
}

int	main(void)
{
	h_begin("a15/scaled");
	rng_seed(20261005);
	h_run("scaled: agrandissement x2 exact", upscale_known);
	h_run("scaled: reduction exacte", downscale_known);
	h_run("scaled: 1:1", scaled_identity);
	h_run("scaled: aleatoire contre modele", scaled_random);
	return (h_end());
}
