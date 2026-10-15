#include "kfix.h"

bool	kfix_is(const t_kfix *f, uint32_t i, uint32_t cp, int32_t at)
{
	const t_fdraw	*g;

	g = kfix_glyph(i);
	if (g->cp != cp)
		return (false);
	return (g->x == (at % 1000) * f->k.cell_w
		&& g->y == (at / 1000) * f->k.cell_h);
}

bool	kfix_same(const t_kfix *a, const t_kfix *b)
{
	int32_t	y;
	int32_t	x;

	if (a->g.s.w != b->g.s.w || a->g.s.h != b->g.s.h)
		return (false);
	y = 0;
	while (y < a->g.s.h)
	{
		x = 0;
		while (x < a->g.s.w)
		{
			if (a->g.px[y * a->g.s.stride + x]
				!= b->g.px[y * b->g.s.stride + x])
				return (false);
			x++;
		}
		y++;
	}
	return (true);
}

void	kfix_feed_n(t_kfix *f, const char *s, size_t n)
{
	kcon_feed(&f->k, s, n);
}

void	kfix_snap(const t_kfix *f, uint32_t *copy)
{
	int32_t	y;

	y = 0;
	while (y < f->g.s.h)
	{
		memcpy(copy + (size_t)y * (size_t)f->g.s.w,
			f->g.px + (size_t)y * (size_t)f->g.s.stride,
			(size_t)f->g.s.w * sizeof(uint32_t));
		y++;
	}
}

uint32_t	kfix_diff(const t_kfix *f, const uint32_t *snap)
{
	int32_t		x;
	int32_t		y;
	uint32_t	n;

	n = 0;
	y = 0;
	while (y < f->g.s.h)
	{
		x = 0;
		while (x < f->g.s.w)
		{
			if (f->g.px[y * f->g.s.stride + x] != snap[y * f->g.s.w + x])
				n++;
			x++;
		}
		y++;
	}
	return (n);
}
