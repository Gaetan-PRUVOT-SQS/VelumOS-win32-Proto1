#include "luna_int.h"

int64_t	lp_poly_area2(const t_lpoly *p)
{
	int64_t	a;
	int32_t	i;
	int32_t	j;

	a = 0;
	i = 0;
	while (i < p->n)
	{
		j = (i + 1) % p->n;
		a += (int64_t)p->p[i].x * p->p[j].y;
		a -= (int64_t)p->p[j].x * p->p[i].y;
		i++;
	}
	return (a);
}

t_rect	lp_poly_box(const t_lpoly *p)
{
	int32_t	lo_x;
	int32_t	hi_x;
	int32_t	lo_y;
	int32_t	hi_y;
	int32_t	i;

	lo_x = p->p[0].x;
	hi_x = lo_x;
	lo_y = p->p[0].y;
	hi_y = lo_y;
	i = 1;
	while (i < p->n)
	{
		lo_x = lp_min(lo_x, p->p[i].x);
		hi_x = lp_max(hi_x, p->p[i].x);
		lo_y = lp_min(lo_y, p->p[i].y);
		hi_y = lp_max(hi_y, p->p[i].y);
		i++;
	}
	return (lp_rect(lp_floor16(lo_x) - 1, lp_floor16(lo_y) - 1,
			lp_floor16(hi_x) - lp_floor16(lo_x) + 3,
			lp_floor16(hi_y) - lp_floor16(lo_y) + 3));
}
