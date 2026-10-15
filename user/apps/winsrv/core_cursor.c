#include "ws_core.h"

t_rect	wcur_rect(t_wcursor c)
{
	t_wcurdef	d;

	if (!c.visible)
		return (rect_make(c.pos.x, c.pos.y, 0, 0));
	wcur_get(c.shape, &d);
	return (rect_make(c.pos.x - d.hx, c.pos.y - d.hy, d.w, d.h));
}

static void	draw_row(t_surface *s, const t_wcurdef *d, t_point at, int32_t y)
{
	int32_t	x;
	char	ch;

	x = 0;
	while (x < d->w)
	{
		ch = d->pix[y * d->w + x];
		if (ch == 'X')
			gfx_put(s, (t_point){at.x + x, at.y + y}, 0xff000000);
		else if (ch == '.')
			gfx_put(s, (t_point){at.x + x, at.y + y}, 0xffffffff);
		x++;
	}
}

void	wcur_draw(t_surface *s, t_wcursor c)
{
	t_wcurdef	d;
	t_point		at;
	int32_t		y;

	if (!c.visible)
		return ;
	wcur_get(c.shape, &d);
	at.x = c.pos.x - d.hx;
	at.y = c.pos.y - d.hy;
	y = 0;
	while (y < d.h)
	{
		draw_row(s, &d, at, y);
		y++;
	}
}
