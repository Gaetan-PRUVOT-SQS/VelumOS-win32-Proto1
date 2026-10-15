#include "fake_gfx.h"

void	gfx_surface_init(t_surface *s, uint32_t *px, int32_t w, int32_t h)
{
	s->px = px;
	s->w = w;
	s->h = h;
	s->stride = w;
	s->clip.x = 0;
	s->clip.y = 0;
	s->clip.w = w;
	s->clip.h = h;
}

void	gfx_set_clip(t_surface *s, t_rect r)
{
	t_rect	bounds;

	bounds.x = 0;
	bounds.y = 0;
	bounds.w = s->w;
	bounds.h = s->h;
	s->clip = fk_cut(r, bounds);
}

void	gfx_put(t_surface *s, t_point p, t_color c)
{
	t_rect	one;

	one.x = p.x;
	one.y = p.y;
	one.w = 1;
	one.h = 1;
	one = fk_visible(s, one);
	if (one.w == 1 && one.h == 1)
		s->px[(size_t)one.y * (size_t)s->stride + (size_t)one.x] = c;
}

t_color	gfx_get(const t_surface *s, t_point p)
{
	if (p.x < 0 || p.y < 0 || p.x >= s->w || p.y >= s->h)
		return (0);
	return (s->px[(size_t)p.y * (size_t)s->stride + (size_t)p.x]);
}

void	gfx_fill(t_surface *s, t_rect r, t_color c)
{
	t_rect	v;
	int32_t	x;
	int32_t	y;

	v = fk_visible(s, r);
	y = v.y;
	while (y < v.y + v.h)
	{
		x = v.x;
		while (x < v.x + v.w)
		{
			s->px[(size_t)y * (size_t)s->stride + (size_t)x] = c;
			x++;
		}
		y++;
	}
}
