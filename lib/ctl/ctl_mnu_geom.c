#include "ctl_int.h"

int32_t	ctl_mnu_item_h(const t_ctlmenuitem *it, const t_lunametrics *m)
{
	if (it->flags & CTL_MI_SEPARATOR)
		return (m->menu_item_h / 4 + 1);
	return (m->menu_item_h);
}

t_rect	ctl_mnu_item_rect(const t_menu *m, uint32_t lvl, uint32_t idx)
{
	const t_menulevel	*lv;
	t_lunametrics		mt;
	t_rect				r;
	uint32_t			i;

	lv = &m->lv[lvl];
	luna_metrics(&mt);
	r = rect_make(lv->rect.x + CTL_MENU_BORDER, lv->rect.y + CTL_MENU_BORDER,
			lv->rect.w - 2 * CTL_MENU_BORDER, 0);
	i = 0;
	while (i < idx && i < lv->count)
	{
		r.y += ctl_mnu_item_h(&lv->items[i], &mt);
		i++;
	}
	if (idx < lv->count)
		r.h = ctl_mnu_item_h(&lv->items[idx], &mt);
	return (r);
}

int32_t	ctl_mnu_item_at(const t_menu *m, uint32_t lvl, t_point p)
{
	const t_menulevel	*lv;
	t_lunametrics		mt;
	int32_t				y;
	uint32_t			i;

	lv = &m->lv[lvl];
	if (!rect_contains(lv->rect, p))
		return (-1);
	luna_metrics(&mt);
	y = lv->rect.y + CTL_MENU_BORDER;
	i = 0;
	while (i < lv->count)
	{
		y += ctl_mnu_item_h(&lv->items[i], &mt);
		if (p.y < y)
			return ((int32_t)i);
		i++;
	}
	return (-1);
}

t_rect	ctl_mnu_bounds(const t_menu *m)
{
	t_rect		b;
	uint32_t	i;

	b = m->lv[0].rect;
	i = 1;
	while (i < m->depth)
	{
		b = rect_union(b, m->lv[i].rect);
		i++;
	}
	return (b);
}
