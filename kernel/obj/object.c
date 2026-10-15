#include "obj_int.h"
#include "velum/heap.h"
#include "velum/panic.h"

t_object	*obj_create(uint32_t type, const t_objops *ops, void *impl)
{
	t_object	*obj;

	if (type == OBJ_NONE || type >= OBJ_TYPES || !ops)
		return (NULL);
	obj = kmalloc_tag(sizeof(t_object), HEAP_OBJECT);
	if (!obj)
		return (NULL);
	obj->type = type;
	obj->refs = 1;
	obj->ops = ops;
	obj->impl = impl;
	spin_init(&obj->lock, "obj");
	waitq_init(&obj->waiters);
	return (obj);
}

void	obj_ref(t_object *obj)
{
	uint32_t	old;

	old = __atomic_fetch_add(&obj->refs, 1, __ATOMIC_RELAXED);
	kassert_check(old != 0 && old < OBJ_REFS_MAX, "obj: reference invalide");
}

bool	obj_tryref(t_object *obj)
{
	uint32_t	old;

	if (!obj)
		return (false);
	old = __atomic_load_n(&obj->refs, __ATOMIC_RELAXED);
	while (old != 0)
	{
		kassert_check(old < OBJ_REFS_MAX, "obj: compteur sature");
		if (__atomic_compare_exchange_n(&obj->refs, &old, old + 1, false,
				__ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
			return (true);
	}
	return (false);
}

void	obj_unref(t_object *obj)
{
	uint32_t	old;

	if (!obj)
		return ;
	old = __atomic_fetch_sub(&obj->refs, 1, __ATOMIC_ACQ_REL);
	kassert_check(old != 0, "obj: unref de trop");
	if (old != 1)
		return ;
	if (obj->ops->destroy)
		obj->ops->destroy(obj);
	kfree(obj);
}

void	obj_signal(t_object *obj)
{
	if (!obj)
		return ;
	wreg_notify(obj);
	waitq_wake_all(&obj->waiters);
}
