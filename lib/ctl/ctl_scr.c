#include "ctl_int.h"

void	ctl_scr_init(t_ctl *c)
{
	t_scrst	*st;

	st = c->priv;
	st->page = 1;
}

void	ctl_scr_paint(t_ctlroot *r, t_ctl *c)
{
	ctl_sb_paint(r->surface, c->rect, c->priv, ctl_enabled(c));
	if (r->focus != c)
		return ;
	ctl_focus_ring(r->surface, c->rect);
}

static bool	scr_apply(t_ctlroot *r, t_ctl *c, int32_t pos)
{
	t_scrst	*st;

	st = c->priv;
	if (ctl_sb_set(st, pos))
	{
		ctl_dirty_add(r, c->rect);
		ctl_notify(r, c, CN_SCROLL);
	}
	return (true);
}

bool	ctl_scr_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e)
{
	t_sbctx	x;
	t_scrst	*st;
	int		res;

	st = c->priv;
	if (e->kind == CTL_MEV_WHEEL)
		return (scr_apply(r, c, st->pos - ctl_wheel_delta(e->wheel)));
	x.r = r;
	x.c = c;
	x.st = st;
	x.bar = c->rect;
	res = ctl_sb_mouse(&x, e);
	if (res == CTL_SBR_MOVED)
		ctl_notify(r, c, CN_SCROLL);
	return (res != CTL_SBR_NONE);
}

bool	ctl_scr_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev)
{
	const t_scrst	*st;

	st = c->priv;
	if (ev->type != INP_KEY_DOWN || (ev->mods & (INPM_ALT | INPM_CTRL)))
		return (false);
	if (ev->code == VK_UP || ev->code == VK_LEFT)
		return (scr_apply(r, c, st->pos - 1));
	if (ev->code == VK_DOWN || ev->code == VK_RIGHT)
		return (scr_apply(r, c, st->pos + 1));
	if (ev->code == VK_PRIOR)
		return (scr_apply(r, c, st->pos - st->page));
	if (ev->code == VK_NEXT)
		return (scr_apply(r, c, st->pos + st->page));
	if (ev->code == VK_HOME)
		return (scr_apply(r, c, 0));
	if (ev->code == VK_END)
		return (scr_apply(r, c, st->max));
	return (false);
}
