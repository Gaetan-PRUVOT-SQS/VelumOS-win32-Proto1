#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "ref.h"

void	scene_open(t_scene *sc, int32_t w, int32_t h, int clip)
{
	size_t	n;

	n = (size_t)w * (size_t)h;
	guard_new(&sc->g, n);
	img_pattern(sc->g.px, n, (uint32_t)(w * 31 + h * 17 + clip));
	sc->exp = malloc(n * sizeof(uint32_t));
	if (sc->exp == NULL)
		abort();
	memcpy(sc->exp, sc->g.px, n * sizeof(uint32_t));
	sc->w = w;
	sc->h = h;
	sc->cv = clip_variant(clip, w, h);
	gfx_surface_init(&sc->s, sc->g.px, w, h);
	gfx_set_clip(&sc->s, sc->cv);
}

bool	scene_in(const t_scene *sc, int64_t x, int64_t y)
{
	if (x < 0 || y < 0 || x >= sc->w || y >= sc->h)
		return (false);
	return (in_rect(sc->cv, x, y));
}

void	scene_blend(t_scene *sc, int64_t x, int64_t y, t_color c)
{
	size_t	i;

	if (!scene_in(sc, x, y))
		return ;
	i = (size_t)y * (size_t)sc->w + (size_t)x;
	sc->exp[i] = ref_blend(sc->exp[i], c);
}

void	scene_close(t_scene *sc, const char *what)
{
	size_t	n;
	size_t	at;

	n = (size_t)sc->w * (size_t)sc->h;
	at = img_diff(sc->g.px, sc->exp, n);
	h_true(at == n, what);
	if (at != n)
		fprintf(stderr, "  %dx%d clip(%d,%d,%d,%d) pixel %zu: obtenu %08x "
			"attendu %08x\n", sc->w, sc->h, sc->cv.x, sc->cv.y, sc->cv.w,
			sc->cv.h, at, sc->g.px[at], sc->exp[at]);
	h_true(guard_ok(&sc->g), "zones de garde intactes");
	free(sc->exp);
	guard_free(&sc->g);
}

void	scene_rect(t_scene *sc, t_rect r, t_color c)
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
				scene_blend(sc, x, y, c);
			x++;
		}
		y++;
	}
}
