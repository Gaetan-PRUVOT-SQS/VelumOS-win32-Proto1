#include "ws_core.h"

void	wdy_init(t_wdirty *d, t_rect screen)
{
	d->n = 0;
	d->screen = screen;
}

t_rect	wdy_bounds(const t_wdirty *d)
{
	t_rect		b;
	uint32_t	i;

	if (d->n == 0)
		return (rect_make(0, 0, 0, 0));
	b = d->r[0];
	i = 1;
	while (i < d->n)
	{
		b = rect_union(b, d->r[i]);
		i++;
	}
	return (b);
}

static bool	already_covered(t_wdirty *d, t_rect r)
{
	uint32_t	i;

	i = 0;
	while (i < d->n)
	{
		if (wr_inside(d->r[i], r))
			return (true);
		if (wr_inside(r, d->r[i]))
		{
			d->n--;
			d->r[i] = d->r[d->n];
			continue ;
		}
		i++;
	}
	return (false);
}

void	wdy_add(t_wdirty *d, t_rect r)
{
	r = rect_intersect(r, d->screen);
	if (rect_empty(r) || already_covered(d, r))
		return ;
	if (d->n == WS_DIRTY_MAX)
	{
		d->r[0] = rect_union(wdy_bounds(d), r);
		d->n = 1;
		return ;
	}
	d->r[d->n] = r;
	d->n++;
}
