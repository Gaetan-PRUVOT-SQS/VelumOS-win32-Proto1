#include "ctl_int.h"

static t_ctl	*hot_of(t_ctl *t)
{
	if (t && ctl_ops_get(t->type)->hot && ctl_usable(t))
		return (t);
	return (NULL);
}

static t_ctl	*pick_target(t_ctlroot *r, const t_mouseev *e)
{
	t_ctl	*t;

	if (r->capture)
		return (r->capture);
	t = ctl_hit(r, e->at);
	ctl_hot_set(r, hot_of(t));
	return (t);
}

static bool	route(t_ctlroot *r, const t_wmmouse *m)
{
	t_mouseev		e;
	t_ctl			*t;
	const t_ctlops	*ops;

	e = ctl_mouse_event(r, m);
	t = pick_target(r, &e);
	if (!t)
		return (false);
	if (!ctl_usable(t))
		return (true);
	if (e.kind == CTL_MEV_DOWN)
	{
		e.dbl = ctl_mouse_dbl(r, t, &e);
		ctl_focus(r, t);
	}
	ops = ctl_ops_get(t->type);
	if (!ops->mouse)
		return (false);
	return (ops->mouse(r, t, &e));
}

bool	ctl_mouse(t_ctlroot *r, const t_wmmouse *m)
{
	bool	used;

	if (!r || !r->root || !m)
		return (false);
	used = route(r, m);
	ctl_reap(r);
	return (used);
}
