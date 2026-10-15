#include "ctl_int.h"

int32_t	ctl_mnu_step(const t_menu *m, uint32_t lvl, int32_t from, int dir)
{
	const t_menulevel	*lv;
	int32_t				n;
	int32_t				i;
	int32_t				k;

	lv = &m->lv[lvl];
	n = (int32_t)lv->count;
	i = from;
	k = 0;
	while (k < n)
	{
		i = (i + dir + n) % n;
		if (!(lv->items[i].flags & CTL_MI_SEPARATOR))
			return (i);
		k++;
	}
	return (-1);
}

static bool	mn_match(const t_ctlmenuitem *it, uint32_t ch)
{
	if ((it->flags & (CTL_MI_SEPARATOR | CTL_MI_DISABLED)) || !it->text)
		return (false);
	return ((uint32_t)ctl_mnemonic(it->text) == ch);
}

uint32_t	ctl_mnu_count(const t_menu *m, uint32_t lvl, uint32_t ch)
{
	uint32_t	i;
	uint32_t	n;

	i = 0;
	n = 0;
	while (i < m->lv[lvl].count)
	{
		if (mn_match(&m->lv[lvl].items[i], ch))
			n++;
		i++;
	}
	return (n);
}

int32_t	ctl_mnu_find(const t_menu *m, uint32_t lvl, uint32_t ch)
{
	const t_menulevel	*lv;
	int32_t				n;
	int32_t				i;
	int32_t				k;

	lv = &m->lv[lvl];
	n = (int32_t)lv->count;
	i = lv->sel;
	k = 0;
	while (k < n)
	{
		i = (i + 1 + n) % n;
		if (mn_match(&lv->items[i], ch))
			return (i);
		k++;
	}
	return (-1);
}
