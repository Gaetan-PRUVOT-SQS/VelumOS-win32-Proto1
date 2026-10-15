#include "desktop.h"

void	dc_reset(t_click *last)
{
	last->index = DESK_NONE;
	last->x = 0;
	last->y = 0;
	last->at_ns = 0;
}

static int	close_enough(const t_click *a, const t_click *b)
{
	int64_t	dx;
	int64_t	dy;

	dx = (int64_t)a->x - b->x;
	dy = (int64_t)a->y - b->y;
	return (dx >= -DBL_SLOP && dx <= DBL_SLOP && dy >= -DBL_SLOP
		&& dy <= DBL_SLOP);
}

int	dc_feed(t_click *last, const t_click *now)
{
	int	same;

	same = last->index != DESK_NONE && last->index == now->index
		&& now->at_ns > last->at_ns
		&& now->at_ns - last->at_ns <= DBL_MAX_NS && close_enough(last, now);
	if (same)
	{
		dc_reset(last);
		return (1);
	}
	*last = *now;
	return (0);
}
