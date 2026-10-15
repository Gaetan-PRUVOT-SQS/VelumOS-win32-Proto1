#include "ctl_int.h"

void	ctl_paint_tree(t_ctlroot *r, t_ctl *c, t_rect clip)
{
	t_rect			vis;
	t_ctl			*kid;
	const t_ctlops	*ops;

	if (!(c->flags & CTL_VISIBLE))
		return ;
	vis = rect_intersect(clip, c->rect);
	if (rect_empty(vis))
		return ;
	gfx_set_clip(r->surface, vis);
	ops = ctl_ops_get(c->type);
	ops->paint(r, c);
	kid = c->first;
	while (kid)
	{
		ctl_paint_tree(r, kid, vis);
		kid = kid->next;
	}
}

t_rect	ctl_paint(t_ctlroot *r)
{
	t_region	work;
	t_rect		bounds;
	t_rect		saved;
	uint32_t	i;

	if (!r || !r->root || !r->surface || region_empty(&r->dirty))
		return (rect_make(0, 0, 0, 0));
	work = r->dirty;
	region_clear(&r->dirty);
	bounds = region_bounds(&work);
	saved = r->surface->clip;
	i = 0;
	while (i < work.n)
	{
		ctl_paint_tree(r, r->root, work.r[i]);
		i++;
	}
	gfx_set_clip(r->surface, saved);
	return (bounds);
}
