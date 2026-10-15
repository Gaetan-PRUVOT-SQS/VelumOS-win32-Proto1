#include "fakes.h"

static void	move_row(t_surface *s, t_rect r, int32_t y, t_point d)
{
	int32_t	sy;
	int32_t	x0;
	int32_t	x1;

	sy = y - d.y;
	if (sy < r.y || sy >= r.y + r.h)
		return ;
	x0 = r.x;
	if (d.x > 0)
		x0 += d.x;
	x1 = r.x + r.w;
	if (d.x < 0)
		x1 += d.x;
	if (x1 > x0)
		memmove(&s->px[(size_t)y * (size_t)s->stride + (size_t)x0],
			&s->px[(size_t)sy * (size_t)s->stride + (size_t)(x0 - d.x)],
			(size_t)(x1 - x0) * sizeof(uint32_t));
}

void	gfx_scroll(t_surface *s, t_rect r, t_point d)
{
	int32_t	y;

	if (r.x < 0 || r.y < 0 || r.w <= 0 || r.h <= 0)
		return ;
	if (r.x + r.w > s->w || r.y + r.h > s->h)
		return ;
	if (d.y <= 0)
	{
		y = r.y;
		while (y < r.y + r.h)
			move_row(s, r, y++, d);
		return ;
	}
	y = r.y + r.h - 1;
	while (y >= r.y)
		move_row(s, r, y--, d);
}
