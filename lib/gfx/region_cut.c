#include "gfx_int.h"

static int	piece_add(t_rect out[4], int n, t_box b)
{
	if (gfx_box_empty(b))
		return (n);
	out[n] = gfx_box_rect(b);
	return (n + 1);
}

int	gfx_rect_split(t_rect a, t_rect cut, t_rect out[4])
{
	t_box	p;
	t_box	c;
	int		n;

	p = gfx_box_of(a);
	c = gfx_box_clip(p, gfx_box_of(cut));
	if (gfx_box_empty(c))
		return (-1);
	n = piece_add(out, 0, gfx_box_make(p.x0, p.y0, p.x1, c.y0));
	n = piece_add(out, n, gfx_box_make(p.x0, c.y1, p.x1, p.y1));
	n = piece_add(out, n, gfx_box_make(p.x0, c.y0, c.x0, c.y1));
	n = piece_add(out, n, gfx_box_make(c.x1, c.y0, p.x1, c.y1));
	return (n);
}

bool	gfx_region_cut(t_region *rg, t_rect cut)
{
	t_rect		piece[4];
	uint32_t	i;
	int			k;
	int			j;

	i = rg->n;
	while (i > 0)
	{
		i--;
		k = gfx_rect_split(rg->r[i], cut, piece);
		if (k < 0)
			continue ;
		rg->n--;
		rg->r[i] = rg->r[rg->n];
		if (rg->n + (uint32_t)k > GFX_REGION_MAX)
			return (false);
		j = 0;
		while (j < k)
		{
			rg->r[rg->n] = piece[j];
			rg->n++;
			j++;
		}
	}
	return (true);
}
