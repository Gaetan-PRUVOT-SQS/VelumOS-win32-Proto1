#include "ctl_int.h"

static t_scrst	*scroll_of(t_ctlroot *r, t_ctl *c)
{
	if (!r || !r->root || !c || c->type != CT_SCROLL)
		return (NULL);
	if (!ctl_in_tree(r, c))
		return (NULL);
	return (c->priv);
}

void	ctl_scroll_set(t_ctlroot *r, t_ctl *c, int max, int page)
{
	t_scrst	*st;
	t_scrst	old;

	st = scroll_of(r, c);
	if (!st)
		return ;
	old = *st;
	if (max < 0)
		max = 0;
	if (max > CTL_SCROLL_MAX)
		max = CTL_SCROLL_MAX;
	if (page < 1)
		page = 1;
	if (page > CTL_SCROLL_MAX)
		page = CTL_SCROLL_MAX;
	st->max = max;
	st->page = page;
	ctl_sb_set(st, st->pos);
	if (old.max != st->max || old.page != st->page || old.pos != st->pos)
		ctl_dirty_add(r, c->rect);
}

int	ctl_scroll_pos(const t_ctl *c)
{
	const t_scrst	*st;

	if (!c || c->type != CT_SCROLL)
		return (0);
	st = c->priv;
	return (st->pos);
}

void	ctl_scroll_move(t_ctlroot *r, t_ctl *c, int pos)
{
	t_scrst	*st;

	st = scroll_of(r, c);
	if (st && ctl_sb_set(st, pos))
		ctl_dirty_add(r, c->rect);
}
