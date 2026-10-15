#include "ctl_int.h"

static bool	press_track(t_ctlroot *r, t_ctl *c, bool inside)
{
	bool	was;

	was = (c->state & CTL_ST_PRESSED) != 0;
	if (was != inside)
	{
		c->state ^= CTL_ST_PRESSED;
		ctl_dirty_add(r, c->rect);
	}
	return (true);
}

static bool	press_end(t_ctlroot *r, t_ctl *c, bool inside)
{
	bool	was;

	was = (c->state & CTL_ST_PRESSED) != 0;
	c->state &= ~CTL_ST_PRESSED;
	r->capture = NULL;
	ctl_dirty_add(r, c->rect);
	if (was && inside)
		ctl_activate(r, c);
	return (true);
}

bool	ctl_press_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e)
{
	bool	inside;

	if (e->kind == CTL_MEV_DOWN)
	{
		r->capture = c;
		c->state |= CTL_ST_PRESSED;
		ctl_dirty_add(r, c->rect);
		return (true);
	}
	if (r->capture != c)
		return (false);
	inside = rect_contains(c->rect, e->at);
	if (e->kind == CTL_MEV_MOVE)
		return (press_track(r, c, inside));
	if (e->kind == CTL_MEV_UP)
		return (press_end(r, c, inside));
	return (true);
}
