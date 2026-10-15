#include <stdlib.h>
#include "fake.h"

static t_process	g_fake_proc = {7, 1, "essai", PF_FSWRITE, 0, 0, 1, 1,
	NULL, NULL, NULL, NULL, 0, 0, 0, 0, {0, 0, "p"}};
static t_fakeh		g_fake_h[FAKE_HANDLES];

t_process	*proc_current(void)
{
	return (&g_fake_proc);
}

t_process	*fake_proc(void)
{
	return (&g_fake_proc);
}

t_fakeh	*fake_htab(void)
{
	return (g_fake_h);
}

t_object	*obj_create(uint32_t type, const t_objops *ops, void *impl)
{
	t_object	*o;

	o = calloc(1, sizeof(*o));
	if (o == NULL)
		return (NULL);
	o->type = type;
	o->refs = 1;
	o->ops = ops;
	o->impl = impl;
	return (o);
}

void	obj_unref(t_object *obj)
{
	if (obj->refs == 0)
		abort();
	if (--obj->refs)
		return ;
	if (obj->ops && obj->ops->destroy)
		obj->ops->destroy(obj);
	free(obj);
}
