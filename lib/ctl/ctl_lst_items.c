#include "ctl_int.h"

static bool	grow(t_list *l)
{
	char		**n;
	uint32_t	cap;

	if (l->count < l->cap)
		return (true);
	cap = l->cap * 2;
	if (cap == 0)
		cap = 8;
	n = ctl_alloc(cap * sizeof(char *));
	if (!n)
		return (false);
	if (l->count)
		memcpy(n, l->items, l->count * sizeof(char *));
	ctl_free(l->items);
	l->items = n;
	l->cap = cap;
	return (true);
}

void	ctl_lst_drop(t_list *l)
{
	uint32_t	i;

	i = 0;
	while (i < l->count)
	{
		ctl_free(l->items[i]);
		i++;
	}
	ctl_free(l->items);
	l->items = NULL;
	l->count = 0;
	l->cap = 0;
}

int	ctl_list_add(t_ctlroot *r, t_ctl *list, const char *item)
{
	t_list	*l;
	char	*copy;
	char	tmp[CTL_TEXT_MAX];

	l = ctl_lst_of(r, list);
	if (!l || !item)
		return (E_INVAL);
	if (l->count >= CTL_LIST_ITEMS_MAX)
		return (E_NOSPC);
	ctl_copy_text(tmp, item);
	copy = ctl_alloc(strlen(tmp) + 1);
	if (!copy)
		return (E_NOMEM);
	if (!grow(l))
	{
		ctl_free(copy);
		return (E_NOMEM);
	}
	memcpy(copy, tmp, strlen(tmp) + 1);
	l->items[l->count] = copy;
	l->count++;
	ctl_lst_sync(list);
	ctl_dirty_add(r, list->rect);
	return ((int)l->count - 1);
}

int	ctl_list_remove(t_ctlroot *r, t_ctl *list, int index)
{
	t_list	*l;

	l = ctl_lst_of(r, list);
	if (!l)
		return (E_INVAL);
	if (index < 0 || (uint32_t)index >= l->count)
		return (E_RANGE);
	ctl_free(l->items[index]);
	memmove(l->items + index, l->items + index + 1,
		(l->count - (uint32_t)index - 1) * sizeof(char *));
	l->count--;
	if (l->sel == index)
		l->sel = -1;
	else if (l->sel > index)
		l->sel--;
	ctl_lst_sync(list);
	ctl_dirty_add(r, list->rect);
	return (0);
}

void	ctl_list_clear(t_ctlroot *r, t_ctl *list)
{
	t_list	*l;

	l = ctl_lst_of(r, list);
	if (!l)
		return ;
	ctl_lst_drop(l);
	l->sel = -1;
	l->sb.pos = 0;
	ctl_lst_sync(list);
	ctl_dirty_add(r, list->rect);
}
