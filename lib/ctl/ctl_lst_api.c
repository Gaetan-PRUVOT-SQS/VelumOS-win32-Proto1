#include "ctl_int.h"

int	ctl_list_selected(const t_ctl *list)
{
	const t_list	*l;

	if (!list || list->type != CT_LIST)
		return (-1);
	l = list->priv;
	return (l->sel);
}

int	ctl_list_count(const t_ctl *list)
{
	const t_list	*l;

	if (!list || list->type != CT_LIST)
		return (E_INVAL);
	l = list->priv;
	return ((int)l->count);
}

int	ctl_list_text(const t_ctl *list, int index, char *out, uint32_t cap)
{
	const t_list	*l;
	uint32_t		len;
	uint32_t		n;

	if (!list || list->type != CT_LIST || !out || cap == 0)
		return (E_INVAL);
	l = list->priv;
	if (index < 0 || (uint32_t)index >= l->count)
		return (E_RANGE);
	len = (uint32_t)strlen(l->items[index]);
	n = 0;
	while (n < len && ctl_u8_next(l->items[index], len, n) <= cap - 1)
		n = ctl_u8_next(l->items[index], len, n);
	memcpy(out, l->items[index], n);
	out[n] = '\0';
	return ((int)n);
}
