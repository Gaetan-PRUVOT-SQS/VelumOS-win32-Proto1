#include "ctl_int.h"

static t_ctl	*pick(const t_focusscan *sc, bool forward)
{
	if (forward && sc->after)
		return (sc->after);
	if (forward)
		return (sc->first);
	if (sc->before)
		return (sc->before);
	return (sc->last);
}

bool	ctl_focus_move(t_ctlroot *r, bool forward)
{
	t_focusscan	sc;
	t_ctl		*to;

	ctl_focus_scan(r, &sc);
	to = pick(&sc, forward);
	if (!to)
		return (false);
	ctl_focus(r, to);
	if (to->type == CT_EDIT)
	{
		ctl_edt_select_all(to, to->priv);
		ctl_dirty_add(r, to->rect);
	}
	return (true);
}
