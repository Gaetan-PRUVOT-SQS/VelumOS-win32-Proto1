#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static const t_rect	g_rects[] = {{0, 0, 7, 5}, {1, 1, 3, 2}, {-3, -2, 5, 4},
{5, 3, 9, 9}, {2, 2, 0, 3}, {2, 2, 3, -1}, {-100, -100, 1000, 1000},
{3, 1, 1, 1}, {INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX},
{INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX}, {-5, 2, INT32_MAX, 1},
{0, -9, 1, INT32_MAX}, {6, 4, 1, 1}, {7, 5, 1, 1}, {-1, -1, 1, 1}};

static void	run_fill(int32_t w, int32_t h, int clip, t_rect r)
{
	t_scene	sc;
	t_color	c;

	c = 0xff204060;
	if (clip % 3 == 1)
		c = 0x80c08040;
	scene_open(&sc, w, h, clip);
	gfx_fill(&sc.s, r, c);
	scene_rect(&sc, r, c);
	scene_close(&sc, "fill: partition rect x clip");
}

static void	fill_grid(void)
{
	static const int32_t	sizes[4][2] = {{1, 1}, {2, 2}, {7, 5}, {33, 20}};
	int						s;
	int						k;
	size_t					i;

	s = 0;
	while (s < 4)
	{
		k = 0;
		while (k < CLIP_VARIANTS)
		{
			i = 0;
			while (i < sizeof(g_rects) / sizeof(g_rects[0]))
			{
				run_fill(sizes[s][0], sizes[s][1], k, g_rects[i]);
				i++;
			}
			k++;
		}
		s++;
	}
}

static void	fill_alpha(void)
{
	static const t_color	colors[5] = {0x00ffffff, 0x01102030, 0x7f405060,
		0x80708090, 0xfe0a0b0c};
	t_scene					sc;
	int						i;

	i = 0;
	while (i < 5)
	{
		scene_open(&sc, 7, 5, 0);
		gfx_fill(&sc.s, rect_make(1, 1, 5, 3), colors[i]);
		scene_rect(&sc, rect_make(1, 1, 5, 3), colors[i]);
		scene_close(&sc, "fill: partition d'alpha");
		i++;
	}
}

static void	fill_big(void)
{
	int	k;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		run_fill(1024, 768, k, rect_make(0, 0, 1024, 768));
		run_fill(1024, 768, k, rect_make(100, 50, 600, 500));
		run_fill(1024, 768, k, rect_make(-50, -50, 2000, 2000));
		k++;
	}
}

int	main(void)
{
	h_begin("a15/fill");
	h_run("fill: grille de partitions", fill_grid);
	h_run("fill: partitions d'alpha", fill_alpha);
	h_run("fill: 1024x768", fill_big);
	return (h_end());
}
