#include "fakes.h"

t_rect	rect_make(int32_t x, int32_t y, int32_t w, int32_t h)
{
	t_rect	r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

static bool	inside(const t_surface *s, int32_t x, int32_t y)
{
	if (x < 0 || y < 0 || x >= s->w || y >= s->h)
		return (false);
	if (x < s->clip.x || y < s->clip.y)
		return (false);
	return (x < s->clip.x + s->clip.w && y < s->clip.y + s->clip.h);
}

void	gfx_put(t_surface *s, t_point p, t_color c)
{
	if (inside(s, p.x, p.y))
		s->px[(size_t)p.y * (size_t)s->stride + (size_t)p.x] = c;
}

t_color	gfx_get(const t_surface *s, t_point p)
{
	if (!inside(s, p.x, p.y))
		return (0);
	return (s->px[(size_t)p.y * (size_t)s->stride + (size_t)p.x]);
}
