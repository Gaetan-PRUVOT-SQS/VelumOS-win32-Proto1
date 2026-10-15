#include "ctl_int.h"

bool	ctl_rad_activate(t_ctlroot *r, t_ctl *c)
{
	ctl_radio_select(r, c);
	ctl_dirty_add(r, c->rect);
	ctl_notify(r, c, CN_CLICKED);
	return (true);
}

static bool	rad_move(t_ctlroot *r, t_ctl *c, bool forward)
{
	t_ctl	*to;

	to = ctl_radio_step(c, forward);
	if (!to || to == c)
		return (true);
	ctl_focus(r, to);
	ctl_rad_activate(r, to);
	return (true);
}

bool	ctl_rad_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev)
{
	if (ev->type != INP_KEY_DOWN || (ev->mods & (INPM_ALT | INPM_CTRL)))
		return (false);
	if (ev->code == VK_SPACE)
	{
		if (!(ev->mods & INPM_REPEAT))
			ctl_activate(r, c);
		return (true);
	}
	if (ev->code == VK_LEFT || ev->code == VK_UP)
		return (rad_move(r, c, false));
	if (ev->code == VK_RIGHT || ev->code == VK_DOWN)
		return (rad_move(r, c, true));
	return (false);
}
