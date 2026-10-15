#include "ctl_int.h"

static uint32_t	target(const t_ctl *c, const t_edit *e, uint32_t code,
		bool shift)
{
	uint32_t	len;

	len = (uint32_t)strlen(c->text);
	if (code == VK_HOME)
		return (0);
	if (code == VK_END)
		return (len);
	if (code == VK_LEFT && ctl_edt_has_sel(e) && !shift)
		return (ctl_edt_sel_from(e));
	if (code == VK_RIGHT && ctl_edt_has_sel(e) && !shift)
		return (ctl_edt_sel_to(e));
	if (code == VK_LEFT)
		return (ctl_u8_prev(c->text, e->caret));
	return (ctl_u8_next(c->text, len, e->caret));
}

bool	ctl_edt_nav(t_ctl *c, t_edit *e, uint32_t code, bool shift)
{
	uint32_t	old_caret;
	uint32_t	old_anchor;

	old_caret = e->caret;
	old_anchor = e->anchor;
	e->caret = target(c, e, code, shift);
	if (!shift)
		e->anchor = e->caret;
	return (old_caret != e->caret || old_anchor != e->anchor);
}
