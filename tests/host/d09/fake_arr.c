#include "d09.h"

int	dvmrt_new_array(t_dvm *vm, const t_dmethod *from, t_dnewarr *rq)
{
	const char	*desc;

	(void)from;
	desc = fk_type(rq->type_idx);
	if (rq->len < 0)
		return (dvm_throw(vm, "Ljava/lang/NegativeArraySizeException;",
				NULL));
	rq->out = fk_obj(desc, (uint32_t)rq->len);
	if (rq->out == DVM_NULL)
		return (E_NOMEM);
	g_fk.obj[rq->out].is_ref = (desc[1] == 'L' || desc[1] == '[');
	return (0);
}

int	dvmrt_array(t_dvm *vm, t_daacc *acc)
{
	t_fkobj	*o;

	if (acc->arr == DVM_NULL || acc->arr >= FK_OBJS)
		return (dvm_throw(vm, FK_NPE, NULL));
	o = &g_fk.obj[acc->arr];
	if (acc->index < 0 || (uint32_t)acc->index >= o->len)
		return (dvm_throw(vm, "Ljava/lang/ArrayIndexOutOfBoundsException;",
				NULL));
	if (acc->put)
		o->data[acc->index] = acc->val;
	else
		acc->val = o->data[acc->index];
	return (0);
}

int	dvmrt_array_length(t_dvm *vm, t_dref arr, uint32_t *len)
{
	if (arr == DVM_NULL || arr >= FK_OBJS)
		return (dvm_throw(vm, FK_NPE, NULL));
	*len = g_fk.obj[arr].len;
	return (0);
}

int	dvmrt_fill_array(t_dvm *vm, t_dref arr, const t_darray *data)
{
	t_fkobj		*o;
	uint64_t	v;
	uint32_t	i;
	uint32_t	k;

	(void)vm;
	o = &g_fk.obj[arr % FK_OBJS];
	i = 0;
	while (i < data->count && i < o->len)
	{
		v = 0;
		k = data->width;
		while (k-- > 0)
			v = (v << 8) | data->data[(size_t)i * data->width + k];
		if (data->width == 1)
			v = (uint32_t)(int32_t)(int8_t)v;
		if (data->width == 2)
			v = (uint32_t)(int32_t)(int16_t)v;
		o->data[i++] = v;
	}
	return (0);
}

int	dvm_array_view(t_dvm *vm, t_dref ref, t_darrview *out)
{
	(void)vm;
	if (ref == DVM_NULL || ref >= FK_OBJS)
		return (E_INVAL);
	out->data = g_fk.obj[ref].data;
	out->len = g_fk.obj[ref].len;
	out->width = 8;
	out->is_ref = g_fk.obj[ref].is_ref;
	return (0);
}
