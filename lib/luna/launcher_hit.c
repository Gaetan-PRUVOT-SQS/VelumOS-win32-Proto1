#include "launcher_int.h"

int32_t	luna_launcher_hit(const t_launcherlayout *l, int32_t x, int32_t y)
{
	t_point	p;
	int32_t	i;

	if (l == NULL)
		return (LUNA_LAUNCHER_NONE);
	p = lp_pt(x, y);
	i = 0;
	while (i < l->ntiles)
	{
		if (lp_in(l->tile[i], p))
			return (i);
		i++;
	}
	if (lp_in(l->logoff, p))
		return (LUNA_LAUNCHER_LOGOFF);
	if (lp_in(l->poweroff, p))
		return (LUNA_LAUNCHER_POWER);
	return (LUNA_LAUNCHER_NONE);
}

t_rect	luna_launcher_place(int32_t screen_h, int32_t bar_h)
{
	int32_t	y;

	y = lp_max(screen_h - bar_h - LUNA_LAUNCHER_HEIGHT, 0);
	return (lp_rect(LUNA_LAUNCHER_MARGIN_X, y, LUNA_LAUNCHER_WIDTH,
			LUNA_LAUNCHER_HEIGHT));
}

t_rect	luna_launcher_place_in(int32_t screen_w, int32_t screen_h,
			int32_t bar_h)
{
	int32_t	w;
	int32_t	h;
	int32_t	bottom;

	bottom = lp_max(screen_h - lp_max(bar_h, 0), 0);
	w = lp_clamp(screen_w - 2 * LUNA_LAUNCHER_MARGIN_X, 0,
			LUNA_LAUNCHER_WIDTH);
	h = lp_min(bottom, LUNA_LAUNCHER_HEIGHT);
	return (lp_rect(LUNA_LAUNCHER_MARGIN_X, bottom - h, w, h));
}
