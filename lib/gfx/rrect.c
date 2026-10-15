#include "gfx_int.h"

static int64_t	rr_radius(int32_t radius, t_rect r)
{
	int64_t	cap;

	cap = gfx_min64(r.w, r.h) / 2;
	cap = gfx_min64(cap, GFX_RADIUS_MAX);
	return (gfx_max64(0, gfx_min64(radius, cap)));
}

static void	rr_init(t_rrctx *c, t_surface *s, const t_rrect *rr)
{
	c->s = s;
	c->r = gfx_box_of(rr->r);
	c->clip = gfx_box_clip(c->r, gfx_clipbox(s));
	c->color = rr->color;
	c->rad[0] = rr_radius(rr->tl, rr->r);
	c->rad[1] = rr_radius(rr->tr, rr->r);
	c->rad[2] = rr_radius(rr->br, rr->r);
	c->rad[3] = rr_radius(rr->bl, rr->r);
}

static void	rr_row(t_rrctx *c)
{
	int64_t	left;
	int64_t	right;
	int64_t	x0;
	int64_t	x1;

	left = gfx_rr_side(c, 0);
	right = gfx_rr_side(c, 1);
	x0 = gfx_max64(c->clip.x0, c->r.x0 + left);
	x1 = gfx_min64(c->clip.x1, c->r.x1 - right);
	if (x1 > x0)
		gfx_span_color(gfx_px_at(c->s, x0, c->y), (size_t)(x1 - x0), c->color);
}

void	gfx_rrect_fill(t_surface *s, const t_rrect *rr)
{
	t_rrctx	c;

	if (rr == NULL || (rr->color >> 24) == 0)
		return ;
	rr_init(&c, s, rr);
	if (gfx_box_empty(c.clip))
		return ;
	c.y = c.clip.y0;
	while (c.y < c.clip.y1)
	{
		rr_row(&c);
		c.y++;
	}
}
