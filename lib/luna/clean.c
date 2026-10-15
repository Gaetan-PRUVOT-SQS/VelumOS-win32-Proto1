#include "luna_int.h"

bool	lp_ok(const t_surface *s)
{
	if (s == NULL || s->px == NULL || s->w <= 0 || s->h <= 0)
		return (false);
	if (s->w > LP_COORD_MAX || s->h > LP_COORD_MAX)
		return (false);
	return (s->stride >= s->w);
}

bool	lp_pt_ok(t_point p)
{
	return (p.x >= -LP_COORD_MAX && p.x <= LP_COORD_MAX && p.y >= -LP_COORD_MAX
		&& p.y <= LP_COORD_MAX);
}

t_rect	lp_clean(t_rect r)
{
	if (r.x < -LP_COORD_MAX || r.x > LP_COORD_MAX || r.y < -LP_COORD_MAX
		|| r.y > LP_COORD_MAX || r.w <= 0 || r.h <= 0)
		return (lp_rect(0, 0, 0, 0));
	return (lp_rect(r.x, r.y, lp_min(r.w, LP_COORD_MAX),
			lp_min(r.h, LP_COORD_MAX)));
}
