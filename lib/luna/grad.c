#include "luna_int.h"

static uint32_t	grad_ratio(const t_lgrad *g, int32_t i, int32_t pos)
{
	int32_t	span;

	span = g->st[i].at - g->st[i - 1].at;
	if (span <= 0)
		return (255);
	return ((uint32_t)(((pos - g->st[i - 1].at) * 255) / span));
}

t_color	lp_grad_at(const t_lgrad *g, int32_t pos)
{
	t_color	c;
	int32_t	i;

	if (g == NULL || g->n < 1)
		return (0);
	i = 1;
	while (i < g->n && pos > g->st[i].at)
		i++;
	if (g->n == 1 || pos <= g->st[0].at)
		c = g->st[0].c;
	else if (i >= g->n)
		c = g->st[g->n - 1].c;
	else
		c = lp_mix(g->st[i - 1].c, g->st[i].c, grad_ratio(g, i, pos));
	if (g->tint_t)
		c = lp_mix(c, g->tint, g->tint_t);
	return (c);
}

void	lp_vfill(t_surface *s, t_rect r, const t_lgrad *g)
{
	t_rect	vis;
	int32_t	y;

	vis = lp_visible(s, r);
	y = vis.y;
	while (y < vis.y + vis.h)
	{
		lp_fill(s, lp_rect(vis.x, y, vis.w, 1), lp_grad_at(g, y - r.y));
		y++;
	}
}

void	lp_hfill(t_surface *s, t_rect r, const t_lgrad *g)
{
	t_rect	vis;
	int32_t	x;

	vis = lp_visible(s, r);
	x = vis.x;
	while (x < vis.x + vis.w)
	{
		lp_fill(s, lp_rect(x, vis.y, 1, vis.h), lp_grad_at(g, x - r.x));
		x++;
	}
}
