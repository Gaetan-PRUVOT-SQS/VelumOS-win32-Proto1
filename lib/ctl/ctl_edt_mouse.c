#include "ctl_int.h"

static uint32_t	index_of(const t_ctl *c, const t_edit *ed, const t_mouseev *e)
{
	return (ctl_edt_index_at(c, e->at.x - c->rect.x - CTL_EDIT_INSET
			+ ed->scroll));
}

static bool	edt_down(t_ctlroot *r, t_ctl *c, t_edit *ed, const t_mouseev *e)
{
	r->capture = c;
	if (e->dbl)
		ctl_edt_select_all(c, ed);
	else
	{
		ed->caret = index_of(c, ed, e);
		ed->anchor = ed->caret;
	}
	ctl_edt_scroll(c, ed);
	ctl_dirty_add(r, c->rect);
	return (true);
}

bool	ctl_edt_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e)
{
	t_edit		*ed;
	uint32_t	idx;

	ed = c->priv;
	if (e->kind == CTL_MEV_DOWN)
		return (edt_down(r, c, ed, e));
	if (r->capture != c)
		return (false);
	if (e->kind == CTL_MEV_UP)
		r->capture = NULL;
	if (e->kind == CTL_MEV_MOVE)
	{
		idx = index_of(c, ed, e);
		if (idx != ed->caret)
		{
			ed->caret = idx;
			ctl_edt_scroll(c, ed);
			ctl_dirty_add(r, c->rect);
		}
	}
	return (true);
}
