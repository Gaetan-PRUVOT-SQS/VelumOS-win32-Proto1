#include "ctl_int.h"

static int32_t	clamp(int32_t v, int32_t hi)
{
	if (v < 0)
		return (0);
	if (v > hi)
		return (hi);
	return (v);
}

static bool	is_nav(uint32_t code)
{
	return (code == VK_UP || code == VK_DOWN || code == VK_HOME
		|| code == VK_END || code == VK_PRIOR || code == VK_NEXT);
}

static int32_t	nav(const t_list *l, uint32_t code, int32_t rows)
{
	int32_t	last;

	last = (int32_t)l->count - 1;
	if (code == VK_HOME)
		return (0);
	if (code == VK_END)
		return (last);
	if (l->sel < 0)
		return (0);
	if (code == VK_UP)
		return (clamp(l->sel - 1, last));
	if (code == VK_DOWN)
		return (clamp(l->sel + 1, last));
	if (code == VK_PRIOR)
		return (clamp(l->sel - rows, last));
	return (clamp(l->sel + rows, last));
}

bool	ctl_lst_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev)
{
	const t_list	*l;
	t_lstgeom		g;

	if (ev->type != INP_KEY_DOWN || (ev->mods & (INPM_ALT | INPM_CTRL)))
		return (false);
	l = c->priv;
	if (ev->code == VK_RETURN)
	{
		if (l->sel >= 0)
			ctl_notify(r, c, CN_ENTER);
		return (true);
	}
	if (!is_nav(ev->code))
		return (false);
	if (l->count == 0)
		return (true);
	ctl_lst_geom(c, &g);
	ctl_lst_pick(r, c, nav(l, ev->code, g.rows));
	return (true);
}
