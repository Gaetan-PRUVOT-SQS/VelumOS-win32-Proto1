#include "launcher_int.h"

int32_t	lp_launch_state(const t_launcher *l, int32_t id)
{
	int32_t	st;

	st = 0;
	if (l->hot == id)
		st |= LL_ST_HOT;
	if (l->focus == id)
		st |= LL_ST_FOCUS;
	return (st);
}

static void	paint_tiles(t_surface *s, const t_launcher *l,
				const t_launcherlayout *lay)
{
	int32_t	i;

	i = 0;
	while (i < lay->ntiles)
	{
		lp_launch_tile(s, lay->tile[i], &l->tiles[i], lp_launch_state(l, i));
		i++;
	}
}

void	luna_launcher(t_surface *s, const t_launcher *l, t_rect panel)
{
	t_launcherlayout	lay;
	int32_t				n;

	panel = lp_clean(panel);
	if (!lp_ok(s) || l == NULL || panel.w < LL_MIN_W || panel.h < LL_MIN_H)
		return ;
	n = l->ntiles;
	if (l->tiles == NULL)
		n = 0;
	luna_launcher_layout(panel, n, &lay);
	luna_glass(s, panel, LUNA_GLASS_LAUNCHER);
	lp_launch_search(s, &lay);
	lp_launch_title(s, &lay);
	paint_tiles(s, l, &lay);
	lp_launch_foot(s, l, &lay);
}
