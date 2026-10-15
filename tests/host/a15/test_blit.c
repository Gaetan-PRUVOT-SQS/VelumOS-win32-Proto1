#include <stdint.h>
#include <stdlib.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static const t_rect	g_pairs[][2] = {
{{0, 0, 7, 5}, {0, 0, 7, 5}}, {{1, 1, 3, 2}, {2, 2, 3, 2}},
{{-2, -1, 5, 4}, {0, 0, 5, 4}}, {{3, 2, 9, 9}, {-3, -2, 9, 9}},
{{0, 0, 4, 4}, {0, 0, 2, 2}}, {{0, 0, 2, 2}, {0, 0, 4, 4}},
{{2, 2, 0, 3}, {0, 0, 3, 3}}, {{2, 2, 3, 3}, {0, 0, -3, 3}},
{{5, 3, 7, 5}, {4, 4, 7, 5}}, {{-100, -100, 1000, 1000}, {0, 0, 1000, 1000}},
{{0, 0, 7, 5}, {-1000, -1000, 1010, 1010}},
{{INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX}, {0, 0, 7, 5}},
{{0, 0, 7, 5}, {INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX}},
{{INT32_MAX, INT32_MAX, 5, 5}, {INT32_MAX, INT32_MAX, 5, 5}},
{{-3, -3, INT32_MAX, INT32_MAX}, {-5, -5, INT32_MAX, INT32_MAX}},
{{6, 4, 1, 1}, {0, 0, 1, 1}}, {{7, 5, 1, 1}, {0, 0, 1, 1}},
{{0, 0, 3, 3}, {7, 5, 3, 3}}};

static void	blit_pair(int32_t dw, int32_t sw, int clip, t_pair p)
{
	t_scene	dst;
	t_scene	src;
	t_blit	b;

	scene_open(&dst, dw, dw, clip);
	scene_open(&src, sw, sw, 0);
	b = (t_blit){&dst.s, &src.s, p.dr, p.sr};
	if (p.mode)
		gfx_blit_alpha(&b);
	else
		gfx_blit(&b);
	scene_blit(&dst, &src.s, p);
	scene_close(&dst, "blit: partition dst x src x rects x clip");
	scene_close(&src, "blit: la source n'est pas modifiee");
}

static void	blit_sweep(int32_t dw, int32_t sw)
{
	int		k;
	size_t	i;
	t_pair	p;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		i = 0;
		while (i < sizeof(g_pairs) / sizeof(g_pairs[0]))
		{
			p = (t_pair){g_pairs[i][0], g_pairs[i][1], (int)(i + k) % 2};
			blit_pair(dw, sw, k, p);
			i++;
		}
		k++;
	}
}

static void	blit_partitions(void)
{
	static const int32_t	dims[3] = {1, 2, 7};
	int						a;
	int						b;

	a = 0;
	while (a < 3)
	{
		b = 0;
		while (b < 3)
		{
			blit_sweep(dims[a], dims[b]);
			b++;
		}
		a++;
	}
}

static void	blit_random(void)
{
	int		i;
	t_pair	p;

	i = 0;
	while (i < 4000)
	{
		p.dr = rng_rect(7);
		p.sr = rng_rect(7);
		if (i % 2 == 0)
		{
			p.dr = rect_make(rng_range(-4, 9), rng_range(-4, 9),
					rng_range(0, 9), rng_range(0, 9));
			p.sr = rect_make(rng_range(-4, 9), rng_range(-4, 9),
					rng_range(0, 9), rng_range(0, 9));
		}
		p.mode = i % 3 == 0;
		blit_pair(7, 7, i % CLIP_VARIANTS, p);
		i++;
	}
}

int	main(void)
{
	h_begin("a15/blit");
	rng_seed(20261005);
	h_run("blit et blit_alpha: partitions", blit_partitions);
	h_run("blit et blit_alpha: aleatoire contre modele", blit_random);
	return (h_end());
}
