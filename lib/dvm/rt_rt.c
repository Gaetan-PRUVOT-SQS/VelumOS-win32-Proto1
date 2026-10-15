#include "rt_int.h"

size_t	rt_cat(char *buf, size_t n, const char *s)
{
	while (s && *s && n + 1 < DJOIN_MAX)
		buf[n++] = *s++;
	buf[n] = '\0';
	return (n);
}

int	rt_class_idx(t_dvm *vm, uint32_t type_idx, t_dclass **out)
{
	const char	*desc;

	*out = NULL;
	if (!vm || !vm->classes->has_dex)
		return (E_INVAL);
	desc = rt_dtype(&vm->classes->dex, type_idx);
	if (!desc)
		return (E_INVAL);
	return (dvm_class(vm, desc, out));
}

int	dvmrt_new_instance(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref *out)
{
	t_dclass	*c;
	int			r;

	(void)from;
	if (!out)
		return (E_INVAL);
	*out = DVM_NULL;
	r = rt_class_idx(vm, idx, &c);
	if (r != 0)
		return (r);
	if (c->is_array || (c->access & (ACC_ABSTRACT | ACC_INTERFACE))
		|| c == vm->classes->core[DCORE_STRING])
		return (dvm_throw(vm, "Ljava/lang/InstantiationError;", c->desc));
	return (dvm_new(vm, c, out));
}

int	dvmrt_const_string(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref *out)
{
	(void)from;
	if (!vm || !out)
		return (E_INVAL);
	return (rt_intern(vm, idx, out));
}

int	dvmrt_const_class(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref *out)
{
	t_dclass	*c;
	t_dclass	**slot;
	int			r;

	(void)from;
	if (!out)
		return (E_INVAL);
	*out = DVM_NULL;
	r = rt_class_idx(vm, idx, &c);
	if (r != 0)
		return (r);
	*out = c->mirror;
	if (rt_obj(vm, *out, DOK_OBJECT))
		return (0);
	r = dvm_new(vm, vm->classes->core[DCORE_CLASS], out);
	if (r != 0)
		return (r);
	slot = rt_obj(vm, *out, DOK_OBJECT)->data;
	*slot = c;
	c->mirror = *out;
	return (0);
}
