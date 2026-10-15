#include "gfx_int.h"

uint32_t	*gfx_px_at(const t_surface *s, int64_t x, int64_t y)
{
	return (s->px + (size_t)y * (size_t)s->stride + (size_t)x);
}

void	gfx_put(t_surface *s, t_point p, t_color c)
{
	t_box	k;

	k = gfx_clipbox(s);
	if (p.x < k.x0 || p.x >= k.x1 || p.y < k.y0 || p.y >= k.y1)
		return ;
	gfx_store(gfx_px_at(s, p.x, p.y), c);
}

t_color	gfx_get(const t_surface *s, t_point p)
{
	t_box	k;

	k = gfx_bounds(s);
	if (p.x < k.x0 || p.x >= k.x1 || p.y < k.y0 || p.y >= k.y1)
		return (0);
	return (*gfx_px_at(s, p.x, p.y));
}
