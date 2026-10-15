#include "ctl_int.h"

void	ctl_edt_init(t_ctl *c)
{
	t_edit	*e;

	e = c->priv;
	e->max_chars = CTL_TEXT_MAX - 1;
	e->caret = (uint32_t)strlen(c->text);
	e->anchor = e->caret;
}

static void	truncate_chars(t_ctl *c, t_edit *e)
{
	uint32_t	len;
	uint32_t	pos;
	uint32_t	n;

	len = (uint32_t)strlen(c->text);
	pos = 0;
	n = 0;
	while (pos < len && n < e->max_chars)
	{
		pos = ctl_u8_next(c->text, len, pos);
		n++;
	}
	c->text[pos] = '\0';
	if (e->caret > pos)
		e->caret = pos;
	if (e->anchor > pos)
		e->anchor = pos;
}

void	ctl_edt_clamp(t_ctl *c, t_edit *e)
{
	truncate_chars(c, e);
	e->caret = (uint32_t)strlen(c->text);
	e->anchor = e->caret;
	e->scroll = 0;
	ctl_edt_scroll(c, e);
}

int	ctl_edit_set_limit(t_ctlroot *r, t_ctl *c, uint32_t max_chars)
{
	t_edit	*e;

	if (!r || !r->root || !c || c->type != CT_EDIT || max_chars == 0)
		return (E_INVAL);
	if (!ctl_in_tree(r, c))
		return (E_INVAL);
	if (max_chars > CTL_TEXT_MAX - 1)
		max_chars = CTL_TEXT_MAX - 1;
	e = c->priv;
	e->max_chars = max_chars;
	truncate_chars(c, e);
	ctl_edt_scroll(c, e);
	ctl_dirty_add(r, c->rect);
	return (0);
}
