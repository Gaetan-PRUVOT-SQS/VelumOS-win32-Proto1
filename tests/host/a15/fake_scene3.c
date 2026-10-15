#include <stdlib.h>
#include <string.h>
#include "ref.h"

void	scene_set(t_scene *sc, int64_t x, int64_t y, t_color c)
{
	if (!scene_in(sc, x, y))
		return ;
	sc->exp[(size_t)y * (size_t)sc->w + (size_t)x] = c;
}

uint32_t	*scene_snapshot(const t_scene *sc)
{
	size_t		n;
	uint32_t	*copy;

	n = (size_t)sc->w * (size_t)sc->h;
	copy = malloc(n * sizeof(uint32_t));
	if (copy == NULL)
		abort();
	memcpy(copy, sc->exp, n * sizeof(uint32_t));
	return (copy);
}

static void	blit_pixel(t_scene *sc, const t_surface *src, t_pair p,
		t_point at)
{
	int64_t	sx;
	int64_t	sy;
	t_color	c;

	sx = at.x - p.dr.x + (int64_t)p.sr.x;
	sy = at.y - p.dr.y + (int64_t)p.sr.y;
	if (sx < 0 || sy < 0 || sx >= src->w || sy >= src->h)
		return ;
	c = src->px[sy * src->stride + sx];
	if (p.mode)
		scene_blend(sc, at.x, at.y, c);
	else
		scene_set(sc, at.x, at.y, c);
}

void	scene_blit(t_scene *sc, const t_surface *src, t_pair p)
{
	t_rect	d;
	int32_t	x;
	int32_t	y;

	d = p.dr;
	if (p.sr.w < d.w)
		d.w = p.sr.w;
	if (p.sr.h < d.h)
		d.h = p.sr.h;
	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(d, x, y))
				blit_pixel(sc, src, p, pt(x, y));
			x++;
		}
		y++;
	}
}

void	scene_scroll(t_scene *sc, t_rect r, t_point delta)
{
	uint32_t	*snap;
	int32_t		x;
	int32_t		y;

	snap = scene_snapshot(sc);
	y = 0;
	while (y < sc->h)
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(r, x, y) && in_rect(r, (int64_t)x - delta.x,
					(int64_t)y - delta.y) && (int64_t)x - delta.x < sc->w
				&& (int64_t)y - delta.y < sc->h && (int64_t)x - delta.x >= 0
				&& (int64_t)y - delta.y >= 0)
				scene_set(sc, x, y, snap[(y - (int64_t)delta.y) * sc->w
					+ (x - (int64_t)delta.x)]);
			x++;
		}
		y++;
	}
	free(snap);
}
