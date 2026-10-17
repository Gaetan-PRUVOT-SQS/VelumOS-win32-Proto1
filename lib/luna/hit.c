#include "luna_int.h"

static const uint32_t	g_edge[3][3] = {
{HT_TOPLEFT, HT_TOP, HT_TOPRIGHT},
{HT_LEFT, HT_NOWHERE, HT_RIGHT},
{HT_BOTTOMLEFT, HT_BOTTOM, HT_BOTTOMRIGHT}
};

static int32_t	edge_side(int32_t a, int32_t b, int32_t wa, int32_t wb)
{
	if (a < wa && b < wb)
	{
		if (a < b)
			return (-1);
		if (a > b)
			return (1);
		return (0);
	}
	if (a < wa)
		return (-1);
	if (b < wb)
		return (1);
	return (0);
}

static uint32_t	hit_edge(const t_lgeo *g, t_point p)
{
	int32_t	l;
	int32_t	t;
	int32_t	sx;
	int32_t	sy;
	int32_t	c;

	l = p.x - g->outer.x;
	t = p.y - g->outer.y;
	c = lm_detail()->corner_len;
	sx = edge_side(l, g->outer.w - 1 - l, lm_detail()->resize,
			lm_detail()->resize);
	sy = edge_side(t, g->outer.h - 1 - t, lm_detail()->resize,
			lm_detail()->resize);
	if (sx != 0 && sy == 0 && lp_min(t, g->outer.h - 1 - t) < c)
		sy = edge_side(t, g->outer.h - 1 - t, c, c);
	else if (sy != 0 && sx == 0 && lp_min(l, g->outer.w - 1 - l) < c)
		sx = edge_side(l, g->outer.w - 1 - l, c, c);
	return (g_edge[sy + 1][sx + 1]);
}

static uint32_t	hit_caption(const t_lgeo *g, t_point p)
{
	int32_t	i;

	i = 0;
	while (i < LG_NBTN)
	{
		if ((g->flags & (LG_SHOWN << i)) && lp_in(g->btn[i], p))
		{
			if (g->flags & (LG_ENABLED << i))
				return (lg_btn_ht(i));
			return (HT_CAPTION);
		}
		i++;
	}
	if (lp_in(g->sysmenu, p))
		return (HT_SYSMENU);
	if (lp_in(g->caption, p))
		return (HT_CAPTION);
	return (HT_NOWHERE);
}

uint32_t	luna_hit_test(const t_lunawin *w, t_point p)
{
	t_lgeo		g;
	uint32_t	h;

	if (w == NULL)
		return (HT_NOWHERE);
	lg_make(w, &g);
	if (!lp_in(g.outer, p))
		return (HT_NOWHERE);
	if (!(g.flags & LG_FRAMED))
		return (HT_CLIENT);
	h = HT_NOWHERE;
	if (g.flags & LG_SIZABLE)
		h = hit_edge(&g, p);
	if (h == HT_NOWHERE)
		h = hit_caption(&g, p);
	if (h == HT_NOWHERE && lp_in(g.client, p))
		h = HT_CLIENT;
	return (h);
}
