#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	one_vs_ref(t_point a, t_point b, int clip, t_color c)
{
	t_scene	sc;
	t_grid	*g;

	scene_open(&sc, 33, 20, clip);
	g = grid_new(33, 20);
	ref_line(g, a, b);
	gfx_line(&sc.s, a, b, c);
	scene_grid(&sc, g, c);
	scene_close(&sc, "line: pixels egaux a l'algorithme de reference");
	grid_free(g);
}

static void	line_vs_reference(void)
{
	int		i;
	t_point	a;
	t_point	b;

	i = 0;
	while (i < 4000)
	{
		a = pt((int32_t)rng_range(-15, 50), (int32_t)rng_range(-15, 35));
		b = pt((int32_t)rng_range(-15, 50), (int32_t)rng_range(-15, 35));
		one_vs_ref(a, b, i % CLIP_VARIANTS, 0xff112233);
		one_vs_ref(a, b, i % CLIP_VARIANTS, 0x80ff00ff);
		i++;
	}
	one_vs_ref(pt(-100000, -77777), pt(90001, 66666), 0, 0xffffffff);
	one_vs_ref(pt(5, -90000), pt(28, 99999), 2, 0xffffffff);
}

static void	one_exact(t_point a, t_point b, int clip)
{
	t_scene	sc;

	scene_open(&sc, 7, 5, clip);
	gfx_line(&sc.s, a, b, 0xffa0b0c0);
	scene_exact_line(&sc, a, b, 0xffa0b0c0);
	scene_close(&sc, "line: coordonnees extremes contre forme close 128 bits");
}

static void	line_extremes(void)
{
	int		i;
	t_point	a;
	t_point	b;

	i = 0;
	while (i < 20000)
	{
		a = rng_point(7);
		b = rng_point(7);
		one_exact(a, b, i % CLIP_VARIANTS);
		i++;
	}
	one_exact(pt(INT32_MIN, INT32_MIN), pt(INT32_MAX, INT32_MAX), 0);
	one_exact(pt(INT32_MIN, 0), pt(INT32_MAX, 4), 0);
	one_exact(pt(INT32_MAX, 4), pt(INT32_MIN, 0), 0);
	one_exact(pt(0, INT32_MIN), pt(6, INT32_MAX), 0);
	one_exact(pt(INT32_MIN, INT32_MAX), pt(INT32_MAX, INT32_MIN), 0);
}

int	main(void)
{
	h_begin("a15/line");
	rng_seed(20261005);
	h_run("line: partitions contre Bresenham de reference", line_vs_reference);
	h_run("line: valeurs limites contre la forme close", line_extremes);
	return (h_end());
}
