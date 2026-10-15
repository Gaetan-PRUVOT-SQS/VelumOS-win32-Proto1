#include "rt_int.h"

int	dvm_new(t_dvm *vm, t_dclass *c, t_dref *out)
{
	t_dnewobj	rq;

	if (!vm || !c || !out)
		return (E_INVAL);
	if (c->is_array || (c->access & (ACC_ABSTRACT | ACC_INTERFACE))
		|| c == vm->classes->core[DCORE_STRING])
		return (E_INVAL);
	rq.cls = c;
	rq.len = c->words;
	rq.bytes = (uint64_t)c->pay_end + (uint64_t)c->words * 4u;
	rq.kind = DOK_OBJECT;
	return (rt_alloc(vm, &rq, out));
}

t_dclass	*dvm_class_of(t_dvm *vm, t_dref ref)
{
	t_dobj	*o;

	o = rt_obj(vm, ref, DOK_ANY);
	if (!o)
		return (NULL);
	return (o->cls);
}

void	*dvm_payload(t_dvm *vm, t_dref ref, const t_dclass *builtin)
{
	t_dobj	*o;

	o = rt_obj(vm, ref, DOK_OBJECT);
	if (!o || !builtin || builtin->pay_end == builtin->pay_off)
		return (NULL);
	if (!rt_assignable(o->cls, builtin))
		return (NULL);
	return ((uint8_t *)o->data + builtin->pay_off);
}
