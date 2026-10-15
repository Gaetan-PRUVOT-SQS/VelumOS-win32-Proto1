#include "gfx_int.h"

void	gfx_ramp_next(t_ramp *rp)
{
	int	ch;

	ch = 0;
	while (ch < 4)
	{
		gfx_dda_next(&rp->c[ch]);
		ch++;
	}
}

t_color	gfx_ramp_color(const t_ramp *rp)
{
	return (((t_color)rp->c[3].q << 24) | ((t_color)rp->c[2].q << 16)
		| ((t_color)rp->c[1].q << 8) | (t_color)rp->c[0].q);
}
