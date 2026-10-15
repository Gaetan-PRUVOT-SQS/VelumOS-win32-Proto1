#include "gfx_int.h"

static t_box	frame_side(t_box b, int side)
{
	if (side == 0)
		return (gfx_box_make(b.x0, b.y0, b.x1, b.y0 + 1));
	if (side == 1 && b.y1 - b.y0 > 1)
		return (gfx_box_make(b.x0, b.y1 - 1, b.x1, b.y1));
	if (side == 2)
		return (gfx_box_make(b.x0, b.y0 + 1, b.x0 + 1, b.y1 - 1));
	if (side == 3 && b.x1 - b.x0 > 1)
		return (gfx_box_make(b.x1 - 1, b.y0 + 1, b.x1, b.y1 - 1));
	return (gfx_box_make(0, 0, 0, 0));
}

void	gfx_frame(t_surface *s, t_rect r, t_color c)
{
	t_box	b;
	int		side;

	b = gfx_box_of(r);
	if (gfx_box_empty(b))
		return ;
	side = 0;
	while (side < 4)
	{
		gfx_fill_box(s, frame_side(b, side), c);
		side++;
	}
}
