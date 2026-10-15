#include "in_int.h"

uint64_t	in_narrow(uint32_t kind, uint64_t v)
{
	if (kind == DAK_WIDE)
		return (v);
	if (kind == DAK_BOOLEAN)
		return ((uint8_t)v);
	if (kind == DAK_BYTE)
		return ((uint32_t)(int32_t)(int8_t)v);
	if (kind == DAK_CHAR)
		return ((uint16_t)v);
	if (kind == DAK_SHORT)
		return ((uint32_t)(int32_t)(int16_t)v);
	return ((uint32_t)v);
}

static int	in_put_all(t_in *in, t_dref arr, const uint32_t *args, uint32_t n)
{
	t_darrview	v;
	t_daacc		acc;
	int			rc;

	rc = dvm_array_view(in->vm, arr, &v);
	acc = (t_daacc){arr, 0, 0, DAK_INT, 1};
	if (rc == 0 && v.is_ref)
		acc.kind = DAK_OBJECT;
	while (rc == 0 && (uint32_t)acc.index < n)
	{
		acc.val = args[acc.index];
		rc = dvmrt_array(in->vm, &acc);
		acc.index++;
	}
	return (rc);
}

int	in_op_filled(t_in *in)
{
	t_dnewarr		rq;
	const uint32_t	*args;
	uint32_t		n;
	int				rc;

	args = in_gather(in, &n);
	if (in->bad)
		return (0);
	rq = (t_dnewarr){in->i.idx, (int32_t)n, DVM_NULL};
	rc = dvmrt_new_array(in->vm, in->m, &rq);
	if (rc == 0)
		rc = in_put_all(in, rq.out, args, n);
	if (rc == 0)
		in->vm->result = rq.out;
	return (rc);
}
