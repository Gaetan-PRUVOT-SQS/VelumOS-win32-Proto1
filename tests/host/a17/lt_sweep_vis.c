#include "lt_hit.h"

bool	lt_sweep_visit(const t_lunawin *w, t_point p, int32_t step)
{
	int32_t	dx;
	int32_t	dy;
	bool	near_x;
	bool	near_y;

	dx = p.x - w->outer.x;
	dy = p.y - w->outer.y;
	if (step <= 1)
		return (true);
	near_x = dx < LT_BAND_X || dx >= w->outer.w - LT_BAND_BTN;
	near_y = dy < LT_BAND || dy >= w->outer.h - LT_BAND;
	if (near_x && near_y)
		return (true);
	if (near_x)
		return (dy % step == 0);
	if (near_y)
		return (dx % step == 0);
	return (dx % step == 0 && dy % step == 0);
}
