#include "ctl_int.h"

bool	ctl_edt_has_sel(const t_edit *e)
{
	return (e->caret != e->anchor);
}

uint32_t	ctl_edt_sel_from(const t_edit *e)
{
	if (e->caret < e->anchor)
		return (e->caret);
	return (e->anchor);
}

uint32_t	ctl_edt_sel_to(const t_edit *e)
{
	if (e->caret > e->anchor)
		return (e->caret);
	return (e->anchor);
}

bool	ctl_edt_erase_sel(t_ctl *c, t_edit *e)
{
	uint32_t	from;
	uint32_t	to;
	uint32_t	len;

	if (!ctl_edt_has_sel(e))
		return (false);
	from = ctl_edt_sel_from(e);
	to = ctl_edt_sel_to(e);
	len = (uint32_t)strlen(c->text);
	memmove(c->text + from, c->text + to, len - to + 1);
	e->caret = from;
	e->anchor = from;
	return (true);
}

void	ctl_edt_select_all(t_ctl *c, t_edit *e)
{
	e->anchor = 0;
	e->caret = (uint32_t)strlen(c->text);
}
