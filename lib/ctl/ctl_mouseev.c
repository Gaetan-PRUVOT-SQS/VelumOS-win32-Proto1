#include "ctl_int.h"

static uint32_t	kind_of(const t_wmmouse *m, uint32_t was)
{
	uint32_t	left;

	left = 1u << BTN_LEFT;
	if (m->type == INP_MOUSE_MOVE)
		return (CTL_MEV_MOVE);
	if (m->type == INP_WHEEL)
		return (CTL_MEV_WHEEL);
	if (m->type == INP_MOUSE_DOWN && (m->buttons & ~was & left))
		return (CTL_MEV_DOWN);
	if (m->type == INP_MOUSE_UP && (was & ~m->buttons & left))
		return (CTL_MEV_UP);
	return (CTL_MEV_NONE);
}

t_mouseev	ctl_mouse_event(t_ctlroot *r, const t_wmmouse *m)
{
	t_mouseev	e;
	t_rootpriv	*rp;
	uint32_t	was;

	rp = ctl_rootpriv(r);
	was = rp->buttons;
	rp->buttons = m->buttons;
	e.kind = kind_of(m, was);
	e.at.x = m->x;
	e.at.y = m->y;
	e.wheel = m->wheel;
	e.dbl = false;
	return (e);
}

static bool	near_click(const t_rootpriv *rp, const t_mouseev *e)
{
	int32_t	dx;
	int32_t	dy;

	dx = e->at.x - rp->click_at.x;
	dy = e->at.y - rp->click_at.y;
	if (dx < 0)
		dx = -dx;
	if (dy < 0)
		dy = -dy;
	return (dx <= CTL_DBLCLK_DIST && dy <= CTL_DBLCLK_DIST);
}

bool	ctl_mouse_dbl(t_ctlroot *r, const t_ctl *c, const t_mouseev *e)
{
	t_rootpriv	*rp;
	int64_t		now;
	bool		dbl;

	rp = ctl_rootpriv(r);
	if (!rp->clock)
		return (false);
	now = rp->clock();
	dbl = rp->click_ctl == (uintptr_t)c && near_click(rp, e)
		&& now >= rp->click_ns && now - rp->click_ns <= CTL_DBLCLK_NS;
	rp->click_ctl = (uintptr_t)c;
	rp->click_ns = now;
	rp->click_at = e->at;
	if (dbl)
		rp->click_ctl = 0;
	return (dbl);
}
