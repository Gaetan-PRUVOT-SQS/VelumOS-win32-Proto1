#include "ctl_int.h"

static int32_t	item_w(const t_ctlmenuitem *it)
{
	t_ctltext	tx;

	if ((it->flags & CTL_MI_SEPARATOR) || !it->text)
		return (0);
	tx.box = rect_make(0, 0, 0, 0);
	tx.text = it->text;
	tx.flags = 0;
	tx.mnemonic = true;
	tx.font = NULL;
	return (ctl_text_width(&tx));
}

static t_point	menu_size(const t_menu *m, uint32_t lvl)
{
	const t_menulevel	*lv;
	t_lunametrics		mt;
	t_point				sz;
	uint32_t			i;

	lv = &m->lv[lvl];
	luna_metrics(&mt);
	sz.x = 0;
	sz.y = 2 * CTL_MENU_BORDER;
	i = 0;
	while (i < lv->count)
	{
		sz.y += ctl_mnu_item_h(&lv->items[i], &mt);
		if (item_w(&lv->items[i]) > sz.x)
			sz.x = item_w(&lv->items[i]);
		i++;
	}
	sz.x += 2 * CTL_MENU_BORDER + CTL_MENU_GUTTER + CTL_MENU_ARROW;
	return (sz);
}

static t_point	sub_origin(const t_menu *m, uint32_t lvl, t_point sz,
		int32_t sw)
{
	const t_menulevel	*par;
	t_rect				it;
	t_point				o;

	par = &m->lv[lvl - 1];
	it = ctl_mnu_item_rect(m, lvl - 1, (uint32_t)par->sel);
	o.x = par->rect.x + par->rect.w - CTL_MENU_BORDER;
	o.y = it.y - CTL_MENU_BORDER;
	if (o.x + sz.x > sw)
		o.x = par->rect.x - sz.x + CTL_MENU_BORDER;
	return (o);
}

static void	place(const t_surface *s, t_rect *r)
{
	if (r->x + r->w > s->w)
		r->x = s->w - r->w;
	if (r->y + r->h > s->h)
		r->y = s->h - r->h;
	if (r->x < 0)
		r->x = 0;
	if (r->y < 0)
		r->y = 0;
}

void	ctl_mnu_layout(t_ctlroot *r, t_ctl *c, uint32_t lvl)
{
	t_menu		*m;
	t_menulevel	*lv;
	t_point		sz;
	t_point		o;

	m = c->priv;
	lv = &m->lv[lvl];
	sz = menu_size(m, lvl);
	o.x = lv->rect.x;
	o.y = lv->rect.y;
	if (lvl > 0)
		o = sub_origin(m, lvl, sz, r->surface->w);
	lv->rect = rect_make(o.x, o.y, sz.x, sz.y);
	place(r->surface, &lv->rect);
}
