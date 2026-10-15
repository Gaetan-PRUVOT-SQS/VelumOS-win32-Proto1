#include "gfx_int.h"

void	gfx_dda_make(t_dda *d, int64_t m, int64_t step)
{
	d->m = m;
	d->sq = step / m;
	d->sr = step % m;
	if (d->sr < 0)
	{
		d->sr += m;
		d->sq--;
	}
	d->q = 0;
	d->r = 0;
}

void	gfx_dda_seek(t_dda *d, int64_t num)
{
	d->q = num / d->m;
	d->r = num % d->m;
}

void	gfx_dda_next(t_dda *d)
{
	d->q += d->sq;
	d->r += d->sr;
	if (d->r >= d->m)
	{
		d->r -= d->m;
		d->q++;
	}
}
