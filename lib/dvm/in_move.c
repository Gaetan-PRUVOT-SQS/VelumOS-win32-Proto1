#include "in_int.h"

int	in_op_move(t_in *in)
{
	if (in->i.op >= OP_MOVE_WIDE && in->i.op <= OP_MOVE_WIDE_16)
		in_setw(in, in->i.a, in_getw(in, in->i.b));
	else
		in_set(in, in->i.a, in_get(in, in->i.b));
	return (0);
}

int	in_op_move_result(t_in *in)
{
	if (in->i.op == OP_MOVE_RESULT_WIDE)
		in_setw(in, in->i.a, in->vm->result);
	else
		in_set(in, in->i.a, (uint32_t)in->vm->result);
	return (0);
}

int	in_op_move_exception(t_in *in)
{
	in_set(in, in->i.a, in->vm->pending);
	in->vm->pending = DVM_NULL;
	return (0);
}

int	in_op_return(t_in *in)
{
	uint64_t	v;

	v = 0;
	if (in->i.op == OP_RETURN_WIDE)
		v = in_getw(in, in->i.a);
	else if (in->i.op != OP_RETURN_VOID)
		v = in_get(in, in->i.a);
	if (in->bad)
		return (0);
	in->vm->result = v;
	in_pop(in);
	return (0);
}

int	in_op_const(t_in *in)
{
	if (in->i.op >= OP_CONST_WIDE_16)
		in_setw(in, in->i.a, (uint64_t)in->i.lit);
	else
		in_set(in, in->i.a, (uint32_t)in->i.lit);
	return (0);
}
