#include "gfx_int.h"

t_rect	rect_make(int32_t x, int32_t y, int32_t w, int32_t h)
{
	t_rect	r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

t_rect	rect_intersect(t_rect a, t_rect b)
{
	return (gfx_box_rect(gfx_box_clip(gfx_box_of(a), gfx_box_of(b))));
}

t_rect	rect_union(t_rect a, t_rect b)
{
	t_box	p;
	t_box	q;

	if (rect_empty(a) && rect_empty(b))
		return (rect_make(0, 0, 0, 0));
	if (rect_empty(a))
		return (b);
	if (rect_empty(b))
		return (a);
	p = gfx_box_of(a);
	q = gfx_box_of(b);
	return (gfx_box_rect(gfx_box_make(gfx_min64(p.x0, q.x0),
				gfx_min64(p.y0, q.y0), gfx_max64(p.x1, q.x1),
				gfx_max64(p.y1, q.y1))));
}

bool	rect_empty(t_rect r)
{
	return (r.w <= 0 || r.h <= 0);
}

bool	rect_contains(t_rect r, t_point p)
{
	t_box	b;

	b = gfx_box_of(r);
	return (p.x >= b.x0 && p.x < b.x1 && p.y >= b.y0 && p.y < b.y1);
}
