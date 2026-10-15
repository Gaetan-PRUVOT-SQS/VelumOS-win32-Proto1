#include "obj_int.h"
#include "velum/heap.h"

static void	task_destroy(t_object *o)
{
	t_taskobj	*t;

	t = o->impl;
	if (t->proc)
		proc_unref(t->proc);
	if (t->thread)
		thread_unref(t->thread);
	kfree(t);
}

static bool	task_signaled(t_object *o)
{
	t_taskobj	*t;

	t = o->impl;
	if (__atomic_load_n(&t->dead, __ATOMIC_ACQUIRE))
		return (true);
	if (t->proc)
		return (__atomic_load_n(&t->proc->state, __ATOMIC_ACQUIRE)
			== PS_ZOMBIE);
	return (__atomic_load_n(&t->thread->state, __ATOMIC_ACQUIRE)
		== TS_ZOMBIE);
}

static const t_objops	g_proc_ops = {"process", task_destroy, task_signaled};
static const t_objops	g_thread_ops = {"thread", task_destroy, task_signaled};

static t_object	*task_new(uint32_t type, t_process *p, t_thread *th)
{
	t_taskobj		*t;
	t_object		*o;
	const t_objops	*ops;

	t = kmalloc_tag(sizeof(t_taskobj), HEAP_OBJECT);
	if (!t)
		return (NULL);
	t->proc = p;
	t->thread = th;
	t->dead = 0;
	t->pad = 0;
	ops = &g_thread_ops;
	if (type == OBJ_PROCESS)
		ops = &g_proc_ops;
	o = obj_create(type, ops, t);
	if (!o)
	{
		kfree(t);
		return (NULL);
	}
	if (p)
		proc_ref(p);
	if (th)
		thread_ref(th);
	return (o);
}

t_object	*obj_process_new(t_process *p)
{
	if (!p)
		return (NULL);
	return (task_new(OBJ_PROCESS, p, NULL));
}

t_object	*obj_thread_new(t_thread *t)
{
	if (!t)
		return (NULL);
	return (task_new(OBJ_THREAD, NULL, t));
}
