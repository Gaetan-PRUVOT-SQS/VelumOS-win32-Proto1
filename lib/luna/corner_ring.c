#include "luna_int.h"

void	lp_corner_ring(t_surface *s, const t_lcring *k)
{
	int32_t	ix;
	int32_t	iy;
	int32_t	cov;

	if (s == NULL || k == NULL)
		return ;
	iy = 0;
	while (iy < k->rad)
	{
		ix = 0;
		while (ix < k->rad)
		{
			cov = lp_corner_cov(k->rad, ix, iy);
			if (ix > 0 && iy > 0)
				cov -= lp_corner_cov(k->rad - 1, ix - 1, iy - 1);
			lp_plot(s, lp_pt(k->x + k->dx * ix, k->y + k->dy * iy), k->c,
				(uint32_t)lp_clamp(cov, 0, 16));
			ix++;
		}
		iy++;
	}
}
