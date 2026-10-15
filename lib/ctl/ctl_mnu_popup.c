#include "ctl_int.h"

static bool	fill(t_ctl *c, const t_ctlmenuspec *spec, size_t pool)
{
	t_menu	*m;
	t_pool	p;

	m = c->priv;
	p.cur = (char *)c->priv + sizeof(t_menu);
	p.left = pool;
	m->lv[0].items = ctl_mnu_copy(&p, spec->items, spec->count);
	m->lv[0].count = spec->count;
	m->lv[0].sel = -1;
	m->lv[0].rect = c->rect;
	m->depth = 1;
	return (m->lv[0].items != NULL);
}

static t_ctl	*build(const t_ctlmenuspec *spec, size_t pool)
{
	t_ctlspec	cs;
	t_ctl		*c;

	cs.type = CT_MENU;
	cs.id = spec->id;
	cs.flags = CTL_VISIBLE | CTL_ENABLED | CTL_FOCUSABLE;
	cs.rect = rect_make(spec->at.x, spec->at.y, 0, 0);
	cs.text = NULL;
	c = ctl_make(&cs, pool);
	if (!c)
		return (NULL);
	if (fill(c, spec, pool))
		return (c);
	ctl_free(c);
	return (NULL);
}

static void	close_others(t_ctlroot *r)
{
	t_ctl	*kid;

	kid = r->root->first;
	while (kid)
	{
		if (kid->type == CT_MENU)
			ctl_mnu_end(r, kid);
		kid = kid->next;
	}
}

static void	open(t_ctlroot *r, t_ctl *c)
{
	t_menu	*m;

	m = c->priv;
	close_others(r);
	if (r->capture)
	{
		r->capture->state &= ~CTL_ST_PRESSED;
		ctl_dirty_add(r, r->capture->rect);
	}
	if (r->focus)
		ctl_dirty_add(r, r->focus->rect);
	m->prev = r->focus;
	ctl_link_last(r->root, c);
	ctl_mnu_layout(r, c, 0);
	ctl_mnu_refresh(r, c);
	r->focus = c;
	r->capture = c;
}

t_ctl	*ctl_menu_popup(t_ctlroot *r, const t_ctlmenuspec *spec)
{
	t_ctl	*c;
	size_t	pool;

	if (!r || !r->root || !spec)
		return (NULL);
	pool = ctl_mnu_pool(spec->items, spec->count, 0);
	if (pool == 0 || (spec->id && ctl_find_id(r, spec->id)))
		return (NULL);
	c = build(spec, pool);
	if (!c)
		return (NULL);
	open(r, c);
	return (c);
}
