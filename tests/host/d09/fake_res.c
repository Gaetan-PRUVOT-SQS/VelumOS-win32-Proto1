#include "d09.h"

int	dvmrt_instance_of(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref obj)
{
	(void)vm;
	(void)from;
	if (obj == DVM_NULL || obj >= FK_OBJS)
		return (0);
	return (fk_same(g_fk.obj[obj].desc, fk_type(idx)));
}

int	dvmrt_catches(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref exc)
{
	(void)vm;
	(void)from;
	if (exc == DVM_NULL || exc >= FK_OBJS)
		return (0);
	return (fk_same(g_fk.obj[exc].desc, fk_type(idx))
		|| fk_same(fk_type(idx), "Ljava/lang/Throwable;"));
}

int	dvmrt_field(t_dvm *vm, const t_dmethod *from, t_dfacc *acc)
{
	uint64_t	*slot;

	(void)vm;
	(void)from;
	if (acc->field_idx >= FK_FIELDS || acc->obj >= FK_OBJS)
		return (E_RANGE);
	slot = &g_fk.statics[acc->field_idx];
	if (!acc->is_static && !g_fk.obj[acc->obj].data)
		return (E_INVAL);
	if (!acc->is_static)
		slot = &g_fk.obj[acc->obj].data[acc->field_idx % FK_SLOTS];
	if (acc->put)
		*slot = acc->val;
	else
		acc->val = *slot;
	return (0);
}
