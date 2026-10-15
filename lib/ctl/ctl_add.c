#include "ctl_int.h"

static bool	parent_ok(const t_ctlroot *r, const t_ctl *parent)
{
	if (!ctl_in_tree(r, parent))
		return (false);
	if (parent->type != CT_PANEL && parent->type != CT_GROUP)
		return (false);
	return (ctl_depth(parent) + 1 < CTL_DEPTH_MAX);
}

static bool	spec_ok(const t_ctlroot *r, const t_ctlspec *spec)
{
	if (spec->type >= CT_TYPES || spec->type == CT_MENU)
		return (false);
	if (spec->id != 0 && ctl_find_id(r, spec->id))
		return (false);
	return (true);
}

t_ctl	*ctl_add(t_ctlroot *r, t_ctl *parent, const t_ctlspec *spec)
{
	t_ctl	*c;

	if (!r || !r->root || !spec)
		return (NULL);
	if (!parent)
		parent = r->root;
	if (!parent_ok(r, parent) || !spec_ok(r, spec))
		return (NULL);
	c = ctl_make(spec, 0);
	if (!c)
		return (NULL);
	ctl_link_last(parent, c);
	ctl_dirty_add(r, c->rect);
	return (c);
}
