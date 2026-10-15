#include "ctl_int.h"

void	ctl_lst_geom(const t_ctl *c, t_lstgeom *g)
{
	const t_list	*l;
	t_lunametrics	m;
	const t_font	*f;
	t_rect			in;

	l = c->priv;
	luna_metrics(&m);
	f = ctl_font(0);
	g->rowh = 1;
	if (f && f->height > 0)
		g->rowh = f->height + 2 * CTL_ROW_PAD;
	in = rect_make(c->rect.x + CTL_FRAME, c->rect.y + CTL_FRAME,
			c->rect.w - 2 * CTL_FRAME, c->rect.h - 2 * CTL_FRAME);
	g->rows = in.h / g->rowh;
	if (g->rows < 1)
		g->rows = 1;
	g->bar_on = (int64_t)l->count > g->rows && in.w > m.scroll_w;
	g->items = in;
	g->bar = rect_make(0, 0, 0, 0);
	if (!g->bar_on)
		return ;
	g->items.w -= m.scroll_w;
	g->bar = rect_make(in.x + in.w - m.scroll_w, in.y, m.scroll_w, in.h);
}

void	ctl_lst_sync(t_ctl *c)
{
	t_list		*l;
	t_lstgeom	g;

	l = c->priv;
	ctl_lst_geom(c, &g);
	l->sb.page = g.rows;
	l->sb.max = 0;
	if ((int64_t)l->count > g.rows)
		l->sb.max = (int32_t)l->count - g.rows;
	ctl_sb_set(&l->sb, l->sb.pos);
}

void	ctl_lst_show(t_ctl *c, int32_t idx)
{
	t_list	*l;

	l = c->priv;
	ctl_lst_sync(c);
	if (idx < l->sb.pos)
		ctl_sb_set(&l->sb, idx);
	else if (idx >= l->sb.pos + l->sb.page)
		ctl_sb_set(&l->sb, idx - l->sb.page + 1);
}

t_rect	ctl_lst_item_rect(const t_ctl *c, int32_t idx)
{
	const t_list	*l;
	t_lstgeom		g;

	l = c->priv;
	ctl_lst_geom(c, &g);
	return (rect_make(g.items.x, g.items.y + (idx - l->sb.pos) * g.rowh,
			g.items.w, g.rowh));
}

int32_t	ctl_lst_index_at(const t_ctl *c, t_point p)
{
	const t_list	*l;
	t_lstgeom		g;
	int32_t			idx;

	l = c->priv;
	ctl_lst_geom(c, &g);
	if (!rect_contains(g.items, p))
		return (-1);
	idx = l->sb.pos + (p.y - g.items.y) / g.rowh;
	if (idx < 0 || idx >= (int32_t)l->count)
		return (-1);
	return (idx);
}
