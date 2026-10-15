#include "ctl_int.h"

void	ctl_focus(t_ctlroot *r, t_ctl *c)
{
	if (!r || !r->root)
		return ;
	if (r->capture && r->capture->type == CT_MENU)
		return ;
	if (c && (!ctl_in_tree(r, c) || !(c->flags & CTL_FOCUSABLE)
			|| !ctl_usable(c)))
		return ;
	if (r->focus == c)
		return ;
	if (r->focus)
		ctl_dirty_add(r, r->focus->rect);
	r->focus = c;
	if (c)
		ctl_dirty_add(r, c->rect);
}

bool	ctl_tabstop(const t_ctl *c)
{
	if (!(c->flags & CTL_FOCUSABLE) || !ctl_usable(c))
		return (false);
	if (c->type == CT_RADIO)
		return (ctl_radio_stop(c));
	return (true);
}

static void	tally(t_focusscan *sc, t_ctl *cur, const t_ctl *focus, bool past)
{
	if (!sc->first)
		sc->first = cur;
	sc->last = cur;
	if (cur == focus)
		return ;
	if (!past)
		sc->before = cur;
	else if (!sc->after)
		sc->after = cur;
}

void	ctl_focus_scan(const t_ctlroot *r, t_focusscan *sc)
{
	t_ctl	*cur;
	bool	past;

	memset(sc, 0, sizeof(*sc));
	past = false;
	cur = r->root;
	while (cur)
	{
		if (cur == r->focus)
			past = true;
		if (ctl_tabstop(cur))
			tally(sc, cur, r->focus, past);
		cur = ctl_walk_next(cur);
	}
}

bool	ctl_focus_after(t_ctlroot *r, const t_ctl *c)
{
	t_ctl	*cur;

	cur = ctl_walk_next(c);
	while (cur)
	{
		if (ctl_tabstop(cur))
		{
			ctl_focus(r, cur);
			return (true);
		}
		cur = ctl_walk_next(cur);
	}
	return (false);
}
