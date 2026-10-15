#include "in_int.h"

static uint32_t	in_isimple(uint32_t k, uint32_t x, uint32_t y)
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
		return (x << (y & 31));
	if (k == 9)
		return ((uint32_t)((int32_t)x >> (y & 31)));
	return (x >> (y & 31));
}

int	in_iop(t_in *in, uint32_t k, uint32_t *x, uint32_t y)
{
	int32_t	a;

	a = (int32_t)(*x);
	if (k != 3 && k != 4)
	{
		*x = in_isimple(k, *x, y);
		return (0);
	}
	if (y == 0)
		return (in_throw(in, IN_ARITH));
	if (y == 0xffffffffu && k == 3)
		*x = 0u - *x;
	else if (y == 0xffffffffu)
		*x = 0;
	else if (k == 3)
		*x = (uint32_t)(a / (int32_t)y);
	else
		*x = (uint32_t)(a % (int32_t)y);
	return (0);
}

int	in_op_ibin(t_in *in)
{
	uint32_t	x;
	uint32_t	src[2];
	int			rc;

	src[0] = in->i.b;
	src[1] = in->i.c;
	if (in->i.op >= OP_ADD_INT_2ADDR)
	{
		src[0] = in->i.a;
		src[1] = in->i.b;
	}
	x = in_get(in, src[0]);
	rc = in_iop(in, (in->i.op - OP_ADD_INT) % 32, &x, in_get(in, src[1]));
	if (rc == 0)
		in_set(in, in->i.a, x);
	return (rc);
}

int	in_op_ilit(t_in *in)
{
	uint32_t	k;
	uint32_t	x;
	uint32_t	y;
	int			rc;

	k = (in->i.op - OP_ADD_INT_LIT16) % 8;
	if (in->i.op >= OP_ADD_INT_LIT8)
		k = in->i.op - OP_ADD_INT_LIT8;
	x = in_get(in, in->i.b);
	y = (uint32_t)in->i.lit;
	if (k == 1)
	{
		x = y;
		y = in_get(in, in->i.b);
	}
	rc = in_iop(in, k, &x, y);
	if (rc == 0)
		in_set(in, in->i.a, x);
	return (rc);
}
