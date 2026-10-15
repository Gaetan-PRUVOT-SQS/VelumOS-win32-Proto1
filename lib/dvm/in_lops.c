#include "in_int.h"

static uint64_t	in_lsimple(uint32_t k, uint64_t x, uint64_t y)
{
	if (k == 0)
		return (x + y);
	if (k == 1)
		return (x - y);
	if (k == 2)
		return (x * y);
	if (k == 5)
		return (x & y);
	if (k == 6)
		return (x | y);
	if (k == 7)
		return (x ^ y);
	if (k == 8)
		return (x << (y & 63));
	if (k == 9)
		return ((uint64_t)((int64_t)x >> (y & 63)));
	return (x >> (y & 63));
}

int	in_lop(t_in *in, uint32_t k, uint64_t *x, uint64_t y)
{
	int64_t	a;

	a = (int64_t)(*x);
	if (k != 3 && k != 4)
	{
		*x = in_lsimple(k, *x, y);
		return (0);
	}
	if (y == 0)
		return (in_throw(in, IN_ARITH));
	if (y == UINT64_MAX && k == 3)
		*x = 0u - *x;
	else if (y == UINT64_MAX)
		*x = 0;
	else if (k == 3)
		*x = (uint64_t)(a / (int64_t)y);
	else
		*x = (uint64_t)(a % (int64_t)y);
	return (0);
}

int	in_op_lbin(t_in *in)
{
	uint64_t	v[2];
	uint32_t	src[2];
	uint32_t	k;
	int			rc;

	src[0] = in->i.b;
	src[1] = in->i.c;
	if (in->i.op >= OP_ADD_LONG_2ADDR)
	{
		src[0] = in->i.a;
		src[1] = in->i.b;
	}
	k = (in->i.op - OP_ADD_LONG) % 32;
	v[0] = in_getw(in, src[0]);
	if (k >= 8)
		v[1] = in_get(in, src[1]);
	else
		v[1] = in_getw(in, src[1]);
	rc = in_lop(in, k, &v[0], v[1]);
	if (rc == 0)
		in_setw(in, in->i.a, v[0]);
	return (rc);
}
