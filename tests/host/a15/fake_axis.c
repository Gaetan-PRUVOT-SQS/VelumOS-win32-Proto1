#include "ref.h"

int64_t	ref_floor_div(__int128 a, __int128 b)
{
	__int128	q;

	q = a / b;
	if (a % b != 0 && ((a < 0) != (b < 0)))
		q--;
	return ((int64_t)q);
}

int64_t	ref_clamp(int64_t v, int64_t lo, int64_t hi)
{
	if (v < lo)
		return (lo);
	if (v > hi)
		return (hi);
	return (v);
}

t_axis	ref_axis(int64_t i, int64_t sw, int64_t dw)
{
	t_axis	a;
	int64_t	pos;

	pos = ref_floor_div(((__int128)(2 * i + 1) * sw - dw) * 128, dw);
	a.i0 = ref_floor_div(pos, 256);
	a.i1 = a.i0 + 1;
	a.frac = pos - a.i0 * 256;
	return (a);
}
