#include "ctl_int.h"

void	ctl_mnu_refresh(t_ctlroot *r, t_ctl *c)
{
	t_rect	old;

	old = c->rect;
	c->rect = ctl_mnu_bounds(c->priv);
	ctl_dirty_add(r, old);
	ctl_dirty_add(r, c->rect);
}

void	ctl_mnu_select(t_ctlroot *r, t_ctl *c, uint32_t l, int32_t i)
{
	t_menu		*m;
	t_menulevel	*lv;

	m = c->priv;
	lv = &m->lv[l];
	if (lv->sel == i)
		return ;
	if (lv->sel >= 0)
		ctl_dirty_add(r, ctl_mnu_item_rect(m, l, (uint32_t)lv->sel));
	lv->sel = i;
	if (i >= 0)
		ctl_dirty_add(r, ctl_mnu_item_rect(m, l, (uint32_t)i));
	if (m->depth > l + 1)
	{
		m->depth = l + 1;
		ctl_mnu_refresh(r, c);
	}
}

void	ctl_mnu_open_sub(t_ctlroot *r, t_ctl *c, uint32_t l)
{
	t_menu				*m;
	const t_ctlmenuitem	*it;

	m = c->priv;
	if (m->depth != l + 1 || l + 1 >= CTL_MENU_LEVELS_MAX)
		return ;
	if (m->lv[l].sel < 0)
		return ;
	it = &m->lv[l].items[m->lv[l].sel];
	if (!it->sub || it->nsub == 0 || (it->flags & CTL_MI_DISABLED))
		return ;
	m->lv[l + 1].items = it->sub;
	m->lv[l + 1].count = it->nsub;
	m->lv[l + 1].sel = -1;
	m->depth = l + 2;
	ctl_mnu_layout(r, c, l + 1);
	ctl_mnu_refresh(r, c);
}

void	ctl_mnu_pop(t_ctlroot *r, t_ctl *c)
{
	t_menu	*m;

	m = c->priv;
	if (m->depth <= 1)
		return ;
	m->depth--;
	ctl_mnu_refresh(r, c);
}
