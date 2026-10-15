#include "gfx_int.h"

static bool	line_range(t_line *c)
{
	int64_t	lo;
	int64_t	mlo;
	int64_t	mhi;

	mlo = c->clip.x0;
	mhi = c->clip.x1;
	if (c->steep)
	{
		mlo = c->clip.y0;
		mhi = c->clip.y1;
	}
	lo = gfx_max64(0, mlo - c->maj0);
	c->hi = gfx_min64(c->len, mhi - 1 - c->maj0);
	c->i = lo;
	c->q = 0;
	c->r = 0;
	if (lo > c->hi)
		return (false);
	if (c->len > 0)
	{
		c->q = (c->dmin * (uint64_t)lo) / (uint64_t)c->len;
		c->r = (c->dmin * (uint64_t)lo) % (uint64_t)c->len;
	}
	return (true);
}

static void	line_step(t_line *c)
{
	c->i++;
	c->r += c->dmin;
	if (c->r >= (uint64_t)c->len)
	{
		c->r -= (uint64_t)c->len;
		c->q++;
	}
}

static int	line_pixel(t_surface *s, const t_line *c, t_color col)
{
	int64_t	minor;
	int64_t	lo;
	int64_t	hi;

	minor = c->min0 + c->sgn * (int64_t)(c->q + (2 * c->r > (uint64_t)c->len));
	lo = c->clip.y0;
	hi = c->clip.y1;
	if (c->steep)
	{
		lo = c->clip.x0;
		hi = c->clip.x1;
	}
	if ((c->sgn > 0 && minor >= hi) || (c->sgn < 0 && minor < lo))
		return (-1);
	if (minor < lo || minor >= hi)
	{
		if (c->dmin == 0)
			return (-1);
		return (0);
	}
	if (c->steep)
		gfx_store(gfx_px_at(s, minor, c->maj0 + c->i), col);
	else
		gfx_store(gfx_px_at(s, c->maj0 + c->i, minor), col);
	return (1);
}

void	gfx_line_walk(t_surface *s, t_line *c, t_color col)
{
	if (!line_range(c))
		return ;
	while (c->i <= c->hi)
	{
		if (line_pixel(s, c, col) < 0)
			return ;
		line_step(c);
	}
}
