#include "fake.h"

void	gfx_set_clip(t_surface *s, t_rect r)
{
	s->clip = rect_intersect(r, rect_make(0, 0, s->w, s->h));
}

void	gfx_put(t_surface *s, t_point p, t_color c)
{
	if (!rect_contains(s->clip, p))
		return ;
	s->px[p.y * s->stride + p.x] = c;
}

void	gfx_hline(t_surface *s, t_point p, int32_t len, t_color c)
{
	t_rect	r;
	int32_t	i;

	r = rect_intersect(rect_make(p.x, p.y, len, 1), s->clip);
	i = 0;
	while (i < r.w)
	{
		gfx_put(s, (t_point){r.x + i, r.y}, c);
		i++;
	}
}

void	gfx_vline(t_surface *s, t_point p, int32_t len, t_color c)
{
	t_rect	r;
	int32_t	i;

	r = rect_intersect(rect_make(p.x, p.y, 1, len), s->clip);
	i = 0;
	while (i < r.h)
	{
		gfx_put(s, (t_point){r.x, r.y + i}, c);
		i++;
	}
}

void	gfx_fill(t_surface *s, t_rect r, t_color c)
{
	int32_t	i;

	r = rect_intersect(r, s->clip);
	i = 0;
	while (i < r.h)
	{
		gfx_hline(s, (t_point){r.x, r.y + i}, r.w, c);
		i++;
	}
}
