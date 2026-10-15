#include "in_int.h"

int	in_op_array_length(t_in *in)
{
	t_dref		arr;
	uint32_t	len;
	int			rc;

	arr = in_get(in, in->i.b);
	if (arr == DVM_NULL)
		return (in_throw(in, IN_NPE));
	len = 0;
	rc = dvmrt_array_length(in->vm, arr, &len);
	if (rc == 0)
		in_set(in, in->i.a, len);
	return (rc);
}

int	in_op_new_array(t_in *in)
{
	t_dnewarr	rq;
	int			rc;

	rq = (t_dnewarr){in->i.idx, (int32_t)in_get(in, in->i.b), DVM_NULL};
	if (in->bad)
		return (0);
	rc = dvmrt_new_array(in->vm, in->m, &rq);
	if (rc == 0)
		in_set(in, in->i.a, rq.out);
	return (rc);
}

int	in_op_fill_data(t_in *in)
{
	t_darray	data;
	t_dref		arr;

	arr = in_get(in, in->i.a);
	if (dexcode_array_data(in->m->insns, in->pc + (uint32_t)in->i.lit,
			&data) < 0)
		return (E_INVAL);
	if (arr == DVM_NULL)
		return (in_throw(in, IN_NPE));
	return (dvmrt_fill_array(in->vm, arr, &data));
}

int	in_op_aget(t_in *in)
{
	t_daacc	acc;
	int		rc;

	acc = (t_daacc){in_get(in, in->i.b), (int32_t)in_get(in, in->i.c), 0,
		(uint8_t)(in->i.op - OP_AGET), 0};
	if (in->bad)
		return (0);
	if (acc.arr == DVM_NULL)
		return (in_throw(in, IN_NPE));
	rc = dvmrt_array(in->vm, &acc);
	if (rc != 0)
		return (rc);
	if (acc.kind == DAK_WIDE)
		in_setw(in, in->i.a, acc.val);
	else
		in_set(in, in->i.a, (uint32_t)in_narrow(acc.kind, acc.val));
	return (0);
}

int	in_op_aput(t_in *in)
{
	t_daacc	acc;

	acc = (t_daacc){in_get(in, in->i.b), (int32_t)in_get(in, in->i.c), 0,
		(uint8_t)(in->i.op - OP_APUT), 1};
	if (acc.kind == DAK_WIDE)
		acc.val = in_getw(in, in->i.a);
	else
		acc.val = in_narrow(acc.kind, in_get(in, in->i.a));
	if (in->bad)
		return (0);
	if (acc.arr == DVM_NULL)
		return (in_throw(in, IN_NPE));
	return (dvmrt_array(in->vm, &acc));
}
