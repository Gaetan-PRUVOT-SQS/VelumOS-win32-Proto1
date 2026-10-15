#include "gfx_int.h"

t_surface	gfx_surface_sub(const t_surface *s, t_rect r)
{
	t_surface	sub;
	t_box		b;
	t_box		c;

	gfx_surface_init(&sub, NULL, 0, 0);
	b = gfx_box_clip(gfx_bounds(s), gfx_box_of(r));
	if (gfx_box_empty(b))
		return (sub);
	c = gfx_box_clip(gfx_clipbox(s), b);
	sub.px = gfx_px_at(s, b.x0, b.y0);
	sub.w = (int32_t)(b.x1 - b.x0);
	sub.h = (int32_t)(b.y1 - b.y0);
	sub.stride = s->stride;
	sub.clip = gfx_box_rect(gfx_box_make(c.x0 - b.x0, c.y0 - b.y0,
				c.x1 - b.x0, c.y1 - b.y0));
	return (sub);
}
