#include "in_int.h"

static double	in_dop(uint32_t k, double x, double y)
{
	if (k == 0)
		return (x + y);
	if (k == 1)
		return (x - y);
	if (k == 2)
		return (x * y);
	if (k == 3)
		return (x / y);
	return (in_fmod(x, y));
}

static float	in_fop(uint32_t k, float x, float y)
{
	if (k == 0)
		return (x + y);
	if (k == 1)
		return (x - y);
	if (k == 2)
		return (x * y);
	if (k == 3)
		return (x / y);
	return ((float)in_fmod((double)x, (double)y));
}

int	in_op_fbin(t_in *in)
{
	uint32_t	src[2];
	float		v;

	src[0] = in->i.b;
	src[1] = in->i.c;
	if (in->i.op >= OP_ADD_FLOAT_2ADDR)
	{
		src[0] = in->i.a;
		src[1] = in->i.b;
	}
	v = in_fop((in->i.op - OP_ADD_FLOAT) % 32, in_f32(in_get(in, src[0])),
			in_f32(in_get(in, src[1])));
	in_set(in, in->i.a, in_f32b(v));
	return (0);
}

int	in_op_dbin(t_in *in)
{
	uint32_t	src[2];
	double		v;

	src[0] = in->i.b;
	src[1] = in->i.c;
	if (in->i.op >= OP_ADD_DOUBLE_2ADDR)
	{
		src[0] = in->i.a;
		src[1] = in->i.b;
	}
	v = in_dop((in->i.op - OP_ADD_DOUBLE) % 32, in_f64(in_getw(in, src[0])),
			in_f64(in_getw(in, src[1])));
	in_setw(in, in->i.a, in_f64b(v));
	return (0);
}
