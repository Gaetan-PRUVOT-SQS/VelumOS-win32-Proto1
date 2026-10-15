#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static const t_rect	g_boxes[] = {{0, 0, 1, 1}, {0, 0, 7, 5}, {1, 1, 2, 2},
{1, 1, 1, 4}, {2, 1, 5, 1}, {-2, -2, 6, 5}, {3, 2, 9, 9}, {-1, -1, 9, 7},
{0, 0, 2, 2}, {2, 0, 3, 3}, {INT32_MIN, 0, INT32_MAX, 5},
{0, 2, INT32_MAX, INT32_MAX}, {-5, -5, INT32_MAX, INT32_MAX}, {4, 3, 0, 0},
{4, 3, -2, 5}, {INT32_MAX, INT32_MAX, 2, 2}};

static void	frame_model(t_scene *sc, t_rect r, t_color c)
{
	int32_t	x;
	int32_t	y;

	gfx_frame(&sc->s, r, c);
	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(r, x, y) && (x == r.x || y == r.y
					|| x == (int64_t)r.x + r.w - 1
					|| y == (int64_t)r.y + r.h - 1))
				scene_blend(sc, x, y, c);
			x++;
		}
		y++;
	}
}

static void	frame_cases(void)
{
	t_scene	sc;
	size_t	i;
	int		k;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		i = 0;
		while (i < sizeof(g_boxes) / sizeof(g_boxes[0]))
		{
			scene_open(&sc, 7, 5, k);
			frame_model(&sc, g_boxes[i], 0x80ff0000 + (uint32_t)i);
			scene_close(&sc, "frame: contour 1 pixel sans double trace");
			i++;
		}
		k++;
	}
}

int	main(void)
{
	h_begin("a15/frame");
	h_run("frame: tailles et clips", frame_cases);
	return (h_end());
}
