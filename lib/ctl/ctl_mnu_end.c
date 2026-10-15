#include "ctl_int.h"

void	ctl_mnu_end(t_ctlroot *r, t_ctl *c)
{
	t_menu	*m;
	t_ctl	*back;

	if (c->state & CTL_ST_CLOSED)
		return ;
	m = c->priv;
	c->state |= CTL_ST_CLOSED;
	c->flags &= ~CTL_VISIBLE;
	if (r->capture == c)
		r->capture = NULL;
	ctl_dirty_add(r, c->rect);
	if (r->focus != c)
		return ;
	back = m->prev;
	r->focus = NULL;
	ctl_focus(r, back);
}

void	ctl_mnu_cancel(t_ctlroot *r, t_ctl *c)
{
	ctl_mnu_end(r, c);
	ctl_notify(r, c, CN_CLOSED);
}

void	ctl_mnu_activate(t_ctlroot *r, t_ctl *c, uint32_t l, int32_t i)
{
	t_menu				*m;
	const t_ctlmenuitem	*it;

	m = c->priv;
	it = &m->lv[l].items[i];
	if (it->flags & (CTL_MI_SEPARATOR | CTL_MI_DISABLED))
		return ;
	ctl_mnu_select(r, c, l, i);
	if (it->sub && it->nsub)
	{
		ctl_mnu_open_sub(r, c, l);
		if (m->depth == l + 2)
			ctl_mnu_select(r, c, l + 1, ctl_mnu_step(m, l + 1, -1, 1));
		return ;
	}
	m->picked = it->id;
	ctl_mnu_end(r, c);
	ctl_notify(r, c, CN_SELECT);
}

int	ctl_menu_result(const t_ctl *menu)
{
	const t_menu	*m;

	if (!menu || menu->type != CT_MENU)
		return (0);
	m = menu->priv;
	return ((int)m->picked);
}

void	ctl_menu_close(t_ctlroot *r, t_ctl *menu)
{
	if (!r || !r->root || !menu || menu->type != CT_MENU)
		return ;
	if (!ctl_in_tree(r, menu))
		return ;
	ctl_mnu_end(r, menu);
	ctl_remove(r, menu);
}
