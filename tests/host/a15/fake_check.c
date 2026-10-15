#include "harness.h"
#include "ref.h"

void	check_empty(const char *what, const t_surface *s)
{
	h_true(s->px == NULL, what);
	h_eq_i64(what, s->w, 0);
	h_eq_i64(what, s->h, 0);
	h_eq_i64(what, s->stride, 0);
	h_true(rect_empty(s->clip), what);
}

t_color	scene_px(const t_scene *sc, int32_t x, int32_t y)
{
	return (sc->g.px[(size_t)y * (size_t)sc->w + (size_t)x]);
}

void	scene_adopt(t_scene *sc, t_rect r)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(r, x, y))
				sc->exp[(size_t)y * (size_t)sc->w + (size_t)x]
					= scene_px(sc, x, y);
			x++;
		}
		y++;
	}
}

t_gradient	grad_make(t_rect r, t_color from, t_color to, bool horizontal)
{
	t_gradient	g;

	g.r = r;
	g.from = from;
	g.to = to;
	g.horizontal = horizontal;
	return (g);
}

void	scene_copy_in(t_scene *a, const t_scene *b)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < a->h && y < b->h)
	{
		x = 0;
		while (x < a->w && x < b->w)
		{
			scene_set(a, x, y, scene_px(b, x, y));
			x++;
		}
		y++;
	}
}
