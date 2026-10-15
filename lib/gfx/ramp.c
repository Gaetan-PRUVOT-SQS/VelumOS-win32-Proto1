#include "gfx_int.h"

static void	ramp_channel(t_ramp *rp, int ch, int64_t from, int64_t to)
{
	rp->from[ch] = from;
	rp->delta[ch] = to - from;
	if (rp->den == 0)
		rp->delta[ch] = 0;
	gfx_dda_make(&rp->c[ch], 2 * gfx_max64(rp->den, 1), 2 * rp->delta[ch]);
}

void	gfx_ramp_init(t_ramp *rp, t_color from, t_color to, int64_t n)
{
	int	ch;

	rp->den = gfx_max64(n - 1, 0);
	ch = 0;
	while (ch < 4)
	{
		ramp_channel(rp, ch, (from >> (8 * ch)) & 255, (to >> (8 * ch)) & 255);
		ch++;
	}
}

void	gfx_ramp_seek(t_ramp *rp, int64_t i)
{
	int64_t	den;
	int		ch;

	den = gfx_max64(rp->den, 1);
	ch = 0;
	while (ch < 4)
	{
		gfx_dda_seek(&rp->c[ch], 2 * rp->from[ch] * den
			+ 2 * rp->delta[ch] * i + den);
		ch++;
	}
}
