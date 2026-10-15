#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static const t_rect		g_areas[] = {{0, 0, 9, 7}, {1, 1, 6, 4}, {-3, -2, 8, 6},
{4, 3, 20, 20}, {0, 0, 0, 3}, {2, 2, 3, -1},
{INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX}, {-5, -5, INT32_MAX, INT32_MAX},
{8, 6, 1, 1}};
static const int32_t	g_deltas[] = {0, 1, -1, 2, -3, 5, -9, 9, INT32_MAX,
	INT32_MIN};

static void	scroll_sweep(int clip, t_rect area)
{
	t_scene	sc;
	size_t	i;
	size_t	j;

	i = 0;
	while (i < sizeof(g_deltas) / sizeof(g_deltas[0]))
	{
		j = 0;
		while (j < sizeof(g_deltas) / sizeof(g_deltas[0]))
		{
			scene_open(&sc, 9, 7, clip);
			gfx_scroll(&sc.s, area, pt(g_deltas[i], g_deltas[j]));
			scene_scroll(&sc, area, pt(g_deltas[i], g_deltas[j]));
			scene_close(&sc, "scroll: partition zone x decalage x clip");
			j++;
		}
		i++;
	}
}

static void	scroll_partitions(void)
{
	int		k;
	size_t	a;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		a = 0;
		while (a < sizeof(g_areas) / sizeof(g_areas[0]))
		{
			scroll_sweep(k, g_areas[a]);
			a++;
		}
		k++;
	}
}

static void	scroll_console(void)
{
	t_scene	sc;
	int32_t	y;
	int32_t	x;

	scene_open(&sc, 8, 6, 0);
	gfx_scroll(&sc.s, rect_make(0, 0, 8, 6), pt(0, -2));
	y = 0;
	while (y < 4)
	{
		x = 0;
		while (x < 8)
		{
			h_eq_u64("une ligne remonte de deux rangees",
				scene_px(&sc, x, y), sc.exp[(y + 2) * 8 + x]);
			x++;
		}
		y++;
	}
	h_eq_u64("zone decouverte laissee telle quelle", scene_px(&sc, 3, 5),
		sc.exp[5 * 8 + 3]);
	scene_adopt(&sc, rect_make(0, 0, 8, 6));
	scene_close(&sc, "console");
}

int	main(void)
{
	h_begin("a15/scroll");
	h_run("scroll: partitions", scroll_partitions);
	h_run("scroll: defilement de console", scroll_console);
	return (h_end());
}
