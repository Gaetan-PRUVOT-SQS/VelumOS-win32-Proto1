#include "ctl_int.h"

static void	band(t_surface *s, t_rect r, t_color col)
{
	int32_t	w;
	int32_t	in;

	w = CTL_RING_W;
	in = r.h - 2 * w;
	gfx_fill(s, rect_make(r.x, r.y, r.w, w), col);
	gfx_fill(s, rect_make(r.x, r.y + r.h - w, r.w, w), col);
	gfx_fill(s, rect_make(r.x, r.y + w, w, in), col);
	gfx_fill(s, rect_make(r.x + r.w - w, r.y + w, w, in), col);
}

static void	paint_ring(t_surface *s, t_rect r, t_color col)
{
	if (r.w <= 0 || r.h <= 0)
		return ;
	if (r.w <= 2 * CTL_RING_W || r.h <= 2 * CTL_RING_W)
	{
		gfx_fill(s, r, col);
		return ;
	}
	band(s, r, col);
}

void	ctl_focus_ring(t_surface *s, t_rect r)
{
	paint_ring(s, r, luna_color_accent());
}

void	ctl_focus_ring_row(t_surface *s, t_rect r, bool sel)
{
	if (sel)
		paint_ring(s, r, ctl_color_selected_text());
	else
		paint_ring(s, r, luna_color_accent());
}
