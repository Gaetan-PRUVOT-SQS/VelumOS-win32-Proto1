#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static const int32_t	g_sizes[3][2] = {{1, 1}, {2, 2}, {7, 5}};

static void	put_grid(int32_t w, int32_t h, int clip, t_color c)
{
	t_scene	sc;
	int32_t	x;
	int32_t	y;

	y = -3;
	while (y < h + 3)
	{
		x = -3;
		while (x < w + 3)
		{
			scene_open(&sc, w, h, clip);
			gfx_put(&sc.s, (t_point){x, y}, c);
			scene_blend(&sc, x, y, c);
			scene_close(&sc, "put: partition de position");
			x++;
		}
		y++;
	}
}

static void	put_partitions(void)
{
	int	k;
	int	c;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		c = 0;
		while (c < 3)
		{
			put_grid(g_sizes[c][0], g_sizes[c][1], k, 0xff3366cc);
			put_grid(g_sizes[c][0], g_sizes[c][1], k, 0x80ff8000);
			put_grid(g_sizes[c][0], g_sizes[c][1], k, 0x00ffffff);
			c++;
		}
		k++;
	}
}

static void	put_extremes(void)
{
	static const int32_t	e[6] = {INT32_MIN, INT32_MIN + 1, -1, INT32_MAX - 1,
		INT32_MAX, 1 << 30};
	t_scene					sc;
	t_point					p;
	int						i;

	scene_open(&sc, 7, 5, 0);
	i = 0;
	while (i < 36)
	{
		p.x = e[i / 6];
		p.y = e[i % 6];
		gfx_put(&sc.s, p, 0xffffffff);
		i++;
	}
	scene_close(&sc, "put: coordonnees extremes sans effet");
}

static void	get_cases(void)
{
	t_scene		sc;
	int32_t		x;
	int32_t		y;
	t_color		want;

	scene_open(&sc, 7, 5, 1);
	y = -2;
	while (y < 7)
	{
		x = -2;
		while (x < 9)
		{
			want = 0;
			if (x >= 0 && x < 7 && y >= 0 && y < 5)
				want = sc.g.px[y * 7 + x];
			h_eq_u64("get: bornes de la surface, clip ignore",
				gfx_get(&sc.s, (t_point){x, y}), want);
			x++;
		}
		y++;
	}
	h_eq_u64("get extreme", gfx_get(&sc.s, (t_point){INT32_MIN, INT32_MAX}), 0);
	scene_close(&sc, "get ne modifie rien");
}

int	main(void)
{
	h_begin("a15/pixel");
	h_run("put: partitions position x clip x alpha", put_partitions);
	h_run("put: valeurs limites", put_extremes);
	h_run("get: partitions", get_cases);
	return (h_end());
}
