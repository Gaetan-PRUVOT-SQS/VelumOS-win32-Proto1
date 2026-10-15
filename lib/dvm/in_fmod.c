#include "in_int.h"

static uint64_t	in_norm(uint64_t bits, int32_t *e)
{
	uint64_t	m;

	*e = (int32_t)((bits >> 52) & 0x7ff);
	m = bits & 0xfffffffffffffull;
	if (*e != 0)
		return (m | (1ull << 52));
	*e = 1;
	while (m != 0 && m < (1ull << 52))
	{
		m <<= 1;
		(*e)--;
	}
	return (m);
}

static uint64_t	in_reduce(uint64_t mx, uint64_t my, int32_t n)
{
	while (n > 0)
	{
		if (mx >= my)
			mx -= my;
		mx <<= 1;
		n--;
	}
	if (mx >= my)
		mx -= my;
	return (mx);
}

static uint64_t	in_pack(uint64_t m, int32_t e, uint64_t sign)
{
	if (m == 0)
		return (sign);
	while (m < (1ull << 52))
	{
		m <<= 1;
		e--;
	}
	if (e > 0)
		return (sign | ((uint64_t)e << 52) | (m - (1ull << 52)));
	if (e < -62)
		return (sign);
	return (sign | (m >> (1 - e)));
}

double	in_fmod(double x, double y)
{
	uint64_t	b[2];
	uint64_t	m[2];
	int32_t		e[2];

	b[0] = in_f64b(x);
	b[1] = in_f64b(y);
	if ((b[1] << 1) == 0 || (b[0] << 1) >= (0x7ffull << 53)
		|| (b[1] << 1) > (0x7ffull << 53))
		return (in_f64(0x7ff8000000000000ull));
	if ((b[0] << 1) < (b[1] << 1))
		return (x);
	m[0] = in_norm(b[0], &e[0]);
	m[1] = in_norm(b[1], &e[1]);
	m[0] = in_reduce(m[0], m[1], e[0] - e[1]);
	return (in_f64(in_pack(m[0], e[1], b[0] & (1ull << 63))));
}
