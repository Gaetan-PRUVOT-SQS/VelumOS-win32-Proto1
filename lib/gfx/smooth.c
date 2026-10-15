#include "gfx_int.h"

static void	smooth_weights(uint32_t w[4], uint32_t fx, uint32_t fy)
{
	w[0] = (256 - fx) * (256 - fy);
	w[1] = fx * (256 - fy);
	w[2] = (256 - fx) * fy;
	w[3] = fx * fy;
}

static t_color	smooth_pixel(const t_blit *b, const t_tap_axis *tx,
		const t_tap_axis *ty)
{
	uint32_t	p[4];
	uint32_t	w[4];

	p[0] = *gfx_px_at(b->src, tx->i0, ty->i0);
	p[1] = *gfx_px_at(b->src, tx->i1, ty->i0);
	p[2] = *gfx_px_at(b->src, tx->i0, ty->i1);
	p[3] = *gfx_px_at(b->src, tx->i1, ty->i1);
	smooth_weights(w, tx->frac, ty->frac);
	return (gfx_bilinear(p, w));
}

static void	smooth_row(const t_blit *b, const t_box *d, int64_t y, t_dda *x)
{
	t_tap_axis	ty;
	t_tap_axis	tx;
	t_box		range;
	int64_t		px;

	range = gfx_box_clip(gfx_box_of(b->sr), gfx_bounds(b->src));
	gfx_tap_axis(&ty, (((2 * (y - b->dr.y) + 1) * b->sr.h * 128) / b->dr.h)
		- 128 + (int64_t)b->sr.y * 256, range.y0, range.y1 - 1);
	gfx_dda_seek(x, (2 * (d->x0 - b->dr.x) + 1) * b->sr.w * 128);
	px = d->x0;
	while (px < d->x1)
	{
		gfx_tap_axis(&tx, x->q - 128 + (int64_t)b->sr.x * 256, range.x0,
			range.x1 - 1);
		*gfx_px_at(b->dst, px, y) = smooth_pixel(b, &tx, &ty);
		gfx_dda_next(x);
		px++;
	}
}

void	gfx_blit_smooth(const t_blit *b)
{
	t_box	d;
	t_dda	x;
	int64_t	y;

	if (b == NULL || b->dr.w <= 0 || b->dr.h <= 0 || b->sr.w <= 0
		|| b->sr.h <= 0 || gfx_box_empty(gfx_bounds(b->src)))
		return ;
	if (b->sr.w > GFX_SMOOTH_MAX || b->sr.h > GFX_SMOOTH_MAX)
	{
		gfx_blit_scaled(b);
		return ;
	}
	d = gfx_box_clip(gfx_box_of(b->dr), gfx_clipbox(b->dst));
	if (gfx_box_empty(d)
		|| gfx_box_empty(gfx_box_clip(gfx_box_of(b->sr), gfx_bounds(b->src))))
		return ;
	gfx_dda_make(&x, b->dr.w, 256 * (int64_t)b->sr.w);
	y = d.y0;
	while (y < d.y1)
	{
		smooth_row(b, &d, y, &x);
		y++;
	}
}
