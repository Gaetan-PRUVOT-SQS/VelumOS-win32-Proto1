#include "luna_int.h"

#define GB_RMAX 4096

static t_color	cell_avg(const t_surface *s, t_rect c)
{
	uint32_t	sum[3];
	uint32_t	n;
	t_color		p;
	int32_t		x;
	int32_t		y;

	sum[0] = 0;
	sum[1] = 0;
	sum[2] = 0;
	n = 0;
	y = c.y - 1;
	while (++y < c.y + c.h)
	{
		x = c.x - 1;
		while (++x < c.x + c.w)
		{
			p = s->px[(size_t)y * (size_t)s->stride + (size_t)x];
			sum[0] += (p >> 16) & 0xff;
			sum[1] += (p >> 8) & 0xff;
			sum[2] += p & 0xff;
			n++;
		}
	}
	return (0xff000000 | ((sum[0] / n) << 16) | ((sum[1] / n) << 8)
		| (sum[2] / n));
}

void	lp_gb_down(const t_surface *s, t_rect src, t_lgb *o)
{
	t_rect	c;
	int32_t	cx;
	int32_t	cy;

	o->src = src;
	o->w = (src.w + LX_SCALE - 1) / LX_SCALE;
	o->h = (src.h + LX_SCALE - 1) / LX_SCALE;
	cy = -1;
	while (++cy < o->h)
	{
		cx = -1;
		while (++cx < o->w)
		{
			c = lp_rect(src.x + cx * LX_SCALE, src.y + cy * LX_SCALE,
					lp_min(LX_SCALE, src.w - cx * LX_SCALE),
					lp_min(LX_SCALE, src.h - cy * LX_SCALE));
			o->a[cy * o->w + cx] = cell_avg(s, c);
		}
	}
}

void	lp_gb_blur(t_lgb *o, int32_t r)
{
	t_lgl		p;
	int32_t		pass;
	int32_t		i;

	r = lp_clamp(r, 1, GB_RMAX);
	pass = -1;
	while (++pass < LX_PASSES)
	{
		p = (t_lgl){o->w, 1, r};
		i = -1;
		while (++i < o->h)
			lp_gb_line(o->a + i * o->w, o->b + i * o->w, &p);
		p = (t_lgl){o->h, o->w, r};
		i = -1;
		while (++i < o->w)
			lp_gb_line(o->b + i, o->a + i, &p);
	}
}
