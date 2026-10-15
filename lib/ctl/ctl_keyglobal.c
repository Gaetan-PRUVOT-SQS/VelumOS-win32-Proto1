#include "ctl_int.h"

static t_ctl	*default_button(const t_ctlroot *r)
{
	t_ctl	*cur;

	cur = r->root;
	while (cur)
	{
		if (cur->type == CT_BUTTON && (cur->flags & CTL_DEFAULT)
			&& ctl_usable(cur))
			return (cur);
		cur = ctl_walk_next(cur);
	}
	return (NULL);
}

static bool	fire(t_ctlroot *r, t_ctl *c)
{
	if (!c || c->type != CT_BUTTON || !ctl_usable(c))
		return (false);
	return (ctl_activate(r, c));
}

bool	ctl_key_global(t_ctlroot *r, const t_inpevent *ev)
{
	bool	alt;
	bool	ctrl;

	alt = (ev->mods & INPM_ALT) != 0;
	ctrl = (ev->mods & INPM_CTRL) != 0;
	if (ev->code == VK_TAB && !alt && !ctrl)
		return (ctl_focus_move(r, !(ev->mods & INPM_SHIFT)));
	if (alt && !ctrl)
		return (ctl_mnemonic_fire(r, ev->code));
	if (alt || ctrl || (ev->mods & INPM_REPEAT))
		return (false);
	if (ev->code == VK_RETURN)
		return (fire(r, default_button(r)));
	if (ev->code == VK_ESCAPE)
		return (fire(r, ctl_find_id(r, CTL_ID_CANCEL)));
	return (false);
}
