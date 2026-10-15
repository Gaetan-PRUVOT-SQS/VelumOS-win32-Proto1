#include "rt_int.h"

static int	rt_akind_ok(const t_dclass *c, uint8_t kind)
{
	char	e;

	e = c->desc[1];
	if (kind == DAK_INT)
		return (e == 'I' || e == 'F');
	if (kind == DAK_WIDE)
		return (e == 'J' || e == 'D');
	if (kind == DAK_OBJECT)
		return (c->is_ref);
	if (kind == DAK_BOOLEAN)
		return (e == 'Z');
	if (kind == DAK_BYTE)
		return (e == 'B');
	if (kind == DAK_CHAR)
		return (e == 'C');
	return (kind == DAK_SHORT && e == 'S');
}

static void	rt_aget(const t_dobj *o, t_daacc *a)
{
	if (o->cls->width == 1)
		a->val = (uint32_t)(int32_t)((int8_t *)o->data)[a->index];
	if (a->kind == DAK_BOOLEAN)
		a->val = ((uint8_t *)o->data)[a->index];
	if (o->cls->width == 2)
		a->val = (uint32_t)(int32_t)((int16_t *)o->data)[a->index];
	if (a->kind == DAK_CHAR)
		a->val = ((uint16_t *)o->data)[a->index];
	if (o->cls->width == 4)
		a->val = ((uint32_t *)o->data)[a->index];
	if (o->cls->width == 8)
		a->val = ((uint64_t *)o->data)[a->index];
}

void	rt_aput(const t_dobj *o, const t_daacc *a)
{
	if (o->cls->width == 1)
		((uint8_t *)o->data)[a->index] = (uint8_t)a->val;
	if (o->cls->width == 2)
		((uint16_t *)o->data)[a->index] = (uint16_t)a->val;
	if (o->cls->width == 4)
		((uint32_t *)o->data)[a->index] = (uint32_t)a->val;
	if (o->cls->width == 8)
		((uint64_t *)o->data)[a->index] = a->val;
}

static int	rt_astore_ok(const t_dvm *vm, const t_dobj *o, const t_daacc *a)
{
	t_dobj	*v;

	if (!o->cls->is_ref || a->val == DVM_NULL)
		return (1);
	if (a->val > 0xffffffffu)
		return (0);
	v = rt_obj(vm, (t_dref)a->val, DOK_ANY);
	return (v && rt_assignable(v->cls, o->cls->elem));
}

int	dvmrt_array(t_dvm *vm, t_daacc *acc)
{
	t_dobj	*o;

	if (!vm || !acc)
		return (E_INVAL);
	if (acc->arr == DVM_NULL)
		return (dvm_throw(vm, DX_NPE, NULL));
	o = rt_obj(vm, acc->arr, DOK_ARRAY);
	if (!o || !rt_akind_ok(o->cls, acc->kind))
		return (dvm_throw(vm, DX_CCE, NULL));
	if (acc->index < 0 || (uint32_t)acc->index >= o->len)
		return (dvm_throw(vm, DX_AIOOBE, NULL));
	if (!acc->put)
		rt_aget(o, acc);
	if (!acc->put)
		return (0);
	if (!rt_astore_ok(vm, o, acc))
		return (dvm_throw(vm, DX_ASE, NULL));
	rt_aput(o, acc);
	return (0);
}
