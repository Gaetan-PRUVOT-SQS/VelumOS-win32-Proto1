#include "rt_int.h"

int	dvm_class(t_dvm *vm, const char *desc, t_dclass **out)
{
	if (!vm || !desc || !out)
		return (E_INVAL);
	*out = rt_find(vm, desc);
	if (*out)
		return (0);
	if (desc[0] == '[')
		return (rt_array_class(vm, desc, out));
	return (rt_link(vm, desc, out));
}

const char	*dvm_class_name(const t_dclass *c)
{
	if (!c)
		return (NULL);
	return (c->desc);
}

static int	rt_implements(const t_dclass *c, const t_dclass *to)
{
	uint32_t	i;

	while (c)
	{
		i = 0;
		while (i < c->nifaces)
		{
			if (c->ifaces[i] == to || rt_implements(c->ifaces[i], to))
				return (1);
			i++;
		}
		c = c->super;
	}
	return (0);
}

int	rt_assignable(const t_dclass *from, const t_dclass *to)
{
	if (!from || !to)
		return (0);
	if (from == to)
		return (1);
	if (from->is_array && to->is_array)
	{
		if (from->is_ref && to->is_ref)
			return (rt_assignable(from->elem, to->elem));
		return (0);
	}
	if (rt_implements(from, to))
		return (1);
	from = from->super;
	while (from && from != to)
		from = from->super;
	return (from != NULL);
}

int	dvm_is_instance(t_dvm *vm, t_dref ref, const t_dclass *c)
{
	t_dobj	*o;

	o = rt_obj(vm, ref, DOK_ANY);
	if (!o || !c)
		return (0);
	return (rt_assignable(o->cls, c));
}
