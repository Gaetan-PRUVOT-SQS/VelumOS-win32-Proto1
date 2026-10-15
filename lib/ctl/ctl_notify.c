#include "ctl_int.h"

bool	ctl_notify(t_ctlroot *r, t_ctl *c, uint32_t code)
{
	t_ctlcb	cb;
	void	*user;

	cb = c->cb;
	user = c->user;
	if (cb)
		cb(c, code, user);
	if (!ctl_alive(r, c))
		return (false);
	if (r->command)
		r->command(c, code, r->user);
	return (ctl_alive(r, c));
}

bool	ctl_activate(t_ctlroot *r, t_ctl *c)
{
	const t_ctlops	*ops;

	ops = ctl_ops_get(c->type);
	if (!ops || !ops->activate)
		return (false);
	return (ops->activate(r, c));
}

void	ctl_hot_set(t_ctlroot *r, t_ctl *c)
{
	if (r->hot == c)
		return ;
	if (r->hot)
	{
		r->hot->state &= ~CTL_ST_HOT;
		ctl_dirty_add(r, r->hot->rect);
	}
	r->hot = c;
	if (c)
	{
		c->state |= CTL_ST_HOT;
		ctl_dirty_add(r, c->rect);
	}
}
