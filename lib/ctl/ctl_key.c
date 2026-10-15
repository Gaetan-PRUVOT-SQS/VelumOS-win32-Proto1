#include "ctl_int.h"

static t_ctl	*key_target(const t_ctlroot *r)
{
	if (r->capture && r->capture->type == CT_MENU)
		return (r->capture);
	return (r->focus);
}

static bool	route(t_ctlroot *r, const t_inpevent *ev)
{
	t_ctl			*t;
	const t_ctlops	*ops;

	if (ev->type != INP_KEY_DOWN && ev->type != INP_CHAR)
		return (false);
	t = key_target(r);
	if (t && ctl_usable(t))
	{
		ops = ctl_ops_get(t->type);
		if (ops->key && ops->key(r, t, ev))
			return (true);
	}
	if (ev->type != INP_KEY_DOWN)
		return (false);
	return (ctl_key_global(r, ev));
}

bool	ctl_key(t_ctlroot *r, const t_inpevent *ev)
{
	bool	used;

	if (!r || !r->root || !ev)
		return (false);
	used = route(r, ev);
	ctl_reap(r);
	return (used);
}
