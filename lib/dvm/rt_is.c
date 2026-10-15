#include "rt_int.h"

int	dvmrt_instance_of(t_dvm *vm, const t_dmethod *from, uint32_t idx,
		t_dref obj)
{
	t_dclass	*c;
	t_dobj		*o;
	int			r;

	(void)from;
	r = rt_class_idx(vm, idx, &c);
	if (r == DVM_THROWN)
		return (E_NOENT);
	if (r != 0)
		return (r);
	o = rt_obj(vm, obj, DOK_ANY);
	if (!o)
		return (0);
	return (rt_assignable(o->cls, c));
}

int	dvmrt_catches(t_dvm *vm, const t_dmethod *from, uint32_t idx, t_dref exc)
{
	t_dref	pending;
	int		r;

	if (!vm)
		return (E_INVAL);
	if (idx == DEX_NO_INDEX)
		return (1);
	pending = vm->pending;
	dvm_pin(vm, pending);
	r = dvmrt_instance_of(vm, from, idx, exc);
	dvm_unpin(vm, pending);
	vm->pending = pending;
	if (r == E_NOENT)
		return (0);
	return (r);
}

int	dvmrt_new_array(t_dvm *vm, const t_dmethod *from, t_dnewarr *rq)
{
	const char	*desc;

	(void)from;
	if (!vm || !rq || !vm->classes->has_dex)
		return (E_INVAL);
	rq->out = DVM_NULL;
	desc = rt_dtype(&vm->classes->dex, rq->type_idx);
	if (!desc || desc[0] != '[')
		return (E_INVAL);
	return (dvm_array_new(vm, desc, rq->len, &rq->out));
}

int	dvmrt_array_length(t_dvm *vm, t_dref arr, uint32_t *len)
{
	t_dobj	*o;

	if (!vm || !len)
		return (E_INVAL);
	if (arr == DVM_NULL)
		return (dvm_throw(vm, DX_NPE, NULL));
	o = rt_obj(vm, arr, DOK_ARRAY);
	if (!o)
		return (dvm_throw(vm, DX_CCE, NULL));
	*len = o->len;
	return (0);
}

int	dvmrt_fill_array(t_dvm *vm, t_dref arr, const t_darray *data)
{
	t_dobj		*o;
	t_daacc		a;
	uint32_t	k;

	if (!vm || !data || !data->data)
		return (E_INVAL);
	if (arr == DVM_NULL)
		return (dvm_throw(vm, DX_NPE, NULL));
	o = rt_obj(vm, arr, DOK_ARRAY);
	if (!o || o->cls->is_ref || o->cls->width != data->width)
		return (dvm_throw(vm, DX_CCE, NULL));
	if (data->count > o->len)
		return (dvm_throw(vm, DX_AIOOBE, NULL));
	a.index = 0;
	while ((uint32_t)a.index < data->count)
	{
		a.val = 0;
		k = data->width;
		while (k-- > 0)
			a.val = (a.val << 8)
				| data->data[(size_t)a.index * data->width + k];
		rt_aput(o, &a);
		a.index++;
	}
	return (0);
}
