#include "obj_int.h"

void	obj_task_exited(t_object *o)
{
	t_taskobj	*t;

	if (!o || (o->type != OBJ_PROCESS && o->type != OBJ_THREAD))
		return ;
	t = o->impl;
	__atomic_store_n(&t->dead, 1, __ATOMIC_RELEASE);
	obj_signal(o);
}

t_process	*obj_process_of(t_object *o)
{
	t_taskobj	*t;

	if (!o || o->type != OBJ_PROCESS)
		return (NULL);
	t = o->impl;
	return (t->proc);
}

t_thread	*obj_thread_of(t_object *o)
{
	t_taskobj	*t;

	if (!o || o->type != OBJ_THREAD)
		return (NULL);
	t = o->impl;
	return (t->thread);
}
