#include "ctl_int.h"

static void	move(t_ctlroot *r, t_ctl *c, int dir)
{
	const t_menu	*m;
	uint32_t		l;
	int32_t			from;

	m = c->priv;
	l = m->depth - 1;
	from = m->lv[l].sel;
	if (dir < 0 && from < 0)
		from = (int32_t)m->lv[l].count;
	from = ctl_mnu_step(m, l, from, dir);
	if (from >= 0)
		ctl_mnu_select(r, c, l, from);
}

static void	edge(t_ctlroot *r, t_ctl *c, bool last)
{
	const t_menu	*m;
	uint32_t		l;
	int32_t			i;

	m = c->priv;
	l = m->depth - 1;
	if (last)
		i = ctl_mnu_step(m, l, (int32_t)m->lv[l].count, -1);
	else
		i = ctl_mnu_step(m, l, -1, 1);
	if (i >= 0)
		ctl_mnu_select(r, c, l, i);
}

static void	mnemonic(t_ctlroot *r, t_ctl *c, uint32_t code)
{
	const t_menu	*m;
	uint32_t		l;
	int32_t			i;

	if (code >= 'a' && code <= 'z')
		code = code - 'a' + 'A';
	m = c->priv;
	l = m->depth - 1;
	i = ctl_mnu_find(m, l, code);
	if (i < 0)
		return ;
	if (ctl_mnu_count(m, l, code) == 1)
		ctl_mnu_activate(r, c, l, i);
	else
		ctl_mnu_select(r, c, l, i);
}

static void	horizontal(t_ctlroot *r, t_ctl *c, uint32_t code)
{
	t_menu		*m;
	uint32_t	l;
	int32_t		sel;

	m = c->priv;
	l = m->depth - 1;
	sel = m->lv[l].sel;
	if (code == VK_LEFT || code == VK_ESCAPE)
	{
		if (m->depth > 1)
			ctl_mnu_pop(r, c);
		else if (code == VK_ESCAPE)
			ctl_mnu_cancel(r, c);
		return ;
	}
	if (sel >= 0)
		ctl_mnu_open_sub(r, c, l);
	if (sel >= 0 && m->depth == l + 2)
		ctl_mnu_select(r, c, l + 1, ctl_mnu_step(m, l + 1, -1, 1));
}

bool	ctl_mnu_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev)
{
	const t_menu	*m;
	uint32_t		l;

	m = c->priv;
	l = m->depth - 1;
	if (ev->type == INP_CHAR)
		mnemonic(r, c, ev->code);
	else if (ev->code == VK_UP || ev->code == VK_DOWN)
		move(r, c, 2 * (ev->code == VK_DOWN) - 1);
	else if (ev->code == VK_HOME || ev->code == VK_END)
		edge(r, c, ev->code == VK_END);
	else if (ev->code == VK_LEFT || ev->code == VK_RIGHT
		|| ev->code == VK_ESCAPE)
		horizontal(r, c, ev->code);
	else if ((ev->code == VK_RETURN || ev->code == VK_SPACE)
		&& m->lv[l].sel >= 0)
		ctl_mnu_activate(r, c, l, m->lv[l].sel);
	return (true);
}
