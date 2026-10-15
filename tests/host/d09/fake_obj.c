#include "d09.h"

int	dvm_throw(t_dvm *vm, const char *desc, const char *msg)
{
	t_dref	ref;

	(void)msg;
	ref = fk_obj(desc, 0);
	if (ref == DVM_NULL)
		return (E_NOMEM);
	vm->pending = ref;
	return (DVM_THROWN);
}

int	dvmrt_new_instance(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref *out)
{
	(void)vm;
	(void)from;
	*out = fk_obj(fk_type(idx), FK_SLOTS);
	if (*out == DVM_NULL)
		return (E_NOMEM);
	return (0);
}

int	dvmrt_const_string(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref *out)
{
	(void)vm;
	(void)from;
	(void)idx;
	*out = fk_obj("Ljava/lang/String;", 0);
	if (*out == DVM_NULL)
		return (E_NOMEM);
	return (0);
}

int	dvmrt_const_class(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref *out)
{
	(void)vm;
	(void)from;
	(void)idx;
	*out = fk_obj("Ljava/lang/Class;", 0);
	if (*out == DVM_NULL)
		return (E_NOMEM);
	return (0);
}
