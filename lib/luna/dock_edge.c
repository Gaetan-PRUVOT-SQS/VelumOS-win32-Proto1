#include "dock_int.h"

static int32_t	cov_ring(const t_ldplate *p, int32_t x, int32_t y)
{
	t_ldplate	in;

	in = *p;
	in.r = lp_rect(p->r.x + p->thick, p->r.y + p->thick,
			p->r.w - 2 * p->thick, p->r.h - 2 * p->thick);
	in.rad = p->rad - p->thick;
	return (ld_cov(p, x, y) - ld_cov(&in, x, y));
}

static int32_t	cov_rim(const t_ldplate *p, int32_t x, int32_t y)
{
	t_ldplate	low;

	low = *p;
	low.r.y += 1;
	return (ld_cov(p, x, y) - ld_cov(&low, x, y));
}

void	ld_ring(t_surface *s, const t_ldplate *p)
{
	if (p->thick <= 0)
		return ;
	ld_paint(s, p, cov_ring);
}

void	ld_rim(t_surface *s, const t_ldplate *p)
{
	ld_paint(s, p, cov_rim);
}
