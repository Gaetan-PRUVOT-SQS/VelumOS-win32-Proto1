#include "ctl_int.h"

static uint32_t	flag_if(bool changed, uint32_t flags)
{
	if (changed)
		return (CTL_EDR_USED | flags);
	return (CTL_EDR_USED);
}

static uint32_t	down_ctrl(t_ctl *c, t_edit *e, uint32_t code)
{
	if (code == CTL_VK_A)
	{
		ctl_edt_select_all(c, e);
		return (CTL_EDR_USED | CTL_EDR_VIEW);
	}
	if (code == CTL_VK_C)
		return (flag_if(ctl_edt_copy(c, e), 0));
	if (code == CTL_VK_X)
		return (flag_if(ctl_edt_copy(c, e) && ctl_edt_erase_sel(c, e),
				CTL_EDR_TEXT));
	if (code == CTL_VK_V)
		return (flag_if(ctl_edt_paste(c, e), CTL_EDR_TEXT));
	return (0);
}

static uint32_t	down_plain(t_ctl *c, t_edit *e, const t_inpevent *ev)
{
	bool	shift;

	shift = (ev->mods & INPM_SHIFT) != 0;
	if (ev->code == VK_LEFT || ev->code == VK_RIGHT || ev->code == VK_HOME
		|| ev->code == VK_END)
		return (flag_if(ctl_edt_nav(c, e, ev->code, shift), CTL_EDR_VIEW));
	if (ev->code == VK_BACK)
		return (flag_if(ctl_edt_erase_prev(c, e), CTL_EDR_TEXT));
	if (ev->code == VK_DELETE)
		return (flag_if(ctl_edt_erase_next(c, e), CTL_EDR_TEXT));
	if (ev->code == VK_RETURN)
		return (CTL_EDR_USED | CTL_EDR_ENTER);
	return (0);
}

bool	ctl_edt_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev)
{
	uint32_t	res;
	bool		alt;
	bool		ctrl;

	alt = (ev->mods & INPM_ALT) != 0;
	ctrl = (ev->mods & INPM_CTRL) != 0;
	res = 0;
	if (ev->type == INP_CHAR)
		res = ctl_edt_char(c, c->priv, ev);
	else if (ctrl && !alt)
		res = down_ctrl(c, c->priv, ev->code);
	else if (!alt && !ctrl)
		res = down_plain(c, c->priv, ev);
	return (ctl_edt_finish(r, c, res));
}
