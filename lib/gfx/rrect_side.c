#include "gfx_int.h"

static int64_t	rr_x(const t_rrctx *c, int side, int64_t u)
{
	if (side == 0)
		return (c->r.x0 + u);
	return (c->r.x1 - 1 - u);
}

static t_color	scale_alpha(t_color color, uint32_t cover)
{
	uint32_t	t;

	t = (color >> 24) * cover + 128;
	t = (t + (t >> 8)) >> 8;
	return ((color & 0x00ffffff) | (t << 24));
}

static void	rr_paint(const t_rrctx *c, int side, int64_t rad, int64_t v)
{
	int64_t	u;
	int64_t	end;

	u = gfx_max64(0, c->clip.x0 - c->r.x0);
	end = gfx_min64(rad, c->clip.x1 - c->r.x0);
	if (side == 1)
	{
		u = gfx_max64(0, c->r.x1 - c->clip.x1);
		end = gfx_min64(rad, c->r.x1 - c->clip.x0);
	}
	while (u < end)
	{
		gfx_store(gfx_px_at(c->s, rr_x(c, side, u), c->y),
			scale_alpha(c->color, gfx_corner_cover(rad, u, v)));
		u++;
	}
}

int64_t	gfx_rr_side(const t_rrctx *c, int side)
{
	int64_t	top;
	int64_t	bottom;

	top = c->y - c->r.y0;
	bottom = c->r.y1 - 1 - c->y;
	if (top < c->rad[side])
	{
		rr_paint(c, side, c->rad[side], top);
		return (c->rad[side]);
	}
	if (bottom < c->rad[3 - side])
	{
		rr_paint(c, side, c->rad[3 - side], bottom);
		return (c->rad[3 - side]);
	}
	return (0);
}
