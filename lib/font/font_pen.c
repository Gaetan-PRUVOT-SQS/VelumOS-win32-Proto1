#include "font_int.h"

static int32_t	clamp_to(int64_t value, int32_t low, int32_t high)
{
	if (value < low)
		return (low);
	if (value > high)
		return (high);
	return ((int32_t)value);
}

int	font_pen_start(t_pen *pen, t_surface *dst, const t_textreq *rq)
{
	const t_rect	*clip;

	if (!dst->px || dst->w <= 0 || dst->h <= 0)
		return (0);
	clip = &dst->clip;
	pen->dst = dst;
	pen->font = rq->font;
	pen->color = rq->color;
	pen->x = rq->at.x;
	pen->y = rq->at.y;
	pen->box.left = clamp_to(clip->x, 0, dst->w);
	pen->box.top = clamp_to(clip->y, 0, dst->h);
	pen->box.right = clamp_to((int64_t)clip->x + clip->w, 0, dst->w);
	pen->box.bottom = clamp_to((int64_t)clip->y + clip->h, 0, dst->h);
	return (pen->box.left < pen->box.right && pen->box.top < pen->box.bottom);
}

int	font_glyph_cut(const t_pen *pen, const t_glyph *g, t_cut *cut)
{
	int64_t	gx;
	int64_t	gy;

	gx = pen->x + g->xoff;
	gy = pen->y + g->yoff;
	if (gx >= pen->box.right || gx + g->w <= pen->box.left)
		return (0);
	if (gy >= pen->box.bottom || gy + g->h <= pen->box.top)
		return (0);
	cut->x = (int32_t)gx;
	cut->y = (int32_t)gy;
	cut->c0 = clamp_to(pen->box.left - gx, 0, g->w);
	cut->c1 = clamp_to(pen->box.right - gx, 0, g->w);
	cut->r0 = clamp_to(pen->box.top - gy, 0, g->h);
	cut->r1 = clamp_to(pen->box.bottom - gy, 0, g->h);
	return (1);
}
