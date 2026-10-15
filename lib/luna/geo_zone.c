#include "luna_int.h"

static void	zone_sysmenu(t_lgeo *g)
{
	const t_lunadetail	*d;
	int32_t				ic;

	d = lm_detail();
	ic = d->caption_icon;
	g->sysmenu = lp_rect(g->outer.x, g->outer.y, 0, 0);
	g->icon = g->sysmenu;
	if (!(g->flags & LG_SYSMENU) || (g->flags & LG_TOOL))
		return ;
	g->sysmenu = lp_isect(lp_rect(g->caption.x, g->caption.y, d->sysmenu_w,
				g->caption.h), g->caption);
	g->icon = lp_rect(g->caption.x + d->icon_left,
			g->outer.y + (g->cap_end - ic) / 2, ic, ic);
	if (!lg_inside(g->icon, g->sysmenu))
		g->icon = lp_rect(g->outer.x, g->outer.y, 0, 0);
}

static void	zone_title(t_lgeo *g)
{
	int32_t	left;
	int32_t	right;
	int32_t	pad;
	int32_t	i;

	pad = lm_detail()->title_pad;
	left = g->caption.x + pad;
	if (g->sysmenu.w > 0)
		left = g->sysmenu.x + g->sysmenu.w + pad;
	right = g->caption.x + g->caption.w - pad;
	i = 0;
	while (i < LG_NBTN)
	{
		if ((g->flags & (LG_SHOWN << i)) && g->btn[i].x - pad < right)
			right = g->btn[i].x - pad;
		i++;
	}
	g->title = lp_rect(g->outer.x, g->outer.y, 0, 0);
	if (right > left && g->caption.h > 0)
		g->title = lp_rect(left, g->caption.y, right - left, g->caption.h);
}

static void	zone_client(t_lgeo *g)
{
	t_rect	raw;

	g->client = g->outer;
	if (!(g->flags & LG_FRAMED))
		return ;
	raw = lp_rect(g->outer.x + g->border, g->outer.y + g->cap_end,
			g->outer.w - 2 * g->border, g->outer.h - g->cap_end - g->bottom);
	g->client = lp_isect(raw, g->outer);
	if (g->client.w <= 0 || g->client.h <= 0)
		g->client = lp_rect(g->outer.x, g->outer.y, 0, 0);
}

void	lg_zones(t_lgeo *g)
{
	zone_sysmenu(g);
	zone_title(g);
	zone_client(g);
}
