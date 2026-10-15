#include "ctl_int.h"

static bool	lst_wheel(t_ctlroot *r, t_ctl *c, const t_mouseev *e)
{
	t_list	*l;

	l = c->priv;
	ctl_lst_sync(c);
	if (ctl_sb_set(&l->sb, l->sb.pos - ctl_wheel_delta(e->wheel)))
	{
		ctl_dirty_add(r, c->rect);
		ctl_notify(r, c, CN_SCROLL);
	}
	return (true);
}

static bool	lst_down(t_ctlroot *r, t_ctl *c, const t_mouseev *e)
{
	int32_t	idx;

	idx = ctl_lst_index_at(c, e->at);
	if (idx < 0)
		return (true);
	if (ctl_lst_pick(r, c, idx) && e->dbl)
		ctl_notify(r, c, CN_DBLCLK);
	return (true);
}

bool	ctl_lst_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e)
{
	t_lstgeom	g;
	t_sbctx		x;
	int			res;

	if (e->kind == CTL_MEV_WHEEL)
		return (lst_wheel(r, c, e));
	ctl_lst_sync(c);
	ctl_lst_geom(c, &g);
	if (e->kind == CTL_MEV_DOWN && !rect_contains(g.bar, e->at))
		return (lst_down(r, c, e));
	x.r = r;
	x.c = c;
	x.st = &((t_list *)c->priv)->sb;
	x.bar = g.bar;
	res = ctl_sb_mouse(&x, e);
	if (res == CTL_SBR_MOVED)
		ctl_notify(r, c, CN_SCROLL);
	return (res != CTL_SBR_NONE);
}
