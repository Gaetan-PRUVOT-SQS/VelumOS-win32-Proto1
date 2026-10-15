#include "gfx_int.h"

void	gfx_fill_box(t_surface *s, t_box b, t_color c)
{
	int64_t	y;

	b = gfx_box_clip(b, gfx_clipbox(s));
	if (gfx_box_empty(b) || (c >> 24) == 0)
		return ;
	y = b.y0;
	while (y < b.y1)
	{
		gfx_span_color(gfx_px_at(s, b.x0, y), (size_t)(b.x1 - b.x0), c);
		y++;
	}
}

void	gfx_fill(t_surface *s, t_rect r, t_color c)
{
	gfx_fill_box(s, gfx_box_of(r), c);
}

void	gfx_hline(t_surface *s, t_point p, int32_t len, t_color c)
{
	gfx_fill(s, rect_make(p.x, p.y, len, 1), c);
}

void	gfx_vline(t_surface *s, t_point p, int32_t len, t_color c)
{
	gfx_fill(s, rect_make(p.x, p.y, 1, len), c);
}
