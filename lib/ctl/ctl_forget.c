#include "ctl_int.h"

static void	forget_menus(t_ctlroot *r, const t_ctl *c)
{
	t_ctl	*kid;
	t_menu	*m;

	kid = r->root->first;
	while (kid)
	{
		m = kid->priv;
		if (kid->type == CT_MENU && m->prev && ctl_is_within(m->prev, c))
			m->prev = NULL;
		kid = kid->next;
	}
}

void	ctl_forget(t_ctlroot *r, t_ctl *c)
{
	if (r->focus && ctl_is_within(r->focus, c))
		r->focus = NULL;
	if (r->hot && ctl_is_within(r->hot, c))
		r->hot = NULL;
	if (r->capture && ctl_is_within(r->capture, c))
		r->capture = NULL;
	forget_menus(r, c);
}
