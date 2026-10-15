#include "ref.h"

void	scene_grid(t_scene *sc, const t_grid *g, t_color c)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (grid_get(g, x, y))
				scene_blend(sc, x, y, c);
			x++;
		}
		y++;
	}
}

void	scene_exact_line(t_scene *sc, t_point a, t_point b, t_color c)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (ref_on_line(a, b, x, y))
				scene_blend(sc, x, y, c);
			x++;
		}
		y++;
	}
}

static t_color	ramp_color(t_color from, t_color to, int64_t n, int64_t i)
{
	t_color	out;
	int		ch;

	out = 0;
	ch = 0;
	while (ch < 4)
	{
		out |= (t_color)ref_ramp((from >> (8 * ch)) & 255,
				(to >> (8 * ch)) & 255, n, i) << (8 * ch);
		ch++;
	}
	return (out);
}

void	scene_gradient(t_scene *sc, const t_gradient *g)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(g->r, x, y) && g->horizontal)
				scene_blend(sc, x, y, ramp_color(g->from, g->to, g->r.w,
						(int64_t)x - g->r.x));
			else if (in_rect(g->r, x, y))
				scene_blend(sc, x, y, ramp_color(g->from, g->to, g->r.h,
						(int64_t)y - g->r.y));
			x++;
		}
		y++;
	}
}

void	scene_clear(t_scene *sc, t_color c)
{
	size_t	i;

	i = 0;
	while (i < (size_t)sc->w * (size_t)sc->h)
	{
		sc->g.px[i] = c;
		sc->exp[i] = c;
		i++;
	}
}
