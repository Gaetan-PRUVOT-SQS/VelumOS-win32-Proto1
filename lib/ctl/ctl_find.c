#include "ctl_int.h"

t_ctl	*ctl_find_id(const t_ctlroot *r, uint32_t id)
{
	t_ctl	*cur;

	if (id == 0)
		return (NULL);
	cur = r->root;
	while (cur)
	{
		if (cur->id == id && !(cur->state & CTL_ST_CLOSED))
			return (cur);
		cur = ctl_walk_next(cur);
	}
	return (NULL);
}

t_ctl	*ctl_find(t_ctlroot *r, uint32_t id)
{
	if (!r || !r->root)
		return (NULL);
	return (ctl_find_id(r, id));
}

uint32_t	ctl_depth(const t_ctl *c)
{
	uint32_t	d;

	d = 0;
	while (c->parent)
	{
		d++;
		c = c->parent;
	}
	return (d);
}

bool	ctl_in_tree(const t_ctlroot *r, const t_ctl *c)
{
	if (!r->root || !c)
		return (false);
	while (c->parent)
		c = c->parent;
	return (c == r->root);
}

bool	ctl_alive(const t_ctlroot *r, const t_ctl *c)
{
	const t_ctl	*cur;

	cur = r->root;
	while (cur)
	{
		if (cur == c)
			return (true);
		cur = ctl_walk_next(cur);
	}
	return (false);
}
