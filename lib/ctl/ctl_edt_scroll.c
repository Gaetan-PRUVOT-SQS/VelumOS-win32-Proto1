#include "ctl_int.h"

void	ctl_edt_scroll(t_ctl *c, t_edit *e)
{
	int32_t	view;
	int32_t	cx;
	int32_t	limit;

	view = c->rect.w - 2 * CTL_EDIT_INSET;
	if (view <= 0)
	{
		e->scroll = 0;
		return ;
	}
	cx = ctl_edt_width(c, e->caret);
	if (cx < e->scroll)
		e->scroll = cx;
	if (cx > e->scroll + view - 1)
		e->scroll = cx - view + 1;
	limit = ctl_edt_width(c, (uint32_t)strlen(c->text)) - view + 1;
	if (limit < 0)
		limit = 0;
	if (e->scroll > limit)
		e->scroll = limit;
	if (e->scroll < 0)
		e->scroll = 0;
}
