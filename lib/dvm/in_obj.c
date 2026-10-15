#include "in_int.h"

int	in_op_const_ref(t_in *in)
{
	t_dref	ref;
	int		rc;

	ref = DVM_NULL;
	if (in->i.op == OP_CONST_CLASS)
		rc = dvmrt_const_class(in->vm, in->m, in->i.idx, &ref);
	else if (in->i.op == OP_NEW_INSTANCE)
		rc = dvmrt_new_instance(in->vm, in->m, in->i.idx, &ref);
	else
		rc = dvmrt_const_string(in->vm, in->m, in->i.idx, &ref);
	if (rc == 0)
		in_set(in, in->i.a, ref);
	return (rc);
}

int	in_op_monitor(t_in *in)
{
	t_dref	ref;

	ref = in_get(in, in->i.a);
	if (in->bad)
		return (0);
	if (ref == DVM_NULL)
		return (in_throw(in, IN_NPE));
	if (in->i.op != OP_THROW)
		return (0);
	in->vm->pending = ref;
	return (DVM_THROWN);
}

static int	in_isinst(t_in *in, t_dref ref, uint32_t *yes)
{
	t_dref	before;
	int		rc;

	before = in->vm->pending;
	rc = dvmrt_instance_of(in->vm, in->m, in->i.idx, ref);
	*yes = (rc > 0);
	if (rc > 0 && in->vm->pending != before && in->vm->pending != DVM_NULL)
		return (DVM_THROWN);
	if (rc > 0)
		return (0);
	return (rc);
}

int	in_op_check_cast(t_in *in)
{
	t_dref		ref;
	uint32_t	yes;
	int			rc;

	ref = in_get(in, in->i.a);
	if (ref == DVM_NULL)
		return (0);
	rc = in_isinst(in, ref, &yes);
	if (rc == 0 && !yes)
		rc = in_throw(in, IN_CAST);
	return (rc);
}

int	in_op_instance_of(t_in *in)
{
	t_dref		ref;
	uint32_t	yes;
	int			rc;

	ref = in_get(in, in->i.b);
	yes = 0;
	rc = 0;
	if (ref != DVM_NULL)
		rc = in_isinst(in, ref, &yes);
	if (rc == 0)
		in_set(in, in->i.a, yes);
	return (rc);
}
