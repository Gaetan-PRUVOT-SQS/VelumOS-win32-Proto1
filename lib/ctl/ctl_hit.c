#include "ctl_int.h"

static t_ctl	*topmost_kid(const t_ctl *c, t_point p)
{
	t_ctl	*kid;
	t_ctl	*found;

	found = NULL;
	kid = c->first;
	while (kid)
	{
		if ((kid->flags & CTL_VISIBLE) && rect_contains(kid->rect, p))
			found = kid;
		kid = kid->next;
	}
	return (found);
}

t_ctl	*ctl_hit(const t_ctlroot *r, t_point p)
{
	t_ctl	*cur;
	t_ctl	*found;

	cur = r->root;
	if (!cur || !(cur->flags & CTL_VISIBLE) || !rect_contains(cur->rect, p))
		return (NULL);
	found = topmost_kid(cur, p);
	while (found)
	{
		cur = found;
		found = topmost_kid(cur, p);
	}
	return (cur);
}
