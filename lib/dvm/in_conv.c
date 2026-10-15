#include "in_int.h"

static uint32_t	in_un32(uint8_t op, uint32_t x)
{
	if (op == OP_NEG_INT)
		return (0u - x);
	if (op == OP_NOT_INT)
		return (~x);
	if (op == OP_INT_TO_BYTE)
		return ((uint32_t)(int32_t)(int8_t)x);
	if (op == OP_INT_TO_CHAR)
		return ((uint16_t)x);
	return ((uint32_t)(int32_t)(int16_t)x);
}

int	in_op_un_int(t_in *in)
{
	uint32_t	x;
	uint8_t		op;

	x = in_get(in, in->i.b);
	op = in->i.op;
	if (op == OP_INT_TO_LONG)
		in_setw(in, in->i.a, (uint64_t)(int64_t)(int32_t)x);
	else if (op == OP_INT_TO_DOUBLE)
		in_setw(in, in->i.a, in_f64b((double)(int32_t)x));
	else if (op == OP_INT_TO_FLOAT)
		in_set(in, in->i.a, in_f32b((float)(int32_t)x));
	else
		in_set(in, in->i.a, in_un32(op, x));
	return (0);
}

int	in_op_un_long(t_in *in)
{
	uint64_t	x;
	uint8_t		op;

	x = in_getw(in, in->i.b);
	op = in->i.op;
	if (op == OP_NEG_LONG)
		in_setw(in, in->i.a, 0u - x);
	else if (op == OP_NOT_LONG)
		in_setw(in, in->i.a, ~x);
	else if (op == OP_LONG_TO_INT)
		in_set(in, in->i.a, (uint32_t)x);
	else if (op == OP_LONG_TO_FLOAT)
		in_set(in, in->i.a, in_f32b((float)(int64_t)x));
	else
		in_setw(in, in->i.a, in_f64b((double)(int64_t)x));
	return (0);
}

int	in_op_un_float(t_in *in)
{
	uint32_t	x;
	double		d;
	uint8_t		op;

	x = in_get(in, in->i.b);
	d = (double)in_f32(x);
	op = in->i.op;
	if (op == OP_NEG_FLOAT)
		in_set(in, in->i.a, x ^ 0x80000000u);
	else if (op == OP_FLOAT_TO_INT)
		in_set(in, in->i.a, (uint32_t)in_d2i(d));
	else if (op == OP_FLOAT_TO_LONG)
		in_setw(in, in->i.a, (uint64_t)in_d2l(d));
	else
		in_setw(in, in->i.a, in_f64b(d));
	return (0);
}

int	in_op_un_double(t_in *in)
{
	uint64_t	x;
	double		d;
	uint8_t		op;

	x = in_getw(in, in->i.b);
	d = in_f64(x);
	op = in->i.op;
	if (op == OP_NEG_DOUBLE)
		in_setw(in, in->i.a, x ^ (1ull << 63));
	else if (op == OP_DOUBLE_TO_INT)
		in_set(in, in->i.a, (uint32_t)in_d2i(d));
	else if (op == OP_DOUBLE_TO_LONG)
		in_setw(in, in->i.a, (uint64_t)in_d2l(d));
	else
		in_set(in, in->i.a, in_f32b((float)d));
	return (0);
}
