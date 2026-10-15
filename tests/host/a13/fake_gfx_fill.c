#include "fakes.h"

static int64_t	imax(int64_t a, int64_t b)
{
	if (a > b)
		return (a);
	return (b);
}

static int64_t	imin(int64_t a, int64_t b)
{
	if (a < b)
		return (a);
	return (b);
}

void	gfx_fill(t_surface *s, t_rect r, t_color c)
{
	int64_t	x0;
	int64_t	x1;
	int64_t	y;
	int64_t	y1;
	int64_t	x;

	x0 = imax(imax(r.x, s->clip.x), 0);
	x1 = imin(imin((int64_t)r.x + r.w, (int64_t)s->clip.x + s->clip.w), s->w);
	y = imax(imax(r.y, s->clip.y), 0);
	y1 = imin(imin((int64_t)r.y + r.h, (int64_t)s->clip.y + s->clip.h), s->h);
	while (y < y1)
	{
		x = x0;
		while (x < x1)
			s->px[y * s->stride + x++] = c;
		y++;
	}
}
