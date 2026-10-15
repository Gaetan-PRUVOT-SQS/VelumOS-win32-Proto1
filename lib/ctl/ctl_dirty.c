#include "ctl_int.h"

bool	ctl_rect_ok(t_rect r)
{
	if (r.x < -CTL_COORD_MAX || r.x > CTL_COORD_MAX)
		return (false);
	if (r.y < -CTL_COORD_MAX || r.y > CTL_COORD_MAX)
		return (false);
	return (r.w >= 0 && r.w <= CTL_COORD_MAX && r.h >= 0
		&& r.h <= CTL_COORD_MAX);
}

void	ctl_dirty_add(t_ctlroot *r, t_rect rect)
{
	t_rect	bounds;

	bounds = rect_make(0, 0, r->surface->w, r->surface->h);
	rect = rect_intersect(rect, bounds);
	if (!rect_empty(rect))
		region_add(&r->dirty, rect);
}

void	ctl_invalidate(t_ctlroot *r, t_ctl *c)
{
	if (!r || !r->root || !c || !ctl_in_tree(r, c))
		return ;
	ctl_dirty_add(r, c->rect);
}

t_rootpriv	*ctl_rootpriv(const t_ctlroot *r)
{
	return (r->root->priv);
}
