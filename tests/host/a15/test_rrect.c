#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static const int32_t	g_sizes[8][2] = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 3},
{7, 5}, {7, 7}, {6, 2}};
static const int32_t	g_radii[8][4] = {{0, 0, 0, 0}, {1, 1, 1, 1},
{2, 2, 2, 2}, {3, 3, 3, 3}, {3, 0, 2, 1}, {100, 100, 100, 100},
{-5, 2, 7, 0}, {INT32_MAX, 1, INT32_MIN, 3}};

static void	hard_model(t_scene *sc, t_rect r, const int32_t cap[4])
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(r, x, y) && hard_inside(r, cap, x, y))
				scene_blend(sc, x, y, 0xff5090c0);
			x++;
		}
		y++;
	}
}

static void	hard_one(t_rect r, const int32_t rad[4], int clip)
{
	t_scene	sc;
	t_rrect	rr;
	int32_t	cap[4];
	int		i;

	rr = (t_rrect){r, 0xff5090c0, rad[0], rad[1], rad[2], rad[3]};
	scene_open(&sc, 12, 12, clip);
	gfx_rrect_fill(&sc.s, &rr);
	i = 0;
	while (i < 4)
	{
		cap[i] = ref_cap(rad[i], r);
		i++;
	}
	hard_model(&sc, r, cap);
	scene_close(&sc, "rrect: bords nets (rayon <= 3) contre le modele");
}

static void	hard_places(int s, int q)
{
	int	k;
	int	o;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		o = 0;
		while (o < 3)
		{
			hard_one(rect_make(o * 3 - 1, o * 2, g_sizes[s][0],
					g_sizes[s][1]), g_radii[q], k);
			o++;
		}
		k++;
	}
}

static void	hard_edges(void)
{
	int	s;
	int	q;

	s = 0;
	while (s < 8)
	{
		q = 0;
		while (q < 8)
		{
			hard_places(s, q);
			q++;
		}
		s++;
	}
}

int	main(void)
{
	h_begin("a15/rrect");
	rng_seed(20261005);
	h_run("rrect: bords nets, rayons et plafond", hard_edges);
	return (h_end());
}
