#include "gfx_int.h"

static void	scale_row(const t_blit *b, const t_box *d, t_dda *x, int64_t y)
{
	int64_t	sy;
	int64_t	px;
	int64_t	sx;

	sy = b->sr.y + ((2 * (y - b->dr.y) + 1) * b->sr.h)
		/ (2 * (int64_t)b->dr.h);
	if (sy < 0 || sy >= b->src->h)
		return ;
	gfx_dda_seek(x, (2 * (d->x0 - b->dr.x) + 1) * b->sr.w);
	px = d->x0;
	while (px < d->x1)
	{
		sx = b->sr.x + x->q;
		if (sx >= 0 && sx < b->src->w)
			*gfx_px_at(b->dst, px, y) = *gfx_px_at(b->src, sx, sy);
		gfx_dda_next(x);
		px++;
	}
}

void	gfx_blit_scaled(const t_blit *b)
{
	t_box	d;
	t_dda	x;
	int64_t	y;

	if (b == NULL || b->dr.w <= 0 || b->dr.h <= 0 || b->sr.w <= 0
		|| b->sr.h <= 0 || gfx_box_empty(gfx_bounds(b->src)))
		return ;
	d = gfx_box_clip(gfx_box_of(b->dr), gfx_clipbox(b->dst));
	if (gfx_box_empty(d))
		return ;
	gfx_dda_make(&x, 2 * (int64_t)b->dr.w, 2 * (int64_t)b->sr.w);
	y = d.y0;
	while (y < d.y1)
	{
		scale_row(b, &d, &x, y);
		y++;
	}
}
