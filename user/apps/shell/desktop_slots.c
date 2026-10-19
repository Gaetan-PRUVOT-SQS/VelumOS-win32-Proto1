#include "../common/layout.h"
#include "desktop.h"

int	desk_slots(t_rect area, t_rect *out, uint32_t max)
{
	int32_t		rows;
	int32_t		cols;
	uint32_t	i;
	uint32_t	n;
	t_rect		r;

	rows = desk_rows(area);
	cols = (area.w - DESK_MARGIN) / DESK_CELL_W;
	n = 0;
	i = 0;
	while (rows > 0 && cols > 0 && i < max && i < (uint32_t)(rows * cols))
	{
		r = lay_rect(area.x + DESK_MARGIN + (int32_t)i / rows * DESK_CELL_W,
				area.y + DESK_MARGIN + (int32_t)i % rows * DESK_STEP_Y,
				DESK_CELL_W, DESK_CELL_H);
		if (lay_inside(area, r))
			out[n++] = r;
		i++;
	}
	return ((int)n);
}

int32_t	desk_slot_near(const t_rect *slots, int32_t n, int32_t x, int32_t y)
{
	int32_t	best;
	int32_t	i;
	int64_t	d;
	int64_t	min;
	int64_t	dx;
	int64_t	dy;

	best = DESK_NONE;
	min = 0;
	i = 0;
	while (i < n)
	{
		dx = (int64_t)slots[i].x + slots[i].w / 2 - x;
		dy = (int64_t)slots[i].y + slots[i].h / 2 - y;
		d = dx * dx + dy * dy;
		if (best == DESK_NONE || d < min)
		{
			best = i;
			min = d;
		}
		i++;
	}
	return (best);
}
