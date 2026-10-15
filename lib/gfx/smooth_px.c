#include "gfx_int.h"

void	gfx_tap_axis(t_tap_axis *a, int64_t pos256, int64_t lo, int64_t hi)
{
	int64_t	whole;

	whole = pos256 / 256;
	if (pos256 % 256 < 0)
		whole--;
	a->frac = (uint32_t)(pos256 - whole * 256);
	a->i0 = gfx_max64(lo, gfx_min64(hi, whole));
	a->i1 = gfx_max64(lo, gfx_min64(hi, whole + 1));
}

t_color	gfx_bilinear(const uint32_t p[4], const uint32_t w[4])
{
	uint32_t	out;
	uint32_t	v;
	int			ch;
	int			k;

	out = 0;
	ch = 0;
	while (ch < 4)
	{
		v = 32768;
		k = 0;
		while (k < 4)
		{
			v += ((p[k] >> (8 * ch)) & 255) * w[k];
			k++;
		}
		out |= (v >> 16) << (8 * ch);
		ch++;
	}
	return (out);
}
