#include "rt_int.h"

int	dvm_local(t_dvm *vm, t_dref ref)
{
	if (!vm || (ref != DVM_NULL && !rt_obj(vm, ref, DOK_ANY)))
		return (E_INVAL);
	if (vm->nlocals >= DVM_LOCALS_MAX)
		return (E_RANGE);
	vm->locals[vm->nlocals++] = ref;
	return (0);
}

void	dvm_local_pop(t_dvm *vm, uint32_t n)
{
	if (!vm)
		return ;
	if (n > vm->nlocals)
		n = vm->nlocals;
	vm->nlocals -= n;
}

int	dvm_pin(t_dvm *vm, t_dref ref)
{
	t_dobj	*o;

	o = rt_obj(vm, ref, DOK_ANY);
	if (!o)
		return (E_INVAL);
	if (o->pins == UINT16_MAX)
		return (E_RANGE);
	o->pins++;
	return (0);
}

void	dvm_unpin(t_dvm *vm, t_dref ref)
{
	t_dobj	*o;

	o = rt_obj(vm, ref, DOK_ANY);
	if (o && o->pins > 0)
		o->pins--;
}
