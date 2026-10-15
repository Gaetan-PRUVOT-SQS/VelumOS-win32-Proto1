#include "ctl_int.h"

static int32_t	level_at(const t_menu *m, t_point p)
{
	int32_t	l;

	l = (int32_t)m->depth - 1;
	while (l >= 0)
	{
		if (rect_contains(m->lv[l].rect, p))
			return (l);
		l--;
	}
	return (-1);
}

static void	hover(t_ctlroot *r, t_ctl *c, uint32_t l, t_point p)
{
	t_menu				*m;
	const t_ctlmenuitem	*it;
	int32_t				i;

	m = c->priv;
	i = ctl_mnu_item_at(m, l, p);
	if (i < 0)
		return ;
	it = &m->lv[l].items[i];
	if (it->flags & CTL_MI_SEPARATOR)
		return ;
	ctl_mnu_select(r, c, l, i);
	if (m->depth == l + 1 && it->sub)
		ctl_mnu_open_sub(r, c, l);
}

static void	press(t_ctlroot *r, t_ctl *c, uint32_t l, t_point p)
{
	const t_menu	*m;
	int32_t			i;

	m = c->priv;
	i = ctl_mnu_item_at(m, l, p);
	if (i >= 0)
		ctl_mnu_activate(r, c, l, i);
}

bool	ctl_mnu_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e)
{
	const t_menu	*m;
	int32_t			l;

	m = c->priv;
	l = level_at(m, e->at);
	if (e->kind == CTL_MEV_DOWN && l < 0)
		ctl_mnu_cancel(r, c);
	else if (e->kind == CTL_MEV_DOWN)
		press(r, c, (uint32_t)l, e->at);
	else if (e->kind == CTL_MEV_MOVE && l >= 0)
		hover(r, c, (uint32_t)l, e->at);
	return (true);
}
