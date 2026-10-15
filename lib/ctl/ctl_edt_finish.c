#include "ctl_int.h"

bool	ctl_edt_finish(t_ctlroot *r, t_ctl *c, uint32_t res)
{
	if (res & (CTL_EDR_TEXT | CTL_EDR_VIEW))
	{
		ctl_edt_scroll(c, c->priv);
		ctl_dirty_add(r, c->rect);
	}
	if ((res & CTL_EDR_TEXT) && !ctl_notify(r, c, CN_CHANGED))
		return (true);
	if (res & CTL_EDR_ENTER)
		ctl_notify(r, c, CN_ENTER);
	return ((res & CTL_EDR_USED) != 0);
}
