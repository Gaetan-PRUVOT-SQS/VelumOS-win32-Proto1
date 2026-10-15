#include "in_int.h"

const uint32_t	*in_gather(t_in *in, uint32_t *n)
{
	uint32_t	k;

	*n = in->i.argc;
	if (in->i.fmt == DF_3RC)
	{
		if (in->i.c > in->nreg || *n > in->nreg - in->i.c)
			in->bad = 1;
		if (in->bad)
			return (in->argv);
		return (in->r + in->i.c);
	}
	k = 0;
	while (k < *n && k < 5)
	{
		in->argv[k] = in_get(in, in->i.args[k]);
		k++;
	}
	return (in->argv);
}

int	in_op_invoke(t_in *in)
{
	t_dinvoke		inv;
	const uint32_t	*args;
	uint32_t		n;
	int				rc;

	args = in_gather(in, &n);
	inv = (t_dinvoke){in->i.idx, DVM_NULL, NULL,
		(uint8_t)((in->i.op - OP_INVOKE_VIRTUAL) % 6)};
	if (in->bad || (inv.kind != DIK_STATIC && n == 0))
		return (in_throw(in, IN_VERIFY));
	if (inv.kind != DIK_STATIC)
		inv.self = args[0];
	rc = dvmrt_resolve(in->vm, in->m, &inv);
	if (rc != 0)
		return (rc);
	if (!inv.target || inv.target->ins != n)
		return (in_throw(in, IN_VERIFY));
	return (in_enter(in, inv.target, args));
}

int	in_op_field(t_in *in)
{
	t_dfacc		f;
	uint32_t	k;
	int			rc;

	k = in->i.op - OP_IGET;
	f = (t_dfacc){in->i.idx, DVM_NULL, 0, (k % 7) == 1, (k % 7) == 2,
		(k % 14) >= 7, k >= 14};
	if (!f.is_static)
		f.obj = in_get(in, in->i.b);
	if (f.put && f.wide)
		f.val = in_getw(in, in->i.a);
	else if (f.put)
		f.val = in_narrow(k % 7, in_get(in, in->i.a));
	if (in->bad)
		return (0);
	if (!f.is_static && f.obj == DVM_NULL)
		return (in_throw(in, IN_NPE));
	rc = dvmrt_field(in->vm, in->m, &f);
	if (rc != 0 || f.put)
		return (rc);
	if (f.wide)
		in_setw(in, in->i.a, f.val);
	else
		in_set(in, in->i.a, (uint32_t)in_narrow(k % 7, f.val));
	return (0);
}
