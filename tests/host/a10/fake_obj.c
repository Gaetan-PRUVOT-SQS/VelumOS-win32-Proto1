#include <stdlib.h>
#include "fake.h"

t_fobj	g_fobj;

t_object	*obj_create(uint32_t type, const t_objops *ops, void *impl)
{
	t_fakeobj	*f;

	if (g_fobj.fail_create)
		return (NULL);
	f = calloc(1, sizeof(*f));
	if (!f)
		return (NULL);
	f->obj.type = type;
	f->obj.refs = 1;
	f->obj.ops = ops;
	f->obj.impl = impl;
	g_fobj.live++;
	return (&f->obj);
}

void	obj_ref(t_object *obj)
{
	obj->refs++;
}

void	obj_unref(t_object *obj)
{
	obj->refs--;
	if (obj->refs != 0)
		return ;
	obj->ops->destroy(obj);
	g_fobj.live--;
	free(obj);
}

void	obj_signal(t_object *obj)
{
	((t_fakeobj *)obj)->signals++;
}

int	fobj_signals(const t_object *o)
{
	return (((const t_fakeobj *)o)->signals);
}
