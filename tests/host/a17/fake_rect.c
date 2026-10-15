#include <stddef.h>
#include <stdint.h>
#include "fake_gfx.h"

t_rect	fk_rect(int64_t x0, int64_t y0, int64_t x1, int64_t y1)
{
	t_rect	r;

	r.x = (int32_t)x0;
	r.y = (int32_t)y0;
	r.w = 0;
	r.h = 0;
	if (x1 > x0 && y1 > y0)
	{
		r.w = (int32_t)(x1 - x0);
		r.h = (int32_t)(y1 - y0);
	}
	return (r);
}

t_rect	fk_cut(t_rect a, t_rect b)
{
	int64_t	x0;
	int64_t	y0;
	int64_t	x1;
	int64_t	y1;

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
	return (fk_rect(x0, y0, x1, y1));
}

t_rect	fk_visible(const t_surface *s, t_rect r)
{
	t_rect	bounds;

	bounds.x = 0;
	bounds.y = 0;
	bounds.w = s->w;
	bounds.h = s->h;
	return (fk_cut(fk_cut(r, bounds), s->clip));
}
