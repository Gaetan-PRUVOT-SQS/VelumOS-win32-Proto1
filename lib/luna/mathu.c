#include "luna_int.h"

int32_t	lp_min(int32_t a, int32_t b)
{
	if (a < b)
		return (a);
	return (b);
}

int32_t	lp_max(int32_t a, int32_t b)
{
	if (a > b)
		return (a);
	return (b);
}

int32_t	lp_clamp(int32_t v, int32_t lo, int32_t hi)
{
	if (v < lo)
		return (lo);
	if (v > hi)
		return (hi);
	return (v);
}

int32_t	lp_floor16(int32_t v)
{
	int32_t	q;

	q = v / 16;
	if (v % 16 < 0)
		q--;
	return (q);
}

int32_t	lp_round16(int32_t v)
{
	return (lp_floor16(v + 8));
}
