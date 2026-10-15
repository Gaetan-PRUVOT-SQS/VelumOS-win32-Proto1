#include "gfx_int.h"

t_box	gfx_box_make(int64_t x0, int64_t y0, int64_t x1, int64_t y1)
{
	t_box	b;

	b.x0 = x0;
	b.y0 = y0;
	b.x1 = x1;
	b.y1 = y1;
	return (b);
}

t_box	gfx_box_of(t_rect r)
{
	if (r.w <= 0 || r.h <= 0)
		return (gfx_box_make(0, 0, 0, 0));
	return (gfx_box_make(r.x, r.y, (int64_t)r.x + r.w, (int64_t)r.y + r.h));
}

t_box	gfx_box_clip(t_box a, t_box b)
{
	return (gfx_box_make(gfx_max64(a.x0, b.x0), gfx_max64(a.y0, b.y0),
			gfx_min64(a.x1, b.x1), gfx_min64(a.y1, b.y1)));
}

bool	gfx_box_empty(t_box b)
{
	return (b.x1 <= b.x0 || b.y1 <= b.y0);
}

t_rect	gfx_box_rect(t_box b)
{
	if (gfx_box_empty(b))
		return (rect_make(0, 0, 0, 0));
	return (rect_make((int32_t)b.x0, (int32_t)b.y0,
			(int32_t)gfx_min64(b.x1 - b.x0, INT32_MAX),
			(int32_t)gfx_min64(b.y1 - b.y0, INT32_MAX)));
}
