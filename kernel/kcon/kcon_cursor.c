#include "kcon_int.h"

static void	flip_row(t_kcon *k, int32_t y)
{
	int32_t	i;
	t_point	p;
	t_color	c;

	i = 0;
	p.y = y;
	while (i < k->cell_w)
	{
		p.x = k->cursor_x + i;
		c = gfx_get(&k->surf, p) ^ KCON_INVERT;
		gfx_put(&k->surf, p, c | KCON_OPAQUE);
		i++;
	}
}

static void	flip(t_kcon *k)
{
	int32_t	j;

	j = 0;
	while (j < k->cell_h)
	{
		flip_row(k, k->cursor_y + j);
		j++;
	}
}

void	kcon_cursor_show(t_kcon *k)
{
	int32_t	col;

	if (k->cursor_on || !k->ready)
		return ;
	col = k->col;
	if (col >= k->cols)
		col = k->cols - 1;
	k->cursor_x = col * k->cell_w;
	k->cursor_y = k->row * k->cell_h;
	flip(k);
	k->cursor_on = true;
}

void	kcon_cursor_hide(t_kcon *k)
{
	if (!k->cursor_on)
		return ;
	flip(k);
	k->cursor_on = false;
}
