#include "ctl_int.h"

t_ctl	*ctl_walk_next(const t_ctl *c)
{
	if (c->first)
		return (c->first);
	while (c && !c->next)
		c = c->parent;
	if (!c)
		return (NULL);
	return (c->next);
}

bool	ctl_is_within(const t_ctl *c, const t_ctl *top)
{
	while (c)
	{
		if (c == top)
			return (true);
		c = c->parent;
	}
	return (false);
}

bool	ctl_shown(const t_ctl *c)
{
	while (c)
	{
		if (!(c->flags & CTL_VISIBLE))
			return (false);
		c = c->parent;
	}
	return (true);
}

bool	ctl_enabled(const t_ctl *c)
{
	while (c)
	{
		if (!(c->flags & CTL_ENABLED))
			return (false);
		c = c->parent;
	}
	return (true);
}

bool	ctl_usable(const t_ctl *c)
{
	return (ctl_shown(c) && ctl_enabled(c));
}
