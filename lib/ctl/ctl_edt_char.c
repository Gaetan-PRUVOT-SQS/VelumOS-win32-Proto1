#include "ctl_int.h"

uint32_t	ctl_edt_char(t_ctl *c, t_edit *e, const t_inpevent *ev)
{
	char		buf[4];
	uint32_t	n;
	bool		alt;
	bool		ctrl;

	alt = (ev->mods & INPM_ALT) != 0;
	ctrl = (ev->mods & INPM_CTRL) != 0;
	if (alt != ctrl)
		return (0);
	if (ev->code < 0x20 || (ev->code >= 0x7f && ev->code < 0xa0))
		return (0);
	n = ctl_u8_encode(ev->code, buf);
	if (n == 0)
		return (0);
	if (ctl_edt_insert(c, e, buf, n))
		return (CTL_EDR_USED | CTL_EDR_TEXT);
	return (CTL_EDR_USED);
}
