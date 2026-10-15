#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	smooth_constant(void)
{
	t_scene	dst;
	t_scene	src;
	size_t	i;
	t_rect	inner;

	inner = rect_make(1, 1, 21, 15);
	scene_open(&src, 5, 5, 0);
	scene_clear(&src, 0x80abcdef);
	scene_open(&dst, 23, 17, 0);
	run_smooth(&dst, &src, rect_make(1, 1, 21, 15), rect_make(0, 0, 5, 5));
	i = 0;
	while (i < 23 * 17)
	{
		if (in_rect(inner, (int64_t)(i % 23), (int64_t)(i / 23)))
			h_eq_u64("source constante: couleur exacte", dst.g.px[i],
				0x80abcdef);
		i++;
	}
	scene_adopt(&dst, rect_make(0, 0, 23, 17));
	scene_close(&dst, "constante");
	scene_close(&src, "constante source");
}

static void	smooth_identity(void)
{
	t_scene	dst;
	t_scene	src;

	scene_open(&src, 9, 9, 0);
	scene_open(&dst, 9, 9, 0);
	run_smooth(&dst, &src, rect_make(0, 0, 9, 9), rect_make(0, 0, 9, 9));
	h_eq_u64("1:1 egale la copie", img_diff(dst.g.px, src.g.px, 81), 81);
	scene_adopt(&dst, rect_make(0, 0, 9, 9));
	scene_close(&dst, "1:1");
	scene_close(&src, "1:1 source");
}

static void	smooth_random(void)
{
	int		i;
	t_pair	p;

	i = 0;
	while (i < 3000)
	{
		p.dr = rect_make(rng_range(-4, 10), rng_range(-4, 10),
				rng_range(-1, 14), rng_range(-1, 14));
		p.sr = rect_make(rng_range(-4, 8), rng_range(-4, 8),
				rng_range(-1, 12), rng_range(-1, 12));
		if (i % 6 == 0)
		{
			p.dr = rng_rect(7);
			p.sr = rng_rect(7);
		}
		p.mode = 1;
		scale_case(7, 7, i % CLIP_VARIANTS, p);
		i++;
	}
}

static void	smooth_monotone(void)
{
	t_scene	dst;
	t_scene	src;
	int		x;

	scene_open(&src, 2, 2, 0);
	scene_open(&dst, 64, 64, 0);
	src.g.px[0] = 0xff000000;
	src.g.px[1] = 0xffffffff;
	run_smooth(&dst, &src, rect_make(0, 0, 64, 1), rect_make(0, 0, 2, 1));
	x = 0;
	while (x < 63)
	{
		h_true((dst.g.px[x] & 255) <= (dst.g.px[x + 1] & 255), "monotone");
		x++;
	}
	h_eq_u64("bord gauche", dst.g.px[0], 0xff000000);
	h_eq_u64("bord droit", dst.g.px[63], 0xffffffff);
	scene_adopt(&dst, rect_make(0, 0, 64, 64));
	scene_adopt(&src, rect_make(0, 0, 2, 2));
	scene_close(&dst, "monotone");
	scene_close(&src, "monotone source");
}

int	main(void)
{
	h_begin("a15/smooth");
	rng_seed(20261005);
	h_run("smooth: source constante", smooth_constant);
	h_run("smooth: 1:1", smooth_identity);
	h_run("smooth: rampe monotone", smooth_monotone);
	h_run("smooth: aleatoire contre modele", smooth_random);
	return (h_end());
}
