#include "ctl_int.h"

static void	marks(t_surface *s, t_rect row, const t_ctlmenuitem *it,
		t_color col)
{
	t_point	mid;

	if (it->flags & CTL_MI_CHECKED)
	{
		mid.x = row.x + CTL_MENU_GUTTER / 2;
		mid.y = row.y + row.h / 2;
		gfx_line(s, (t_point){mid.x - 3, mid.y}, (t_point){mid.x - 1,
			mid.y + 2}, col);
		gfx_line(s, (t_point){mid.x - 1, mid.y + 2}, (t_point){mid.x + 3,
			mid.y - 2}, col);
	}
	if (it->sub && it->nsub)
		ctl_sb_arrow(s, rect_make(row.x + row.w - CTL_MENU_ARROW, row.y,
				CTL_MENU_ARROW, row.h), 3, col);
}

static void	sep(t_surface *s, t_rect row)
{
	gfx_hline(s, (t_point){row.x + CTL_MENU_PAD, row.y + row.h / 2},
		row.w - 2 * CTL_MENU_PAD, ctl_color_text(false));
}

static t_color	highlight(t_ctlroot *r, t_rect row, bool sel, bool enabled)
{
	if (sel && enabled)
	{
		luna_menu_item(r->surface, row, LS_HOT);
		return (luna_color_text());
	}
	if (sel)
		ctl_focus_ring(r->surface, row);
	return (ctl_color_text(enabled));
}

static void	item(t_ctlroot *r, const t_menu *m, uint32_t l, uint32_t i)
{
	const t_ctlmenuitem	*it;
	t_rect				row;
	t_ctltext			tx;

	it = &m->lv[l].items[i];
	row = ctl_mnu_item_rect(m, l, i);
	if (it->flags & CTL_MI_SEPARATOR)
	{
		sep(r->surface, row);
		return ;
	}
	tx.enabled = !(it->flags & CTL_MI_DISABLED);
	tx.color = highlight(r, row, (int32_t)i == m->lv[l].sel, tx.enabled);
	tx.box = rect_make(row.x + CTL_MENU_GUTTER, row.y,
			row.w - CTL_MENU_GUTTER - CTL_MENU_ARROW, row.h);
	tx.text = it->text;
	tx.flags = 0;
	tx.mnemonic = true;
	tx.font = NULL;
	ctl_text_draw(r->surface, &tx);
	marks(r->surface, row, it, tx.color);
}

void	ctl_mnu_paint(t_ctlroot *r, t_ctl *c)
{
	const t_menu	*m;
	uint32_t		l;
	uint32_t		i;

	m = c->priv;
	l = 0;
	while (l < m->depth)
	{
		luna_menu_panel(r->surface, m->lv[l].rect);
		i = 0;
		while (i < m->lv[l].count)
		{
			item(r, m, l, i);
			i++;
		}
		l++;
	}
}
