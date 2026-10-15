#include "ctl_int.h"

void	ctl_remove(t_ctlroot *r, t_ctl *c)
{
	if (!r || !r->root || !c || c == r->root)
		return ;
	if (!ctl_in_tree(r, c))
		return ;
	ctl_dirty_add(r, c->rect);
	ctl_forget(r, c);
	ctl_unlink(c);
	ctl_free_subtree(c);
}

void	ctl_refs_check(t_ctlroot *r)
{
	if (r->focus && !ctl_usable(r->focus))
		r->focus = NULL;
	if (r->hot && !ctl_usable(r->hot))
	{
		r->hot->state &= ~CTL_ST_HOT;
		r->hot = NULL;
	}
	if (r->capture && !ctl_usable(r->capture))
	{
		r->capture->state &= ~CTL_ST_PRESSED;
		r->capture = NULL;
	}
}

void	ctl_reap(t_ctlroot *r)
{
	t_ctl	*kid;
	t_ctl	*next;

	if (!r->root)
		return ;
	kid = r->root->first;
	while (kid)
	{
		next = kid->next;
		if (kid->type == CT_MENU && (kid->state & CTL_ST_CLOSED))
			ctl_remove(r, kid);
		kid = next;
	}
}
