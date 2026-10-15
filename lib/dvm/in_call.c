#include "in_int.h"

int	in_enter(t_in *in, const t_dmethod *m, const uint32_t *args)
{
	if (m->native)
		return (in_native(in, m, args));
	if (!m->insns.p || m->insns.len < 2
		|| (m->access & (ACC_ABSTRACT | ACC_NATIVE)))
		return (in_throw(in, IN_ABSTRACT));
	return (in_push(in, m, args));
}

static int	in_step(t_in *in)
{
	t_dvm	*vm;
	int		rc;

	vm = in->vm;
	if (vm->lim.budget && vm->spent >= vm->lim.budget)
		return (E_TIMEOUT);
	vm->spent++;
	if (dexcode_decode(in->m->insns, in->pc, &in->i) < 0)
		return (E_INVAL);
	in->next = in->pc + in->i.len;
	rc = in_op(in->i.op)(in);
	if (rc == 0 && in->bad)
		rc = in_throw(in, IN_VERIFY);
	in->bad = 0;
	if (rc == 0)
		in->pc = in->next;
	return (rc);
}

static int	in_run(t_in *in)
{
	int	rc;

	rc = 0;
	while (rc == 0 && !in->done)
	{
		rc = in_step(in);
		if (rc > 0)
			rc = in_unwind(in);
	}
	return (rc);
}

int	dvm_call(t_dvm *vm, const t_dmethod *m, const uint32_t *args,
		uint64_t *ret)
{
	t_in		in;
	uint32_t	mark[2];
	int			rc;

	if (!vm || !m || !vm->stack)
		return (E_INVAL);
	__builtin_memset(&in, 0, sizeof(in));
	in.vm = vm;
	mark[0] = vm->sp;
	mark[1] = vm->depth;
	rc = in_enter(&in, m, args);
	in.fp0 = in.fp;
	if (rc == 0 && in.m)
		rc = in_run(&in);
	vm->sp = mark[0];
	vm->depth = mark[1];
	if (rc == 0 && ret)
		*ret = vm->result;
	return (rc);
}
