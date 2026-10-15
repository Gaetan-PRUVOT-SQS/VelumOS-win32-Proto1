#include "luna_int.h"

void	lp_ops_list(t_surface *s, const t_lscale *sc, const t_loplist *l)
{
	if (l != NULL && l->ops != NULL)
		lp_ops(s, sc, l->ops, l->n);
}

static t_loplist	icon_find(t_cicon tab, t_iconid id)
{
	t_loplist	l;

	l.ops = NULL;
	l.n = 0;
	while (tab->id != ICON_NONE && tab->id != id)
		tab++;
	if (tab->id == id)
	{
		l.ops = tab->ops;
		l.n = tab->n;
	}
	return (l);
}

t_loplist	lp_icon_list(t_iconid id)
{
	t_loplist	l;

	l = icon_find(lp_icons_a(), id);
	if (l.ops == NULL)
		l = icon_find(lp_icons_b(), id);
	if (l.ops == NULL)
		l = icon_find(lp_icons_c(), id);
	return (l);
}

void	luna_icon(t_surface *s, t_point p, t_iconid id, int32_t size)
{
	t_lscale	sc;
	t_loplist	l;

	if (!lp_ok(s) || !lp_pt_ok(p) || size < 4 || size > LP_ICON_MAX
		|| id <= ICON_NONE || id >= ICON_IDS)
		return ;
	l = lp_icon_list(id);
	sc.ox = p.x;
	sc.oy = p.y;
	sc.num = size;
	sc.den = 64;
	lp_ops_list(s, &sc, &l);
}

void	lp_emblem(t_surface *s, t_rect r)
{
	t_lscale	sc;
	t_loplist	l;
	int32_t		size;

	size = lp_min(r.w, r.h);
	if (s == NULL || size < 4)
		return ;
	l = lp_icon_list(ICON_LOGO);
	sc.ox = r.x + (r.w - size) / 2;
	sc.oy = r.y + (r.h - size) / 2;
	sc.num = size;
	sc.den = 64;
	lp_ops_list(s, &sc, &l);
}
