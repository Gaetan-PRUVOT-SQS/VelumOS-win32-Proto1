#include "ref.h"

static bool	scale_ok(const t_pair *p)
{
	return (p->dr.w > 0 && p->dr.h > 0 && p->sr.w > 0 && p->sr.h > 0);
}

static void	scaled_pixel(t_scene *sc, const t_surface *src, t_pair p,
		t_point at)
{
	int64_t	sx;
	int64_t	sy;

	sx = p.sr.x + ((2 * ((int64_t)at.x - p.dr.x) + 1) * p.sr.w)
		/ (2 * (int64_t)p.dr.w);
	sy = p.sr.y + ((2 * ((int64_t)at.y - p.dr.y) + 1) * p.sr.h)
		/ (2 * (int64_t)p.dr.h);
	if (sx >= 0 && sy >= 0 && sx < src->w && sy < src->h)
		scene_set(sc, at.x, at.y, src->px[sy * src->stride + sx]);
}

void	scene_scaled(t_scene *sc, const t_surface *src, t_pair p)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < sc->h && scale_ok(&p))
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(p.dr, x, y))
				scaled_pixel(sc, src, p, pt(x, y));
			x++;
		}
		y++;
	}
}
