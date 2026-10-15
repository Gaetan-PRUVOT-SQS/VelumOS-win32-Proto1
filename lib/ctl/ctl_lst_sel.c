#include "ctl_int.h"

void	ctl_lst_init(t_ctl *c)
{
	t_list	*l;

	l = c->priv;
	l->sel = -1;
	l->sb.page = 1;
}

void	ctl_lst_destroy(t_ctl *c)
{
	ctl_lst_drop(c->priv);
}

t_list	*ctl_lst_of(const t_ctlroot *r, const t_ctl *list)
{
	if (!r || !r->root || !list || list->type != CT_LIST)
		return (NULL);
	if (!ctl_in_tree(r, list))
		return (NULL);
	return (list->priv);
}

int	ctl_list_select(t_ctlroot *r, t_ctl *list, int index)
{
	t_list	*l;

	l = ctl_lst_of(r, list);
	if (!l)
		return (E_INVAL);
	if (index < -1 || index >= (int)l->count)
		return (E_RANGE);
	if (index == l->sel)
		return (0);
	l->sel = index;
	if (index >= 0)
		ctl_lst_show(list, index);
	ctl_dirty_add(r, list->rect);
	return (0);
}

bool	ctl_lst_pick(t_ctlroot *r, t_ctl *c, int32_t idx)
{
	t_list	*l;

	l = c->priv;
	if (idx == l->sel)
		return (true);
	l->sel = idx;
	ctl_lst_show(c, idx);
	ctl_dirty_add(r, c->rect);
	return (ctl_notify(r, c, CN_SELECT));
}
