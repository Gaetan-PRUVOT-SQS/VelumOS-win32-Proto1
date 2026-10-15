#include "rt_int.h"

void	rt_oom_retype(t_dvm *vm)
{
	t_dclass	*c;
	t_dclass	*base;
	t_dobj		*o;

	c = rt_find(vm, "Ljava/lang/OutOfMemoryError;");
	base = vm->classes->core[DCORE_THROWABLE];
	o = rt_obj(vm, vm->heap->oom, DOK_OBJECT);
	if (!c || !o || o->cls != base || !rt_assignable(c, base))
		return ;
	if (c->words != base->words || c->pay_end != base->pay_end)
		return ;
	o->cls = c;
}
