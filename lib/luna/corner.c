#include "luna_int.h"

#define CORNER_RMAX 16777216

static int32_t	corner_in(int64_t r8, int64_t x, int64_t y)
{
	int64_t	dx;
	int64_t	dy;

	dx = r8 - x;
	dy = r8 - y;
	return (dx * dx + dy * dy <= r8 * r8);
}

int32_t	lp_corner_cov(int32_t r, int32_t ix, int32_t iy)
{
	int32_t	n;
	int32_t	sx;
	int32_t	sy;
	int64_t	x;
	int64_t	y;

	r = lp_clamp(r, 0, CORNER_RMAX);
	ix = lp_clamp(ix, -1, 2 * r + 1);
	iy = lp_clamp(iy, -1, 2 * r + 1);
	x = (int64_t)ix * 8 + 1;
	y = (int64_t)iy * 8 + 1;
	n = 0;
	sy = 0;
	while (sy < 4)
	{
		sx = 0;
		while (sx < 4)
		{
			n += corner_in((int64_t)r * 8, x + sx * 2, y + sy * 2);
			sx++;
		}
		sy++;
	}
	return (n);
}
