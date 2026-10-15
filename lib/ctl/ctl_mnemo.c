#include "ctl_int.h"

static bool	has_text(const t_ctl *c)
{
	return (c->type == CT_BUTTON || c->type == CT_CHECK
		|| c->type == CT_RADIO || c->type == CT_LABEL
		|| c->type == CT_GROUP);
}

static bool	match(const t_ctl *c, uint32_t code)
{
	if (!has_text(c) || !ctl_usable(c))
		return (false);
	return ((uint32_t)ctl_mnemonic(c->text) == code);
}

static t_ctl	*search(const t_ctlroot *r, uint32_t code)
{
	t_ctl	*cur;
	t_ctl	*first;
	bool	past;

	first = NULL;
	past = false;
	cur = r->root;
	while (cur)
	{
		if (past && cur != r->focus && match(cur, code))
			return (cur);
		if (!first && match(cur, code))
			first = cur;
		if (cur == r->focus)
			past = true;
		cur = ctl_walk_next(cur);
	}
	return (first);
}

bool	ctl_mnemonic_fire(t_ctlroot *r, uint32_t code)
{
	t_ctl	*to;

	if (code >= 'a' && code <= 'z')
		code = code - 'a' + 'A';
	to = search(r, code);
	if (!to)
		return (false);
	ctl_focus(r, to);
	ctl_activate(r, to);
	return (true);
}
