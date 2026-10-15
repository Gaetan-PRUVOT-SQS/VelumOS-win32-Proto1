#include "ref.h"

int32_t	rng_size(int32_t span)
{
	uint64_t	k;

	k = rng_next() % 16;
	if (k < 9)
		return ((int32_t)rng_range(0, span + 6));
	if (k == 9)
		return ((int32_t)rng_range(-5, 0));
	if (k == 10)
		return (INT32_MAX - (int32_t)rng_range(0, 2));
	if (k == 11)
		return (INT32_MIN + (int32_t)rng_range(0, 2));
	if (k == 12)
		return ((int32_t)rng_range(1, 70000));
	return ((int32_t)rng_range(-3, 3 * span));
}

t_rect	rng_rect(int32_t span)
{
	t_rect	r;

	r.x = rng_coord(span);
	r.y = rng_coord(span);
	r.w = rng_size(span);
	r.h = rng_size(span);
	return (r);
}

t_point	rng_point(int32_t span)
{
	t_point	p;

	p.x = rng_coord(span);
	p.y = rng_coord(span);
	return (p);
}

t_point	pt(int32_t x, int32_t y)
{
	t_point	p;

	p.x = x;
	p.y = y;
	return (p);
}
