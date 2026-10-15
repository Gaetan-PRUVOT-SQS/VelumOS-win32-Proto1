#include "rt_int.h"

static t_dfield	*rt_field_in(const t_dclass *c, const char *name,
		const char *type)
{
	uint32_t	i;

	i = 0;
	while (name && type && i < c->nfields)
	{
		if (c->fields[i].name && strcmp(c->fields[i].name, name) == 0
			&& strcmp(c->fields[i].type, type) == 0)
			return (&c->fields[i]);
		i++;
	}
	return (NULL);
}

static int	rt_field_find(t_dvm *vm, const t_dfacc *acc, t_dfield **f,
		t_dclass **k)
{
	const t_dex	*d;
	t_dexfield	df;
	int			r;

	d = &vm->classes->dex;
	if (!vm->classes->has_dex || dex_field(d, acc->field_idx, &df) != 0)
		return (E_INVAL);
	r = rt_class_idx(vm, df.class_idx, k);
	if (r != 0)
		return (r);
	*f = NULL;
	while (*k && !*f)
	{
		*f = rt_field_in(*k, rt_dstr(d, df.name_idx),
				rt_dtype(d, df.type_idx));
		if (!*f)
			*k = (*k)->super;
	}
	if (!*f)
		return (dvm_throw(vm, "Ljava/lang/NoSuchFieldError;",
				rt_dstr(d, df.name_idx)));
	return (0);
}

static int	rt_field_ptr(t_dvm *vm, const t_dfacc *acc, const t_dfield *f,
		uint32_t **p)
{
	t_dobj	*o;

	if (f->is_static != (acc->is_static != 0))
		return (dvm_throw(vm, "Ljava/lang/IncompatibleClassChangeError;",
				f->name));
	if (f->wide != (acc->wide != 0) || f->is_ref != (acc->is_ref != 0))
		return (dvm_throw(vm, DX_VERIFY, f->name));
	if (!f->is_static && acc->obj == DVM_NULL)
		return (dvm_throw(vm, DX_NPE, f->name));
	if (acc->put && f->is_ref && acc->val != DVM_NULL
		&& (acc->val > 0xffffffffu
			|| !rt_obj(vm, (t_dref)acc->val, DOK_ANY)))
		return (dvm_throw(vm, DX_VERIFY, f->name));
	if (f->is_static)
		return (0);
	o = rt_obj(vm, acc->obj, DOK_OBJECT);
	if (!o || f->slot + f->wide >= o->len)
		return (dvm_throw(vm, DX_CCE, f->name));
	*p = &rt_words(o)[f->slot];
	return (0);
}

static void	rt_field_io(t_dfacc *acc, const t_dfield *f, uint32_t *p)
{
	if (acc->put)
		p[0] = (uint32_t)acc->val;
	if (acc->put && f->wide)
		p[1] = (uint32_t)(acc->val >> 32);
	if (!acc->put)
		acc->val = p[0];
	if (!acc->put && f->wide)
		acc->val |= (uint64_t)p[1] << 32;
}

int	dvmrt_field(t_dvm *vm, const t_dmethod *from, t_dfacc *acc)
{
	t_dfield	*f;
	t_dclass	*k;
	uint32_t	*p;
	int			r;

	(void)from;
	if (!vm || !acc)
		return (E_INVAL);
	r = rt_field_find(vm, acc, &f, &k);
	if (r != 0)
		return (r);
	p = &k->statics[f->slot];
	r = rt_field_ptr(vm, acc, f, &p);
	if (r != 0)
		return (r);
	rt_field_io(acc, f, p);
	return (0);
}
