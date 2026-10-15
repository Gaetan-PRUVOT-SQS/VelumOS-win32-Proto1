#include "luna_int.h"

t_point	lp_pt(int32_t x, int32_t y)
{
	t_point	p;

	p.x = x;
	p.y = y;
	return (p);
}

t_rect	lp_rect(int32_t x, int32_t y, int32_t w, int32_t h)
{
	t_rect	r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

bool	lp_in(t_rect r, t_point p)
{
	if (r.w <= 0 || r.h <= 0)
		return (false);
	if (p.x < r.x || p.y < r.y)
		return (false);
	if ((int64_t)p.x >= (int64_t)r.x + r.w)
		return (false);
	return ((int64_t)p.y < (int64_t)r.y + r.h);
}

t_rect	lp_isect(t_rect a, t_rect b)
{
	int64_t	x0;
	int64_t	y0;
	int64_t	x1;
	int64_t	y1;
	t_rect	out;

	x0 = a.x;
	y0 = a.y;
	if (b.x > x0)
		x0 = b.x;
	if (b.y > y0)
		y0 = b.y;
	x1 = (int64_t)a.x + a.w;
	y1 = (int64_t)a.y + a.h;
	if ((int64_t)b.x + b.w < x1)
		x1 = (int64_t)b.x + b.w;
	if ((int64_t)b.y + b.h < y1)
		y1 = (int64_t)b.y + b.h;
	out = lp_rect(a.x, a.y, 0, 0);
	if (a.w <= 0 || a.h <= 0 || b.w <= 0 || b.h <= 0 || x1 <= x0 || y1 <= y0)
		return (out);
	return (lp_rect((int32_t)x0, (int32_t)y0, (int32_t)(x1 - x0),
		(int32_t)(y1 - y0)));
}

t_rect	lp_inset(t_rect r, int32_t d)
{
	int32_t	w;
	int32_t	h;

	w = lp_max(r.w - 2 * d, 0);
	h = lp_max(r.h - 2 * d, 0);
	return (lp_rect(r.x + d, r.y + d, w, h));
}
