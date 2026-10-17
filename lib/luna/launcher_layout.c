#include "launcher_int.h"

static void	layout_tiles(t_launcherlayout *out, int32_t y)
{
	int32_t	span;
	int32_t	left;
	int32_t	right;
	int32_t	i;

	span = out->search.w + LL_GAP;
	i = 0;
	while (i < LUNA_LAUNCHER_TILES)
	{
		out->tile[i] = lp_rect(0, 0, 0, 0);
		left = out->search.x + i * span / LUNA_LAUNCHER_TILES;
		right = out->search.x + (i + 1) * span / LUNA_LAUNCHER_TILES - LL_GAP;
		if (i < out->ntiles)
			out->tile[i] = lp_rect(left, y, lp_max(right - left, 0),
					LL_TILE_H);
		i++;
	}
}

static void	layout_foot(t_launcherlayout *out, int32_t fy)
{
	t_rect	p;
	int32_t	name_x;

	p = out->panel;
	out->avatar = lp_rect(p.x + LL_PAD + LL_FOOT_PAD,
			fy + (LL_FOOT_H - LL_AVATAR) / 2, LL_AVATAR, LL_AVATAR);
	out->poweroff = lp_rect(p.x + p.w - LL_PAD - LL_FOOT_PAD - LL_FOOT_H, fy,
			LL_FOOT_H, LL_FOOT_H);
	out->logoff = lp_rect(out->poweroff.x - LL_FOOT_PAD - LL_LOGOFF_W, fy,
			LL_LOGOFF_W, LL_FOOT_H);
	name_x = out->avatar.x + LL_AVATAR + LL_FOOT_PAD;
	out->user = lp_rect(name_x, fy,
			lp_max(out->logoff.x - LL_FOOT_PAD - name_x, 0), LL_FOOT_H);
}

void	luna_launcher_layout(t_rect panel, int32_t ntiles,
			t_launcherlayout *out)
{
	int32_t	inner;
	int32_t	fy;

	if (out == NULL)
		return ;
	panel = lp_clean(panel);
	inner = lp_max(panel.w - 2 * LL_PAD, 0);
	fy = panel.y + lp_max(panel.h - LL_PAD - LL_FOOT_H, 0);
	out->panel = panel;
	out->ntiles = lp_clamp(ntiles, 0, LUNA_LAUNCHER_TILES);
	out->search = lp_rect(panel.x + LL_PAD, panel.y + LL_PAD, inner,
			LL_SEARCH_H);
	out->title = lp_rect(out->search.x, out->search.y + LL_SEARCH_H + LL_PAD,
			inner, LL_TITLE_H);
	out->sep = lp_rect(out->search.x, fy - LL_FOOT_GAP, inner, 1);
	layout_tiles(out, out->title.y + LL_TITLE_H + LL_TITLE_GAP);
	layout_foot(out, fy);
}
