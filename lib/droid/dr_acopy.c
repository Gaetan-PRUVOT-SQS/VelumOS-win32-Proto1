#include "dr_int.h"

static int	dr_ac_bounds(const t_darrview *a, uint32_t pos, uint32_t len)
{
	return ((int32_t)pos >= 0 && (int32_t)len >= 0 && pos <= a->len
		&& len <= a->len - pos);
}

static int	dr_ac_refs(t_dvm *vm, const uint32_t *args, const t_darrview *v)
{
	t_dclass	*elem;
	uint32_t	*s;
	uint32_t	*d;
	uint32_t	i;

	elem = dr_class(vm, dvm_class_name(dvm_class_of(vm, args[2])) + 1);
	s = (uint32_t *)v[0].data + args[1];
	d = (uint32_t *)v[1].data + args[3];
	i = 0;
	while (i < args[4])
	{
		if (s[i] != DVM_NULL && !dvm_is_instance(vm, s[i], elem))
			return (dvm_throw(vm, DR_ASE, "élément incompatible"));
		d[i] = s[i];
		i++;
	}
	return (0);
}

int	dr_sys_arraycopy(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_darrview	v[2];
	uint8_t		*p;

	(void)ret;
	if (args[0] == DVM_NULL || args[2] == DVM_NULL)
		return (dr_npe(vm));
	if (dvm_array_view(vm, args[0], &v[0]) != 0
		|| dvm_array_view(vm, args[2], &v[1]) != 0
		|| v[0].is_ref != v[1].is_ref)
		return (dvm_throw(vm, DR_ASE, "tableaux incompatibles"));
	if (!dr_ac_bounds(&v[0], args[1], args[4])
		|| !dr_ac_bounds(&v[1], args[3], args[4]))
		return (dvm_throw(vm, DR_AIOOBE, "arraycopy hors bornes"));
	if (dvm_class_of(vm, args[0]) != dvm_class_of(vm, args[2]))
	{
		if (!v[0].is_ref)
			return (dvm_throw(vm, DR_ASE, "types primitifs différents"));
		return (dr_ac_refs(vm, args, v));
	}
	p = v[1].data;
	memmove(p + (size_t)args[3] * v[1].width,
		(uint8_t *)v[0].data + (size_t)args[1] * v[0].width,
		(size_t)args[4] * v[0].width);
	return (0);
}
