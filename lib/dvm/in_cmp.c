#include "in_int.h"

int32_t	in_d2i(double d)
{
	if (d != d)
		return (0);
	if (d >= 2147483647.0)
		return (INT32_MAX);
	if (d <= -2147483648.0)
		return (INT32_MIN);
	return ((int32_t)d);
}

int64_t	in_d2l(double d)
{
	if (d != d)
		return (0);
	if (d >= 9223372036854775808.0)
		return (INT64_MAX);
	if (d <= -9223372036854775808.0)
		return (INT64_MIN);
	return ((int64_t)d);
}

static int32_t	in_cmp_fp(double x, double y, int32_t bias)
{
	if (x != x || y != y)
		return (bias);
	return ((x > y) - (x < y));
}

static int32_t	in_cmp_long(int64_t x, int64_t y)
{
	return ((x > y) - (x < y));
}

int	in_op_cmp(t_in *in)
{
	int32_t	r;
	int32_t	bias;
	uint8_t	op;

	op = in->i.op;
	bias = -1;
	if (op == OP_CMPG_FLOAT || op == OP_CMPG_DOUBLE)
		bias = 1;
	if (op == OP_CMP_LONG)
		r = in_cmp_long((int64_t)in_getw(in, in->i.b),
				(int64_t)in_getw(in, in->i.c));
	else if (op >= OP_CMPL_DOUBLE)
		r = in_cmp_fp(in_f64(in_getw(in, in->i.b)),
				in_f64(in_getw(in, in->i.c)), bias);
	else
		r = in_cmp_fp((double)in_f32(in_get(in, in->i.b)),
				(double)in_f32(in_get(in, in->i.c)), bias);
	in_set(in, in->i.a, (uint32_t)r);
	return (0);
}
