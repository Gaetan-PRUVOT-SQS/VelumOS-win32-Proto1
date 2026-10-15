#include "ws_core.h"

bool	wr_inside(t_rect outer, t_rect inner)
{
	return (inner.x >= outer.x && inner.y >= outer.y
		&& (int64_t)inner.x + inner.w <= (int64_t)outer.x + outer.w
		&& (int64_t)inner.y + inner.h <= (int64_t)outer.y + outer.h);
}

t_rect	wr_offset(t_rect r, int32_t dx, int32_t dy)
{
	r.x += dx;
	r.y += dy;
	return (r);
}
