#include "layout.h"

t_rect	lay_rect(int32_t x, int32_t y, int32_t w, int32_t h)
{
	t_rect	r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

int	lay_inside(t_rect outer, t_rect r)
{
	if (r.w <= 0 || r.h <= 0)
		return (0);
	return (r.x >= outer.x && r.y >= outer.y
		&& (int64_t)r.x + r.w <= (int64_t)outer.x + outer.w
		&& (int64_t)r.y + r.h <= (int64_t)outer.y + outer.h);
}

int	lay_overlap(t_rect a, t_rect b)
{
	if (a.w <= 0 || a.h <= 0 || b.w <= 0 || b.h <= 0)
		return (0);
	return ((int64_t)a.x < (int64_t)b.x + b.w
		&& (int64_t)b.x < (int64_t)a.x + a.w
		&& (int64_t)a.y < (int64_t)b.y + b.h
		&& (int64_t)b.y < (int64_t)a.y + a.h);
}

int	lay_contains(t_rect r, int32_t x, int32_t y)
{
	if (r.w <= 0 || r.h <= 0)
		return (0);
	return (x >= r.x && y >= r.y
		&& (int64_t)x < (int64_t)r.x + r.w
		&& (int64_t)y < (int64_t)r.y + r.h);
}

int32_t	lay_clamp(int32_t v, int32_t lo, int32_t hi)
{
	if (v < lo)
		return (lo);
	if (v > hi)
		return (hi);
	return (v);
}
