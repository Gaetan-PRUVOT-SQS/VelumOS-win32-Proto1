#include "ctl_int.h"

void	ctl_link_last(t_ctl *parent, t_ctl *c)
{
	t_ctl	*last;

	c->parent = parent;
	c->next = NULL;
	if (!parent->first)
	{
		parent->first = c;
		return ;
	}
	last = parent->first;
	while (last->next)
		last = last->next;
	last->next = c;
}

void	ctl_unlink(t_ctl *c)
{
	t_ctl	*prev;

	if (!c->parent)
		return ;
	if (c->parent->first == c)
	{
		c->parent->first = c->next;
		return ;
	}
	prev = c->parent->first;
	while (prev && prev->next != c)
		prev = prev->next;
	if (prev)
		prev->next = c->next;
}

void	ctl_free_one(t_ctl *c)
{
	const t_ctlops	*ops;

	ops = ctl_ops_get(c->type);
	if (ops && ops->destroy)
		ops->destroy(c);
	ctl_free(c);
}

void	ctl_free_subtree(t_ctl *top)
{
	t_ctl	*cur;
	t_ctl	*up;

	cur = top;
	while (cur)
	{
		while (cur->first)
			cur = cur->first;
		if (cur == top)
		{
			ctl_free_one(cur);
			return ;
		}
		up = cur->parent;
		up->first = cur->next;
		ctl_free_one(cur);
		cur = up;
	}
}
