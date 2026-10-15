#include "fake.h"

t_rect	rect_make(int32_t x, int32_t y, int32_t w, int32_t h)
{
	t_rect	r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

bool	rect_empty(t_rect r)
{
	return (r.w <= 0 || r.h <= 0);
}

bool	rect_contains(t_rect r, t_point p)
{
	return (p.x >= r.x && p.y >= r.y && p.x < r.x + r.w && p.y < r.y + r.h);
}

t_rect	rect_intersect(t_rect a, t_rect b)
{
	int32_t	x0;
	int32_t	y0;
	int32_t	x1;
	int32_t	y1;

	x0 = a.x;
	if (b.x > x0)
		x0 = b.x;
	y0 = a.y;
	if (b.y > y0)
		y0 = b.y;
	x1 = a.x + a.w;
	if (b.x + b.w < x1)
		x1 = b.x + b.w;
	y1 = a.y + a.h;
	if (b.y + b.h < y1)
		y1 = b.y + b.h;
	if (x1 <= x0 || y1 <= y0)
		return (rect_make(0, 0, 0, 0));
	return (rect_make(x0, y0, x1 - x0, y1 - y0));
}

t_rect	rect_union(t_rect a, t_rect b)
{
	int32_t	x0;
	int32_t	y0;
	int32_t	x1;
	int32_t	y1;

	if (rect_empty(a))
		return (b);
	if (rect_empty(b))
		return (a);
	x0 = a.x;
	if (b.x < x0)
		x0 = b.x;
	y0 = a.y;
	if (b.y < y0)
		y0 = b.y;
	x1 = a.x + a.w;
	if (b.x + b.w > x1)
		x1 = b.x + b.w;
	y1 = a.y + a.h;
	if (b.y + b.h > y1)
		y1 = b.y + b.h;
	return (rect_make(x0, y0, x1 - x0, y1 - y0));
}
