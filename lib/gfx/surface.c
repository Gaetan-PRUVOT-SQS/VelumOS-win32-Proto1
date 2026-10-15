#include "gfx_int.h"

static bool	surface_ok(const t_surface *s)
{
	return (s != NULL && s->px != NULL && s->w > 0 && s->h > 0
		&& s->stride >= s->w);
}

t_box	gfx_bounds(const t_surface *s)
{
	if (!surface_ok(s))
		return (gfx_box_make(0, 0, 0, 0));
	return (gfx_box_make(0, 0, s->w, s->h));
}

t_box	gfx_clipbox(const t_surface *s)
{
	if (!surface_ok(s))
		return (gfx_box_make(0, 0, 0, 0));
	return (gfx_box_clip(gfx_box_of(s->clip), gfx_bounds(s)));
}

void	gfx_surface_init(t_surface *s, uint32_t *px, int32_t w, int32_t h)
{
	if (s == NULL)
		return ;
	s->px = NULL;
	s->w = 0;
	s->h = 0;
	s->stride = 0;
	s->clip = rect_make(0, 0, 0, 0);
	if (px == NULL || w <= 0 || h <= 0)
		return ;
	s->px = px;
	s->w = w;
	s->h = h;
	s->stride = w;
	s->clip = rect_make(0, 0, w, h);
}

void	gfx_set_clip(t_surface *s, t_rect r)
{
	if (s == NULL)
		return ;
	s->clip = rect_intersect(r, rect_make(0, 0, s->w, s->h));
}
