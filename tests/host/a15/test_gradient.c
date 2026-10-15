#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "gfx_int.h"

static void	ramp_walk(t_color from, t_color to, int64_t n, int64_t start)
{
	t_ramp	rp;
	int64_t	i;
	int		ch;

	gfx_ramp_init(&rp, from, to, n);
	gfx_ramp_seek(&rp, start);
	i = start;
	while (i < n && i < start + 300)
	{
		ch = 0;
		while (ch < 4)
		{
			h_eq_i64("ramp: pas a pas contre la formule exacte", rp.c[ch].q,
				ref_ramp((from >> (8 * ch)) & 255, (to >> (8 * ch)) & 255, n,
					i));
			ch++;
		}
		gfx_ramp_next(&rp);
		i++;
	}
}

static void	ramp_exact(void)
{
	static const int64_t	sizes[] = {1, 2, 3, 4, 7, 10, 255, 256, 257, 768,
		1024, 65539, 2147483647};
	size_t					k;
	int						r;
	t_color					a;
	t_color					b;

	k = 0;
	while (k < sizeof(sizes) / sizeof(sizes[0]))
	{
		r = 0;
		while (r < 40)
		{
			a = (t_color)rng_next();
			b = (t_color)rng_next();
			ramp_walk(a, b, sizes[k], 0);
			ramp_walk(a, b, sizes[k], sizes[k] - 1);
			ramp_walk(a, b, sizes[k], rng_range(0, sizes[k] - 1));
			r++;
		}
		k++;
	}
}

static void	grad_one(int clip, t_rect r, t_color from, t_color to)
{
	t_scene		sc;
	t_gradient	g;

	g.r = r;
	g.from = from;
	g.to = to;
	g.horizontal = (rng_next() & 1) != 0;
	scene_open(&sc, 33, 20, clip);
	gfx_gradient(&sc.s, &g);
	scene_gradient(&sc, &g);
	scene_close(&sc, "gradient: pixels egaux a la formule exacte");
}

static void	grad_random(void)
{
	int		i;
	t_rect	r;

	i = 0;
	while (i < 3000)
	{
		r = rng_rect(33);
		if (i % 2 == 0)
			r = rect_make(rng_range(-5, 30), rng_range(-5, 15),
					rng_range(1, 40), rng_range(1, 25));
		grad_one(i % CLIP_VARIANTS, r, rng_color(), rng_color());
		i++;
	}
}

int	main(void)
{
	h_begin("a15/gradient");
	rng_seed(20261005);
	h_run("ramp: DDA contre formule exacte", ramp_exact);
	h_run("gradient: partitions aleatoires contre modele", grad_random);
	return (h_end());
}
