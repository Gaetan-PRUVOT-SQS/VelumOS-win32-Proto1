#include "gfx_int.h"

static uint32_t	sample_inside(int64_t radius, int64_t u, int64_t v)
{
	int64_t		k;
	int64_t		l;
	int64_t		dx;
	int64_t		dy;
	uint32_t	n;

	n = 0;
	k = 0;
	while (k < GFX_COVER_SIDE)
	{
		l = 0;
		dx = 2 * GFX_COVER_SIDE * (u - radius) + 2 * k + 1;
		while (l < GFX_COVER_SIDE)
		{
			dy = 2 * GFX_COVER_SIDE * (v - radius) + 2 * l + 1;
			if (dx * dx + dy * dy <= 4 * GFX_COVER_SIDE * GFX_COVER_SIDE
				* radius * radius)
				n++;
			l++;
		}
		k++;
	}
	return (n);
}

static uint32_t	hard_cover(int64_t radius, int64_t u, int64_t v)
{
	int64_t	dx;
	int64_t	dy;

	dx = 2 * (radius - u) - 1;
	dy = 2 * (radius - v) - 1;
	if (dx * dx + dy * dy <= 4 * radius * radius)
		return (255);
	return (0);
}

uint32_t	gfx_corner_cover(int64_t radius, int64_t u, int64_t v)
{
	int64_t		far;
	int64_t		near;
	uint32_t	n;

	if (radius <= 3)
		return (hard_cover(radius, u, v));
	far = (radius - u) * (radius - u) + (radius - v) * (radius - v);
	near = (radius - u - 1) * (radius - u - 1)
		+ (radius - v - 1) * (radius - v - 1);
	if (far <= radius * radius)
		return (255);
	if (near >= radius * radius)
		return (0);
	n = sample_inside(radius, u, v);
	return ((n * 255 + GFX_COVER_SIDE * GFX_COVER_SIDE / 2)
		/ (GFX_COVER_SIDE * GFX_COVER_SIDE));
}
