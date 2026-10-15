#include "fake.h"

int	fake_px_count(const t_surface *s, t_rect in, uint32_t color)
{
	int	x;
	int	y;
	int	n;

	n = 0;
	y = in.y;
	while (y < in.y + in.h)
	{
		x = in.x;
		while (x < in.x + in.w)
		{
			if (s->px[y * s->stride + x] == color)
				n++;
			x++;
		}
		y++;
	}
	return (n);
}

static bool	in_region(const t_region *rg, t_point p)
{
	uint32_t	i;

	i = 0;
	while (i < rg->n)
	{
		if (rect_contains(rg->r[i], p))
			return (true);
		i++;
	}
	return (false);
}

static bool	in_list(const t_rect *want, int n, t_point p)
{
	int	i;

	i = 0;
	while (i < n)
	{
		if (rect_contains(want[i], p))
			return (true);
		i++;
	}
	return (false);
}

bool	fake_dirty_is(const t_ctlroot *r, const t_rect *want, int n)
{
	t_point	p;

	p.y = 0;
	while (p.y < r->surface->h)
	{
		p.x = 0;
		while (p.x < r->surface->w)
		{
			if (in_region(&r->dirty, p) != in_list(want, n, p))
				return (false);
			p.x++;
		}
		p.y++;
	}
	return (true);
}
