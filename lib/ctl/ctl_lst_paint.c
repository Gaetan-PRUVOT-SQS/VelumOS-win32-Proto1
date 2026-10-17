#include "ctl_int.h"

static int32_t	focus_row(const t_list *l)
{
	if (l->sel >= 0)
		return (l->sel);
	return (l->sb.pos);
}

static void	draw_item(t_ctlroot *r, const t_ctl *c, int32_t idx)
{
	const t_list	*l;
	t_ctltext		tx;
	t_rect			row;
	bool			sel;

	l = c->priv;
	row = ctl_lst_item_rect(c, idx);
	sel = (idx == l->sel && ctl_enabled(c));
	tx.box = rect_make(row.x + CTL_FRAME, row.y, row.w - 2 * CTL_FRAME, row.h);
	tx.text = l->items[idx];
	tx.flags = CTL_ELLIPSIS;
	tx.enabled = true;
	tx.mnemonic = false;
	tx.font = NULL;
	tx.color = ctl_color_text(ctl_enabled(c));
	if (sel)
	{
		gfx_fill(r->surface, row, luna_color_selection());
		tx.color = ctl_color_selected_text();
	}
	ctl_text_draw(r->surface, &tx);
	if (r->focus == c && idx == focus_row(l))
		ctl_focus_ring_row(r->surface, row, sel);
}

void	ctl_lst_paint(t_ctlroot *r, t_ctl *c)
{
	const t_list	*l;
	t_lstgeom		g;
	t_rect			saved;
	int32_t			i;
	bool			en;

	l = c->priv;
	en = ctl_enabled(c);
	ctl_lst_sync(c);
	ctl_lst_geom(c, &g);
	saved = r->surface->clip;
	gfx_fill(r->surface, c->rect, ctl_color_back(en));
	luna_edit_frame(r->surface, c->rect, en);
	gfx_set_clip(r->surface, rect_intersect(saved, g.items));
	i = l->sb.pos;
	while (i < (int32_t)l->count && i <= l->sb.pos + g.rows)
	{
		draw_item(r, c, i);
		i++;
	}
	gfx_set_clip(r->surface, saved);
	if (g.bar_on)
		ctl_sb_paint(r->surface, g.bar, &l->sb, en);
}
