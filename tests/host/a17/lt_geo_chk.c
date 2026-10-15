#include "lt_geo.h"

static bool	chk_inside(const t_lgeo *g)
{
	int	i;

	if (!lt_rect_in(g->caption, g->outer) || !lt_rect_in(g->client, g->outer))
		return (false);
	if (!lt_rect_in(g->sysmenu, g->caption) || !lt_rect_in(g->icon, g->sysmenu))
		return (false);
	if (!lt_rect_in(g->title, g->caption))
		return (false);
	i = 0;
	while (i < LG_NBTN)
	{
		if (!lt_rect_in(g->btn[i], g->caption))
			return (false);
		i++;
	}
	return (true);
}

static bool	chk_buttons(const t_lgeo *g)
{
	int	i;
	int	j;

	i = 0;
	while (i < LG_NBTN)
	{
		j = i + 1;
		while (j < LG_NBTN)
		{
			if (!lt_rect_disjoint(g->btn[i], g->btn[j]))
				return (false);
			j++;
		}
		if (!lt_rect_disjoint(g->btn[i], g->sysmenu)
			|| !lt_rect_disjoint(g->btn[i], g->title))
			return (false);
		i++;
	}
	return (lt_rect_disjoint(g->sysmenu, g->title));
}

static bool	chk_order(const t_lgeo *g)
{
	if ((g->flags & LG_SHOWN) && (g->flags & (LG_SHOWN << 1))
		&& g->btn[0].x >= g->btn[1].x)
		return (false);
	if ((g->flags & (LG_SHOWN << 1)) && (g->flags & (LG_SHOWN << 2))
		&& g->btn[1].x >= g->btn[2].x)
		return (false);
	if ((g->flags & (LG_SHOWN << 2)) && g->btn[2].x + g->btn[2].w
		> g->outer.x + g->outer.w)
		return (false);
	return (lt_rect_disjoint(g->caption, g->client));
}

bool	lt_geo_check(const t_lunawin *w)
{
	t_lgeo	g;

	lg_make(w, &g);
	if (!(g.flags & LG_FRAMED))
		return (lt_rect_eq(g.client, g.outer));
	return (chk_inside(&g) && chk_buttons(&g) && chk_order(&g)
		&& lt_rect_eq(luna_window_client(w), g.client)
		&& lt_rect_eq(luna_caption_rect(w), g.caption));
}
