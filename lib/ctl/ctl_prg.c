#include "ctl_int.h"

void	ctl_progress_set(t_ctlroot *r, t_ctl *c, uint32_t pct)
{
	t_progress	*p;

	if (!r || !r->root || !c || c->type != CT_PROGRESS)
		return ;
	if (!ctl_in_tree(r, c))
		return ;
	if (pct > 100)
		pct = 100;
	p = c->priv;
	if (p->pct == pct)
		return ;
	p->pct = pct;
	ctl_dirty_add(r, c->rect);
}

void	ctl_prg_paint(t_ctlroot *r, t_ctl *c)
{
	const t_progress	*p;

	p = c->priv;
	luna_progress(r->surface, c->rect, p->pct);
}
